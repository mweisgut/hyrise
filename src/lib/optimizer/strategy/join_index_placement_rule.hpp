#pragma once

#include <memory>
#include <string>

#include "abstract_rule.hpp"
#include "logical_query_plan/abstract_lqp_node.hpp"
#include "storage/index/index_statistics.hpp"
#include "storage/table.hpp"

namespace opossum {

class AbstractLQPNode;
class JoinNode;
class PredicateNode;

class JoinIndexPlacementRule : public AbstractRule {
 public:
  std::string name() const override;
  void apply_to(const std::shared_ptr<AbstractLQPNode>& node) const override;

 protected:
  // return values indicates whether the left input tree contains a JoinNode
  bool _place_join_node_recursively(const std::shared_ptr<AbstractLQPNode>& node, const LQPInputSide input_side,
                                    std::vector<std::shared_ptr<PredicateNode>>& predicates_to_pull_up,
                                    const std::shared_ptr<JoinNode>& latest_join_node = nullptr) const;
  bool _is_join_index_applicable_locally(const std::shared_ptr<JoinNode>& join_node) const;
  bool _is_index_on_join_column(const TableStatistics& indexed_table_statistics, const ColumnID join_column_id) const;
};

}  // namespace opossum
