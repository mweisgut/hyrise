#include "dummy_table_node.hpp"

#include <optional>
#include <string>
#include <vector>

#include "expression/value_expression.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

DummyTableNode::DummyTableNode() : AbstractLQPNode(LQPNodeType::DummyTable) {}

std::string DummyTableNode::description() const { return "[DummyTable]"; }

const std::vector<std::shared_ptr<AbstractExpression>>& DummyTableNode::column_expressions() const {
  return _column_expressions;
}

std::shared_ptr<AbstractLQPNode> DummyTableNode::_shallow_copy_impl(LQPNodeMapping& node_mapping) const {
  return std::make_shared<DummyTableNode>();
}

bool DummyTableNode::_shallow_equals_impl(const AbstractLQPNode& rhs, const LQPNodeMapping& node_mapping) const {
  return true;
}

}  // namespace opossum
