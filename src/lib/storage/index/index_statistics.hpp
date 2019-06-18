#pragma once

#include <string>

#include "segment_index_type.hpp"
#include "types.hpp"

namespace opossum {

struct IndexStatistics {
  std::vector<ColumnID> column_ids;
  std::string name;
  SegmentIndexType type;
};

}  // namespace opossum
