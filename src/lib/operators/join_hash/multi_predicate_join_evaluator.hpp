#pragma once

#include <memory>
#include <vector>

#include "resolve_type.hpp"
#include "storage/reference_segment.hpp"
#include "storage/segment_accessor.hpp"
#include "storage/table.hpp"
#include "types.hpp"

namespace opossum {

class BaseVoidComparator {
 public:
  BaseVoidComparator() = default;

  BaseVoidComparator(const BaseVoidComparator&) = default;

  BaseVoidComparator(BaseVoidComparator&&) = default;

  virtual bool compare(const void *a, const void *b) const = 0;

  virtual ~BaseVoidComparator() = default;
};

template<typename T>
class EqVoidComparator : public BaseVoidComparator {
 public:
  bool compare(const void *a, const void *b) const override {
    return *static_cast<const T *>(a) == *static_cast<const T *>(b);
  }
};

template<typename T>
class LtEqVoidComparator : public BaseVoidComparator {
 public:
  bool compare(const void *a, const void *b) const override {
    return *static_cast<const T *>(a) <= *static_cast<const T *>(b);
  }
};

template<typename T>
class LtVoidComparator : public BaseVoidComparator {
 public:
  bool compare(const void *a, const void *b) const override {
    return *static_cast<const T *>(a) < *static_cast<const T *>(b);
  }
};

template<typename T>
class GtEqVoidComparator : public BaseVoidComparator {
 public:
  bool compare(const void *a, const void *b) const override {
    return *static_cast<const T *>(a) >= *static_cast<const T *>(b);
  }
};

template<typename T>
class GtVoidComparator : public BaseVoidComparator {
 public:
  bool compare(const void *a, const void *b) const override {
    return *static_cast<const T *>(a) > *static_cast<const T *>(b);
  }
};

template<typename T>
class NEqVoidComparator : public BaseVoidComparator {
 public:
  bool compare(const void *a, const void *b) const override {
    return *static_cast<const T *>(a) != *static_cast<const T *>(b);
  }
};


template<typename CompareFunctor, typename L, typename R>
class Comparator {
 public:
  Comparator(CompareFunctor compare_functor,
             std::vector<AbstractSegmentAccessor<L>> left_accessors,
             std::vector<AbstractSegmentAccessor<R>> right_accessors)
      : _compare{std::move(compare_functor)}, _left_accessors{std::move(left_accessors)},
        _right_accessors{std::move(right_accessors)} {
  }

  Comparator(const Comparator&) = delete;

  Comparator(Comparator&&) = delete;

  bool compare(const RowID left, const RowID right) const {

    // Todo: Expensive copy operations?
    const L left_value = _left_accessors[left.chunk_id].access(left.chunk_offset);
    const R right_value = _right_accessors[right.chunk_id]->access(right.chunk_offset);

    // Todo: What, if we don't have a value here?
    return _compare(left_value.value(), right_value.value());
  }

  virtual ~Comparator() = default;

 private:
  const CompareFunctor _compare;
  const std::vector<AbstractSegmentAccessor<L>> _left_accessors;
  const std::vector<AbstractSegmentAccessor<R>> _right_accessors;
};

class MultiPredicateJoinEvaluator {
 public:
  MultiPredicateJoinEvaluator(const Table& left, const Table& right, const std::vector<JoinPredicate>& join_predicates)
      : _left{left},
        _right{right},
        _join_predicates{join_predicates} {
    std::vector<ColumnID> left_col_ids;
    std::vector<ColumnID> right_col_ids;

    for (const auto& pred : _join_predicates) {
      left_col_ids.push_back(pred.column_id_pair.first);
      right_col_ids.push_back(pred.column_id_pair.second);

      resolve_data_type(_left.column_data_type(pred.column_id_pair.first), [&](auto type) {
        using ColumnDataType = typename decltype(type)::type;

        Comparator<std::equal_to<void>, int, int> bla(std::equal_to<void>{},std::vector<AbstractSegmentAccessor<int>>{},
            std::vector<AbstractSegmentAccessor<int>>{});


        switch (pred.predicate_condition) {
          case PredicateCondition::Equals:
            _comparators.emplace_back(std::make_unique<EqVoidComparator<ColumnDataType>>());
            break;
          case PredicateCondition::GreaterThan:
            _comparators.emplace_back(std::make_unique<GtVoidComparator<ColumnDataType>>());
            break;
          case PredicateCondition::GreaterThanEquals:
            _comparators.emplace_back(std::make_unique<GtEqVoidComparator<ColumnDataType>>());
            break;
          case PredicateCondition::LessThan:
            _comparators.emplace_back(std::make_unique<LtVoidComparator<ColumnDataType>>());
            break;
          case PredicateCondition::LessThanEquals:
            _comparators.emplace_back(std::make_unique<LtEqVoidComparator<ColumnDataType>>());
            break;
          case PredicateCondition::NotEquals:
            _comparators.emplace_back(std::make_unique<NEqVoidComparator<ColumnDataType>>());
            break;
          default:
            throw std::runtime_error("Predicate condition not supported!");
        }
      });
    }

    _left_accessors = _create_accessors(_left, left_col_ids);
    _right_accessors = _create_accessors(_right, right_col_ids);
  }

  MultiPredicateJoinEvaluator(const MultiPredicateJoinEvaluator&) = default;

  MultiPredicateJoinEvaluator(MultiPredicateJoinEvaluator&&) = default;

  bool fulfills_all_predicates(const RowID& left_row_id, const RowID& right_row_id) {
    // std::cout << "fulfills_all_predicates" << std::endl;

    ColumnID col_id{0};
    for (const auto& comparator : _comparators) {
      const void* left_value = _left_accessors[col_id][left_row_id.chunk_id]->get_void_ptr(left_row_id.chunk_offset);
      const void* right_value =
          _right_accessors[col_id][right_row_id.chunk_id]->get_void_ptr(right_row_id.chunk_offset);

      // std::cout << "left: " << *static_cast<const int32_t*>(left_value) << std::endl;
      // std::cout << "right: " << *static_cast<const int32_t*>(right_value) << std::endl;

      if (!comparator->compare(left_value, right_value)) {
        return false;
      }
      ++col_id;
    }

    return true;
  }

  virtual ~MultiPredicateJoinEvaluator() = default;

 protected:
  const Table& _left;
  const Table& _right;
  const std::vector<JoinPredicate>& _join_predicates;

  std::vector<std::unique_ptr<BaseVoidComparator>> _comparators;
  // column // chunk
  std::vector<std::vector<std::unique_ptr<BaseSegmentAccessor>>> _left_accessors;
  std::vector<std::vector<std::unique_ptr<BaseSegmentAccessor>>> _right_accessors;

  static std::vector<std::vector<std::unique_ptr<BaseSegmentAccessor>>> _create_accessors(
      const Table& table, const std::vector<ColumnID>& column_ids) {
    std::vector<std::vector<std::unique_ptr<BaseSegmentAccessor>>> accessors;
    accessors.resize(column_ids.size());

    ColumnID accessor_col_index{0};
    for (const auto& col_id : column_ids) {
      resolve_data_type(table.column_data_type(col_id), [&](auto type) {
        using ColumnDataType = typename decltype(type)::type;
        accessors[accessor_col_index].resize(table.chunk_count());
        for (ChunkID chunk_id{0}; chunk_id < table.chunk_count(); ++chunk_id) {
          const auto& segment = table.get_chunk(chunk_id)->get_segment(col_id);
          accessors[accessor_col_index][chunk_id] = create_base_segment_accessor<ColumnDataType>(segment);
        }
      });
      ++accessor_col_index;
    }
    return accessors;
  }

  template <typename T>
  static std::vector<std::unique_ptr<AbstractSegmentAccessor<T>>> _create_accessors(
      const Table& table, const ColumnID column_id) {
    std::vector<std::unique_ptr<AbstractSegmentAccessor<T>>> accessors;

    accessors.resize(table.chunk_count());
    for (ChunkID chunk_id{0}; chunk_id < table.chunk_count(); ++chunk_id) {
      const auto& segment = table.get_chunk(chunk_id)->get_segment(column_id);
      accessors[chunk_id] = opossum::detail::CreateSegmentAccessor<T>::create(segment);
    }

    return accessors;
  }
};

}  // namespace opossum
