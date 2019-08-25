#include "b_tree_index.hpp"

#include "resolve_type.hpp"
#include "storage/index/segment_index_type.hpp"

namespace opossum {

size_t BTreeIndex::estimate_memory_consumption(ChunkOffset row_count, ChunkOffset distinct_count,
                                               uint32_t value_bytes) {
  Fail("BTreeIndex::estimate_memory_consumption() is not implemented yet");
}

BTreeIndex::BTreeIndex(const std::vector<std::shared_ptr<const BaseSegment>>& segments_to_index)
    : AbstractIndex{get_index_type_of<BTreeIndex>()},
      // Empty segment list is illegal but range check needed for accessing the first segment
      _indexed_segment(segments_to_index.empty() ? nullptr : segments_to_index[0]) {
  Assert(static_cast<bool>(_indexed_segment), "BTreeIndex requires segments_to_index not to be empty.");
  Assert((segments_to_index.size() == 1), "BTreeIndex only works with a single segment.");
  _impl = make_shared_by_data_type<BaseBTreeIndexImpl, BTreeIndexImpl>(_indexed_segment->data_type(), _indexed_segment,
                                                                       _index_null_postings);
}

size_t BTreeIndex::_memory_consumption() const { return _impl->memory_consumption(); }

BTreeIndex::Iterator BTreeIndex::_lower_bound(const std::vector<AllTypeVariant>& values) const {
  Assert(!values.empty(), "Value vector has to be non-empty.");

  if (variant_is_null(values[0])) {
    return null_cbegin();
  }

  return _impl->lower_bound(values);
}

BTreeIndex::Iterator BTreeIndex::_upper_bound(const std::vector<AllTypeVariant>& values) const {
  Assert(!values.empty(), "Value vector has to be non-empty.");

  if (variant_is_null(values[0])) {
    return null_cend();
  }

  return _impl->upper_bound(values);
}

BTreeIndex::Iterator BTreeIndex::_cbegin() const { return _impl->cbegin(); }

BTreeIndex::Iterator BTreeIndex::_cend() const { return _impl->cend(); }

std::vector<std::shared_ptr<const BaseSegment>> BTreeIndex::_get_indexed_segments() const { return {_indexed_segment}; }

}  // namespace opossum
