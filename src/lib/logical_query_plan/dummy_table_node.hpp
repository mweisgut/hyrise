#pragma once

#include <optional>
#include <string>
#include <vector>

#include "abstract_lqp_node.hpp"

namespace opossum {

/**
 * This node type represents a dummy table that is used to project literals.
 * See Projection::DummyTable for more details.
 */
class DummyTableNode : public EnableMakeForLQPNode<DummyTableNode>, public AbstractLQPNode {
 public:
  DummyTableNode();

  std::string description() const override;

  const std::vector<std::shared_ptr<AbstractExpression>>& column_expressions() const override;

 protected:
  std::shared_ptr<AbstractLQPNode> _shallow_copy_impl(LQPNodeMapping& node_mapping) const override;
  bool _shallow_equals_impl(const AbstractLQPNode& rhs, const LQPNodeMapping& node_mapping) const override;

 private:
  std::vector<std::shared_ptr<AbstractExpression>> _column_expressions;
};

}  // namespace opossum
