#include "join_index.hpp"

#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "all_type_variant.hpp"
#include "index_scan.hpp"
#include "join_nested_loop.hpp"
#include "multi_predicate_join/multi_predicate_join_evaluator.hpp"
#include "resolve_type.hpp"
#include "storage/index/base_index.hpp"
#include "storage/segment_iterate.hpp"
#include "table_wrapper.hpp"
#include "type_comparison.hpp"
#include "utils/assert.hpp"
#include "utils/performance_warning.hpp"

namespace opossum {

/*
 * This is an index join implementation. It expects to find an index on the right column.
 * It can be used for all join modes except JoinMode::Cross.
 * For the remaining join types or if no index is found it falls back to a nested loop join.
 */

JoinIndex::JoinIndex(const std::shared_ptr<const AbstractOperator>& left,
                     const std::shared_ptr<const AbstractOperator>& right, const JoinMode mode,
                     const OperatorJoinPredicate& primary_predicate)
    : AbstractJoinOperator(OperatorType::JoinIndex, left, right, mode, primary_predicate, {},
                           std::make_unique<JoinIndex::PerformanceData>()) {
  DebugAssert(mode != JoinMode::Cross, "Cross Join is not supported by index join.");
}

const std::string JoinIndex::name() const { return "JoinIndex"; }

std::shared_ptr<AbstractOperator> JoinIndex::_on_deep_copy(
    const std::shared_ptr<AbstractOperator>& copied_input_left,
    const std::shared_ptr<AbstractOperator>& copied_input_right) const {
  return std::make_shared<JoinIndex>(copied_input_left, copied_input_right, _mode, _primary_predicate);
}

void JoinIndex::_on_set_parameters(const std::unordered_map<ParameterID, AllTypeVariant>& parameters) {}

std::shared_ptr<const Table> JoinIndex::_on_execute() {
  _output_table = _initialize_output_table();

  if (_mode == JoinMode::Inner && input_table_right()->type() == TableType::References &&
      _secondary_predicates.empty()) {
    _perform_join_right_reference_table();
  } else {
    _perform_join();
  }

  return _output_table;
}

void JoinIndex::_perform_join() {
  _right_matches.resize(input_table_right()->chunk_count());
  _left_matches.resize(input_table_left()->chunk_count());

  const auto track_left_matches = (_mode == JoinMode::Left || _mode == JoinMode::FullOuter);
  if (track_left_matches) {
    for (ChunkID chunk_id_left = ChunkID{0}; chunk_id_left < input_table_left()->chunk_count(); ++chunk_id_left) {
      // initialize the data structures for left matches
      _left_matches[chunk_id_left].resize(input_table_left()->get_chunk(chunk_id_left)->size());
    }
  }

  const auto is_semi_or_anti_join =
      _mode == JoinMode::Semi || _mode == JoinMode::AntiNullAsFalse || _mode == JoinMode::AntiNullAsTrue;

  const auto track_right_matches = (_mode == JoinMode::Right || _mode == JoinMode::FullOuter);

  _pos_list_left = std::make_shared<PosList>();
  _pos_list_right = std::make_shared<PosList>();

  auto pos_list_size_to_reserve =
      std::max(uint64_t{100}, std::min(input_table_left()->row_count(), input_table_right()->row_count()));

  _pos_list_left->reserve(pos_list_size_to_reserve);
  _pos_list_right->reserve(pos_list_size_to_reserve);

  auto& performance_data = static_cast<PerformanceData&>(*_performance_data);

  auto secondary_predicate_evaluator = MultiPredicateJoinEvaluator{*input_table_left(), *input_table_right(), {}};

  // Scan all chunks for right input
  for (ChunkID chunk_id_right = ChunkID{0}; chunk_id_right < input_table_right()->chunk_count(); ++chunk_id_right) {
    const auto chunk_right = input_table_right()->get_chunk(chunk_id_right);
    const auto indices = chunk_right->get_indices(std::vector<ColumnID>{_primary_predicate.column_ids.second});
    if (track_right_matches) _right_matches[chunk_id_right].resize(chunk_right->size());

    std::shared_ptr<BaseIndex> index = nullptr;

    if (!indices.empty()) {
      // We assume the first index to be efficient for our join
      // as we do not want to spend time on evaluating the best index inside of this join loop
      index = indices.front();
    }

    // Scan all chunks from left input
    if (index) {
      for (ChunkID chunk_id_left = ChunkID{0}; chunk_id_left < input_table_left()->chunk_count(); ++chunk_id_left) {
        const auto segment_left =
            input_table_left()->get_chunk(chunk_id_left)->get_segment(_primary_predicate.column_ids.first);

        segment_with_iterators(*segment_left, [&](auto it, const auto end) {
          _join_two_segments_using_index(it, end, chunk_id_left, chunk_id_right, index);
        });
      }
      performance_data.chunks_scanned_with_index++;
    } else {
      // Fall back to NestedLoopJoin
      const auto segment_right =
          input_table_right()->get_chunk(chunk_id_right)->get_segment(_primary_predicate.column_ids.second);
      for (ChunkID chunk_id_left = ChunkID{0}; chunk_id_left < input_table_left()->chunk_count(); ++chunk_id_left) {
        const auto segment_left =
            input_table_left()->get_chunk(chunk_id_left)->get_segment(_primary_predicate.column_ids.first);
        JoinNestedLoop::JoinParams params{*_pos_list_left,
                                          *_pos_list_right,
                                          _left_matches[chunk_id_left],
                                          _right_matches[chunk_id_right],
                                          track_left_matches,
                                          track_right_matches,
                                          _mode,
                                          _primary_predicate.predicate_condition,
                                          secondary_predicate_evaluator,
                                          !is_semi_or_anti_join};
        JoinNestedLoop::_join_two_untyped_segments(*segment_left, *segment_right, chunk_id_left, chunk_id_right,
                                                   params);
      }
      performance_data.chunks_scanned_without_index++;
    }
  }

  // For Full Outer and Left Join we need to add all unmatched rows for the left side
  if (_mode == JoinMode::Left || _mode == JoinMode::FullOuter) {
    for (ChunkID chunk_id_left = ChunkID{0}; chunk_id_left < input_table_left()->chunk_count(); ++chunk_id_left) {
      for (ChunkOffset chunk_offset{0}; chunk_offset < _left_matches[chunk_id_left].size(); ++chunk_offset) {
        if (!_left_matches[chunk_id_left][chunk_offset]) {
          _pos_list_left->emplace_back(RowID{chunk_id_left, chunk_offset});
          _pos_list_right->emplace_back(NULL_ROW_ID);
        }
      }
    }
  }

  // For Full Outer and Right Join we need to add all unmatched rows for the right side.
  if (_mode == JoinMode::FullOuter || _mode == JoinMode::Right) {
    for (ChunkID chunk_id{0}; chunk_id < _right_matches.size(); ++chunk_id) {
      for (ChunkOffset chunk_offset{0}; chunk_offset < _right_matches[chunk_id].size(); ++chunk_offset) {
        if (!_right_matches[chunk_id][chunk_offset]) {
          _pos_list_right->emplace_back(RowID{chunk_id, chunk_offset});
          _pos_list_left->emplace_back(NULL_ROW_ID);
        }
      }
    }
  }

  _pos_list_left->shrink_to_fit();
  _pos_list_right->shrink_to_fit();

  // write output chunks
  Segments output_segments;

  _write_output_segments(output_segments, input_table_left(), _pos_list_left);
  _write_output_segments(output_segments, input_table_right(), _pos_list_right);

  _output_table->append_chunk(output_segments);

  if (performance_data.chunks_scanned_with_index < performance_data.chunks_scanned_without_index) {
    PerformanceWarning(
        std::string("Only ") + std::to_string(performance_data.chunks_scanned_with_index) + " of " +
        std::to_string(performance_data.chunks_scanned_with_index + performance_data.chunks_scanned_without_index) +
        " chunks scanned using an index");
  }
}

void JoinIndex::_perform_join_right_reference_table() {
  std::vector<ColumnID> data_table_index_column_ids;

  // get referenced data table
  std::shared_ptr<const Table> referenced_data_table;
  if (!input_table_right()->chunks()[0]->segments().empty()) {
    const auto& first_reference_segment = std::dynamic_pointer_cast<ReferenceSegment>(
        input_table_right()->chunks()[0]->segments()[_primary_predicate.column_ids.second]);
    if (first_reference_segment != nullptr) {
      referenced_data_table = first_reference_segment->referenced_table();
      // Experiment assumption (this is not valid generally):
      // Each reference segment of the reference table references the same ColumnId
      data_table_index_column_ids.emplace_back(first_reference_segment->referenced_column_id());
    }
  }
  // use _perform_join if the referenced data table has no index
  Assert(referenced_data_table != nullptr, "ReferenceSegment has no reference table.");
  if (referenced_data_table->get_indexes().empty()) {
    _perform_join();
  } else {
    // Assumption: Original data table of the right input table has
    // an index for each segment that is evaluated for the join

    // build the position list for the right input table
    auto input_right_table_positions = PosList{};
    input_right_table_positions.reserve(input_table_right()->row_count());
    for (const auto& chunk : input_table_right()->chunks()) {
      if (!chunk->segments().empty()) {
        const auto& reference_segment = std::dynamic_pointer_cast<ReferenceSegment>(chunk->segments()[0]);
        Assert(reference_segment != nullptr, "Segment of reference table is not of type ReferenceSegment.");
        const auto& reference_segment_pos_list = reference_segment->pos_list();
        input_right_table_positions.insert(input_right_table_positions.end(), reference_segment_pos_list->begin(),
                                           reference_segment_pos_list->end());
      }
    }

    // std::cout << "input_right_table_positions sorted: ";
    // std::cout << std::is_sorted(input_right_table_positions.begin(), input_right_table_positions.end()) << "\n";

    // std::sort(input_right_table_positions.begin(), input_right_table_positions.end());

    //    const auto first_row_id = input_right_table_positions.front();
    //    const auto last_row_id = input_right_table_positions.back();

    _pos_list_left = std::make_shared<PosList>();
    _pos_list_right = std::make_shared<PosList>();

    _pos_list_left->reserve(input_right_table_positions.size());
    _pos_list_right->reserve(input_right_table_positions.size());

    //  iterate over the left join column
    //    for each value vl of that column:
    //      execute an index scan on the referenced_data_table
    //      get the global posList (right_data_table_matches) for the scan on the referenced_data_table
    //      sort input_right_table_positions
    //      sort right_data_table_matches
    //      input_right_table_matches = intersection(input_right_table_positions, right_data_table_matches)
    //      add the RowID of vl as often as the size of input_right_table_matches to _pos_list_left
    //      add input_right_table_matches to _pos_list_right

    const auto& referenced_data_table_wrapper = std::make_shared<TableWrapper>(referenced_data_table);
    referenced_data_table_wrapper->execute();

    auto left_chunk_id = ChunkID{0};
    for (const auto& chunk : input_table_left()->chunks()) {
      const auto& left_segment = chunk->get_segment(_primary_predicate.column_ids.first);

      resolve_data_and_segment_type(*left_segment, [&](auto type, auto& typed_segment) {
        using Type = typename decltype(type)::type;
        auto iterable = create_iterable_from_segment<Type>(typed_segment);

        auto left_chunk_offset = ChunkOffset{0};
        iterable.with_iterators([&](auto iterator, auto end) {
          for (; iterator != end; ++iterator) {
            const auto& value = *iterator;
            const std::vector<AllTypeVariant> right_values{AllTypeVariant{value.value()}};
            // WARNING! The SegmentIndexType is hard coded for the benchmark experiment here.
            // TODO(anyone) modify passing the SegmentIndexType
            const auto& index_scan_on_data_table = std::make_shared<IndexScan>(
                referenced_data_table_wrapper, SegmentIndexType::GroupKey, data_table_index_column_ids,
                _primary_predicate.predicate_condition, right_values);
            index_scan_on_data_table->execute();
            auto right_data_table_matches = _matches_of_reference_table(index_scan_on_data_table->get_output());

            // std::cout << "right_data_table_matches sorted: ";
            // std::cout << std::is_sorted(right_data_table_matches->begin(), right_data_table_matches->end()) << "\n";

            // std::sort(right_data_table_matches->begin(), right_data_table_matches->end());
            auto input_right_table_matches = PosList{};

            std::set_intersection(input_right_table_positions.begin(), input_right_table_positions.end(),
                                  right_data_table_matches->begin(), right_data_table_matches->end(),
                                  std::back_inserter(input_right_table_matches));
            _append_matches(left_chunk_id, left_chunk_offset, input_right_table_matches);
            ++left_chunk_offset;
          }
        });
      });
      ++left_chunk_id;
    }

    // write output chunks
    Segments output_segments;

    _write_output_segments(output_segments, input_table_left(), _pos_list_left);
    _write_output_segments(output_segments, input_table_right(), _pos_list_right, false);

    _output_table->append_chunk(output_segments);
  }
}

std::shared_ptr<PosList> JoinIndex::_matches_of_reference_table(const std::shared_ptr<const Table>& table) {
  auto matches = std::make_shared<PosList>();
  matches->reserve(table->row_count());

  if (!table->chunks().empty() && !table->get_chunk(ChunkID{0})->segments().empty()) {
    for (const auto& chunk : table->chunks()) {
      const auto& reference_segment = std::dynamic_pointer_cast<ReferenceSegment>(chunk->get_segment(ColumnID{0}));
      Assert(reference_segment != nullptr, "Segment is not a ReferenceSegment");
      const auto& segment_matches = reference_segment->pos_list();
      matches->insert(matches->end(), segment_matches->begin(), segment_matches->end());
    }
  }
  return matches;
}

void JoinIndex::_append_matches(const ChunkID& left_chunk_id, const ChunkOffset& left_chunk_offset,
                                const PosList& right_table_matches) {
  for (const auto& right_row_id : right_table_matches) {
    _pos_list_left->emplace_back(RowID{left_chunk_id, left_chunk_offset});
    _pos_list_right->emplace_back((right_row_id));
  }
}

// join loop that joins two segments of two columns using an iterator for the left, and an index for the right
template <typename LeftIterator>
void JoinIndex::_join_two_segments_using_index(LeftIterator left_it, LeftIterator left_end, const ChunkID chunk_id_left,
                                               const ChunkID chunk_id_right, const std::shared_ptr<BaseIndex>& index) {
  for (; left_it != left_end; ++left_it) {
    const auto left_value = *left_it;
    if (left_value.is_null()) continue;

    auto range_begin = BaseIndex::Iterator{};
    auto range_end = BaseIndex::Iterator{};

    switch (_primary_predicate.predicate_condition) {
      case PredicateCondition::Equals: {
        range_begin = index->lower_bound({left_value.value()});
        range_end = index->upper_bound({left_value.value()});
        break;
      }
      case PredicateCondition::NotEquals: {
        // first, get all values less than the search value
        range_begin = index->cbegin();
        range_end = index->lower_bound({left_value.value()});

        _append_matches(range_begin, range_end, left_value.chunk_offset(), chunk_id_left, chunk_id_right);

        // set range for second half to all values greater than the search value
        range_begin = index->upper_bound({left_value.value()});
        range_end = index->cend();
        break;
      }
      case PredicateCondition::GreaterThan: {
        range_begin = index->cbegin();
        range_end = index->lower_bound({left_value.value()});
        break;
      }
      case PredicateCondition::GreaterThanEquals: {
        range_begin = index->cbegin();
        range_end = index->upper_bound({left_value.value()});
        break;
      }
      case PredicateCondition::LessThan: {
        range_begin = index->upper_bound({left_value.value()});
        range_end = index->cend();
        break;
      }
      case PredicateCondition::LessThanEquals: {
        range_begin = index->lower_bound({left_value.value()});
        range_end = index->cend();
        break;
      }
      default:
        Fail("Unsupported comparison type encountered");
    }

    _append_matches(range_begin, range_end, left_value.chunk_offset(), chunk_id_left, chunk_id_right);
  }
}

// join loop that joins two segments of two columns via their iterators
template <typename BinaryFunctor, typename LeftIterator, typename RightIterator>
void JoinIndex::_join_two_segments_nested_loop(const BinaryFunctor& func, LeftIterator left_it, LeftIterator left_end,
                                               RightIterator right_begin, RightIterator right_end,
                                               const ChunkID chunk_id_left, const ChunkID chunk_id_right) {
  // No index so we fall back on a nested loop join
  for (; left_it != left_end; ++left_it) {
    const auto left_value = *left_it;
    if (left_value.is_null()) continue;

    for (auto right_it = right_begin; right_it != right_end; ++right_it) {
      const auto right_value = *right_it;
      if (right_value.is_null()) continue;

      if (func(left_value.value(), right_value.value())) {
        _pos_list_left->emplace_back(RowID{chunk_id_left, left_value.chunk_offset()});
        _pos_list_right->emplace_back(RowID{chunk_id_right, right_value.chunk_offset()});

        if (_mode == JoinMode::Left || _mode == JoinMode::FullOuter) {
          _left_matches[chunk_id_left][left_value.chunk_offset()] = true;
        }

        if (_mode == JoinMode::FullOuter || _mode == JoinMode::Right) {
          DebugAssert(chunk_id_right < input_table_right()->chunk_count(), "invalid chunk_id in join_index");
          DebugAssert(right_value.chunk_offset() < input_table_right()->get_chunk(chunk_id_right)->size(),
                      "invalid chunk_offset in join_index");
          _right_matches[chunk_id_right][right_value.chunk_offset()] = true;
        }
      }
    }
  }
}

void JoinIndex::_append_matches(const BaseIndex::Iterator& range_begin, const BaseIndex::Iterator& range_end,
                                const ChunkOffset chunk_offset_left, const ChunkID chunk_id_left,
                                const ChunkID chunk_id_right) {
  const auto num_right_matches = std::distance(range_begin, range_end);

  if (num_right_matches == 0) {
    return;
  }

  // Remember the matches for outer joins
  if (_mode == JoinMode::Left || _mode == JoinMode::FullOuter) {
    _left_matches[chunk_id_left][chunk_offset_left] = true;
  }

  // we replicate the left value for each right value
  std::fill_n(std::back_inserter(*_pos_list_left), num_right_matches, RowID{chunk_id_left, chunk_offset_left});

  std::transform(range_begin, range_end, std::back_inserter(*_pos_list_right),
                 [chunk_id_right](ChunkOffset chunk_offset_right) {
                   return RowID{chunk_id_right, chunk_offset_right};
                 });

  if (_mode == JoinMode::FullOuter || _mode == JoinMode::Right) {
    std::for_each(range_begin, range_end, [this, chunk_id_right](ChunkOffset chunk_offset_right) {
      _right_matches[chunk_id_right][chunk_offset_right] = true;
    });
  }
}

void JoinIndex::_write_output_segments(Segments& output_segments, const std::shared_ptr<const Table>& input_table,
                                       std::shared_ptr<PosList> pos_list, bool row_id_correction) {
  // Add segments from table to output chunk
  for (ColumnID column_id{0}; column_id < input_table->column_count(); ++column_id) {
    std::shared_ptr<BaseSegment> segment;

    if (input_table->type() == TableType::References) {
      if (input_table->chunk_count() > 0) {
        auto new_pos_list = std::make_shared<PosList>();

        ChunkID current_chunk_id{0};

        auto reference_segment = std::static_pointer_cast<const ReferenceSegment>(
            input_table->get_chunk(ChunkID{0})->get_segment(column_id));

        if (row_id_correction) {
          // de-reference to the correct RowID so the output can be used in a Multi Join
          for (const auto& row : *pos_list) {
            if (row.is_null()) {
              new_pos_list->push_back(NULL_ROW_ID);
              continue;
            }
            if (row.chunk_id != current_chunk_id) {
              current_chunk_id = row.chunk_id;

              reference_segment = std::dynamic_pointer_cast<const ReferenceSegment>(
                  input_table->get_chunk(current_chunk_id)->get_segment(column_id));
            }
            new_pos_list->push_back((*reference_segment->pos_list())[row.chunk_offset]);
          }
          segment = std::make_shared<ReferenceSegment>(reference_segment->referenced_table(),
                                                       reference_segment->referenced_column_id(), new_pos_list);
        }

        segment = std::make_shared<ReferenceSegment>(reference_segment->referenced_table(),
                                                     reference_segment->referenced_column_id(), pos_list);
      } else {
        // If there are no Chunks in the input_table, we can't deduce the Table that input_table is referencing to.
        // pos_list will contain only NULL_ROW_IDs anyway, so it doesn't matter which Table the ReferenceSegment that
        // we output is referencing. HACK, but works fine: we create a dummy table and let the ReferenceSegment ref
        // it.
        const auto dummy_table = Table::create_dummy_table(input_table->column_definitions());
        segment = std::make_shared<ReferenceSegment>(dummy_table, column_id, pos_list);
      }
    } else {
      segment = std::make_shared<ReferenceSegment>(input_table, column_id, pos_list);
    }

    output_segments.push_back(segment);
  }
}

void JoinIndex::_on_cleanup() {
  _output_table.reset();
  _pos_list_left.reset();
  _pos_list_right.reset();
  _left_matches.clear();
  _right_matches.clear();
}

void JoinIndex::PerformanceData::output_to_stream(std::ostream& stream, DescriptionMode description_mode) const {
  OperatorPerformanceData::output_to_stream(stream, description_mode);

  stream << (description_mode == DescriptionMode::SingleLine ? " / " : "\\n");
  stream << std::to_string(chunks_scanned_with_index) << " of "
         << std::to_string(chunks_scanned_with_index + chunks_scanned_without_index) << " chunks used an index";
}

}  // namespace opossum
