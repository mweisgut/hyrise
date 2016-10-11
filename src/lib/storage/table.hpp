#pragma once

#include <map>
#include <string>
#include <vector>

#include "chunk.hpp"
#include "common.hpp"
#include "types.hpp"

namespace opossum {
class Print;
// A table is partitioned horizontally into a number of chunks
class Table {
 public:
  // creates a table
  // the parameter specifies the maximum chunk size, i.e., partition size
  // default (0) is an unlimited size
  explicit Table(const size_t chunk_size = 0);

  // copying a table is not allowed
  Table(Table const &) = delete;

  // we need to explicitly set the move constructor to default when
  // we overwrite the copy constructor
  Table(Table &&) = default;

  // TODO(anyone): getters should have common name - either with or without get. I prefer without.
  // returns the number of columns
  size_t col_count() const;

  // returns the number of rows
  size_t row_count() const;

  // returns the number of chunks
  size_t chunk_count() const;

  // returns the chunk with the given id
  Chunk &get_chunk(ChunkID chunk_id);

  // returns the column name of the nth column
  const std::string &column_name(size_t column_id) const;

  // returns the column type of the nth column
  const std::string &column_type(size_t column_id) const;

  // returns the column with the given name
  size_t column_id_by_name(const std::string &column_name) const;

  // return the maximum chunk size
  size_t chunk_size() const;

  // adds a column to the end, i.e., right, of the table
  void add_column(const std::string &name, const std::string &type, bool create_value_column = true);

  // inserts a row at the end of the table
  void append(std::initializer_list<AllTypeVariant> values) DEV_ONLY;

  friend class Print;

 protected:
  // 0 means that the chunk has an unlimited size.
  const size_t _chunk_size;
  std::vector<Chunk> _chunks;
  std::vector<std::string> _column_names;
  std::vector<std::string> _column_types;
};
}  // namespace opossum
