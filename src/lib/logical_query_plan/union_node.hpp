#pragma once

#include <memory>
#include <string>
#include <vector>

#include "abstract_lqp_node.hpp"
#include "types.hpp"

namespace opossum {

class UnionNode : public EnableMakeForLQPNode<UnionNode>, public AbstractLQPNode {
 public:
  explicit UnionNode(const UnionMode union_mode);

  std::string description() const override;
  const std::vector<std::shared_ptr<AbstractExpression>>& column_expressions() const override;
  std::shared_ptr<TableStatistics> derive_statistics_from(
      const std::shared_ptr<AbstractLQPNode>& left_input,
      const std::shared_ptr<AbstractLQPNode>& right_input) const override;

  const UnionMode union_mode;

 protected:
  std::shared_ptr<AbstractLQPNode> _shallow_copy_impl(LQPNodeMapping& node_mapping) const override;
  bool _shallow_equals_impl(const AbstractLQPNode& rhs, const LQPNodeMapping& node_mapping) const override;
};
}  // namespace opossum
