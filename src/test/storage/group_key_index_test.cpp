#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "base_test.hpp"
#include "gtest/gtest.h"

#include "storage/base_segment.hpp"
#include "storage/chunk.hpp"
#include "storage/index/group_key/group_key_index.hpp"
#include "storage/value_segment.hpp"
#include "types.hpp"

namespace opossum {

class GroupKeyIndexTest : public BaseTest {
 protected:
  void SetUp() override {
    auto value_segment_str = std::make_shared<ValueSegment<pmr_string>>(true);
    //                                      //  position
    value_segment_str->append(NULL_VALUE);  //  0
    value_segment_str->append("hotel");     //  1
    value_segment_str->append("delta");     //  2
    value_segment_str->append("frank");     //  3
    value_segment_str->append("delta");     //  4
    value_segment_str->append(NULL_VALUE);  //  5
    value_segment_str->append(NULL_VALUE);  //  6
    value_segment_str->append("apple");     //  7
    value_segment_str->append("charlie");   //  8
    value_segment_str->append("charlie");   //  9
    value_segment_str->append("inbox");     // 10
    value_segment_str->append(NULL_VALUE);  // 11

    dict_segment =
        encode_and_compress_segment(value_segment_str, DataType::String, SegmentEncodingSpec{EncodingType::Dictionary});

    index = std::make_shared<GroupKeyIndex>(std::vector<std::shared_ptr<const BaseSegment>>({dict_segment}));

    index_offsets = &(index->_index_offsets);
    index_postings = &(index->_index_postings);
    index_null_positions = &(index->_null_positions);
  }

  std::shared_ptr<GroupKeyIndex> index = nullptr;
  std::shared_ptr<BaseSegment> dict_segment = nullptr;

  /**
   * Use pointers to inner data structures of CompositeGroupKeyIndex in order to bypass the
   * private scope. In order to minimize the friend classes of CompositeGroupKeyIndex the fixture
   * is used as proxy. Since the variables are set in setup() references are not possible.
   */
  std::vector<ChunkOffset>* index_offsets;
  std::vector<ChunkOffset>* index_postings;
  std::vector<ChunkOffset>* index_null_positions;
};

TEST_F(GroupKeyIndexTest, IndexOffsets) {
  auto expected_offsets = std::vector<ChunkOffset>{0, 1, 3, 5, 6, 7, 8};
  EXPECT_EQ(expected_offsets, *index_offsets);
}

/*
  size_t memory_consumption() const;
  
  |    Characteristic               | Block 1 | Block 2 |
  |---------------------------------|---------|---------|
  |[A] index is empty               |    true |   false |
  |[B] index has null positions     |    true |   false |
  |[C] index has non-null positions |    true |   false |
  
  Base Choice:
    A2, B1, C1
  Further derived combinations:
    A2, B1, C2
    A2, B2, C1
   (A1, B1, C1) --infeasible---+
    A1, B2, C2 <-alternative-<-+
*/
TEST_F(GroupKeyIndexTest, IndexMemoryConsumption) {
  const auto& dict_segment_int_no_nulls = BaseTest::create_dict_segment_by_type<int32_t>(DataType::Int, {13, 37});
  const auto& dict_segment_int_nulls =
      BaseTest::create_dict_segment_by_type<int32_t>(DataType::Int, {std::nullopt, std::nullopt});
  const auto& dict_segment_int_empty = BaseTest::create_dict_segment_by_type<int32_t>(DataType::Int, {});
  const auto& index_int_empty =
      std::make_shared<GroupKeyIndex>(std::vector<std::shared_ptr<const BaseSegment>>({dict_segment_int_empty}));
  const auto& index_int_no_nulls =
      std::make_shared<GroupKeyIndex>(std::vector<std::shared_ptr<const BaseSegment>>({dict_segment_int_no_nulls}));
  const auto& index_int_nulls =
      std::make_shared<GroupKeyIndex>(std::vector<std::shared_ptr<const BaseSegment>>({dict_segment_int_nulls}));

  // A2, B1, C1
  // expected memory consumption:
  //  - `_indexed_segments`, shared pointer               ->  16 bytes
  //  - `_index_offsets`                                  ->  24 bytes
  //  - `_index_offsets`, 7 elements, each 4 bytes        ->  28 bytes
  //  - `_index_postings`                                 ->  24 bytes
  //  - `_index_postings`, 8 elements, each 4 bytes       ->  32 bytes
  //  - `_null_positions`                            ->  24 bytes
  //  - `_null_positions`, 4 elements, each 4 bytes  ->  16 bytes
  //  - `_type`                                           ->   1 byte
  //  - sum                                               >> 165 bytes
  EXPECT_EQ(index->memory_consumption(), 165u);

  // A2, B1, C2
  // expected memory consumption:
  //  - `_indexed_segments`, shared pointer               ->  16 bytes
  //  - `_index_offsets`                                  ->  24 bytes
  //  - `_index_offsets`, 1 elements, each 4 bytes        ->   4 bytes
  //  - `_index_postings`                                 ->  24 bytes
  //  - `_index_postings`, 0 elements, each 4 bytes       ->   0 bytes
  //  - `_null_positions`                            ->  24 bytes
  //  - `_null_positions`, 2 elements, each 4 bytes  ->   8 bytes
  //  - `_type`                                           ->   1 byte
  //  - sum                                               >> 101 bytes
  EXPECT_EQ(index_int_nulls->memory_consumption(), 101u);

  // A2, B2, C1
  // expected memory consumption:
  //  - `_indexed_segments`, shared pointer               ->  16 bytes
  //  - `_index_offsets`                                  ->  24 bytes
  //  - `_index_offsets`, 3 elements, each 4 bytes        ->  12 bytes
  //  - `_index_postings`                                 ->  24 bytes
  //  - `_index_postings`, 2 elements, each 4 bytes       ->   8 bytes
  //  - `_null_positions`                            ->  24 bytes
  //  - `_null_positions`, 0 elements, each 4 bytes  ->   0 bytes
  //  - `_type`                                           ->   1 byte
  //  - sum                                               >> 109 bytes
  EXPECT_EQ(index_int_no_nulls->memory_consumption(), 109u);

  // A1, B2, C2
  // expected memory consumption:
  //  - `_indexed_segments`, shared pointer               ->  16 bytes
  //  - `_index_offsets`                                  ->  24 bytes
  //  - `_index_offsets`, 1 elements, each 4 bytes        ->   4 bytes
  //  - `_index_postings`                                 ->  24 bytes
  //  - `_index_postings`, 0 elements, each 4 bytes       ->   0 bytes
  //  - `_null_positions`                            ->  24 bytes
  //  - `_null_positions`, 0 elements, each 4 bytes  ->   0 bytes
  //  - `_type`                                           ->   1 byte
  //  - sum                                               >>  93 bytes
  EXPECT_EQ(index_int_empty->memory_consumption(), 93u);
}

TEST_F(GroupKeyIndexTest, IndexPostings) {
  // check if there are no duplicates in postings
  auto distinct_values = std::unordered_set<ChunkOffset>(index_postings->begin(), index_postings->end());
  EXPECT_TRUE(distinct_values.size() == index_postings->size());

  // check if the correct postings are present for each value-id
  auto expected_postings =
      std::vector<std::unordered_set<ChunkOffset>>{{7}, {8, 9}, {8, 9}, {2, 4}, {2, 4}, {3}, {1}, {10}};
  auto expected_null_positions =
      std::vector<std::unordered_set<ChunkOffset>>{{0, 5, 6, 11}, {0, 5, 6, 11}, {0, 5, 6, 11}, {0, 5, 6, 11}};

  for (size_t i = 0; i < index_postings->size(); ++i) {
    EXPECT_EQ(1u, expected_postings[i].count(index_postings->at(i)));
  }

  for (size_t i = 0; i < index_null_positions->size(); ++i) {
    EXPECT_EQ(1u, expected_null_positions[i].count(index_null_positions->at(i)));
  }
}

TEST_F(GroupKeyIndexTest, IteratorBeginEnd) {
  EXPECT_EQ(index->cbegin(), index_postings->cbegin());
  EXPECT_EQ(index->cend(), index_postings->cbegin() + 8u);
  EXPECT_EQ(index->null_cbegin(), index_null_positions->cbegin());
  EXPECT_EQ(index->null_cend(), index_null_positions->cbegin() + 4u);
  EXPECT_EQ(index->lower_bound({"inbox"}), index_postings->cbegin() + 7u);
  EXPECT_EQ(index->upper_bound({"inbox"}), index_postings->cbegin() + 8u);
  EXPECT_EQ(index->lower_bound({"hyrise"}), index_postings->cbegin() + 7u);
  EXPECT_EQ(index->upper_bound({"hyrise"}), index_postings->cbegin() + 7u);
  EXPECT_EQ(index->lower_bound({"lamp"}), index_postings->cbegin() + 8u);
  EXPECT_EQ(index->upper_bound({"lamp"}), index_postings->cbegin() + 8u);
  EXPECT_THROW(index->lower_bound({NULL_VALUE}), std::logic_error);
  EXPECT_THROW(index->upper_bound({NULL_VALUE}), std::logic_error);
}

}  // namespace opossum
