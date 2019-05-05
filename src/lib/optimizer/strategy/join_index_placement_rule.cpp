#include "join_index_placement_rule.hpp"
#include "logical_query_plan/abstract_lqp_node.hpp"
#include "logical_query_plan/join_node.hpp"
#include "logical_query_plan/logical_plan_root_node.hpp"
#include "logical_query_plan/lqp_utils.hpp"
#include "logical_query_plan/predicate_node.hpp"

namespace opossum {

std::string JoinIndexPlacementRule::name() const { return "JoinIndex Placement Rule"; }

void JoinIndexPlacementRule::apply_to(const std::shared_ptr<AbstractLQPNode>& node) const {
  // TODO(Marcel) remove printing
  std::cout << "JoinIndexPlacementRule" << "\n";

  // Start at the root node and go down the hierarchy
  // If a JoinNode arises
  //   If JoinIndex is applicable
  //     Pull all predicates of the following lower hierarchy (chain of input nodes) over the JoinNode

  // The traversal functions require the existence of a root of the LQP, so make sure we have that
  const auto root_node = node->type == LQPNodeType::Root ? node : LogicalPlanRootNode::make(node);
  std::vector<std::shared_ptr<PredicateNode>> predicates_to_pull_up;
  _place_join_node_recursively(root_node, predicates_to_pull_up);
}

bool JoinIndexPlacementRule::_place_join_node_recursively(
    const std::shared_ptr<AbstractLQPNode>& node, std::vector<std::shared_ptr<PredicateNode>>& predicates_to_pull_up,
    const std::shared_ptr<JoinNode>& latest_join_node) const {
  if (node) {
    std::shared_ptr<JoinNode> updated_latest_join_node;

    if (!(updated_latest_join_node = std::dynamic_pointer_cast<JoinNode>(node))) {
      updated_latest_join_node = latest_join_node;
    }

    const bool left_input_has_join_node =
        _place_join_node_recursively(node->left_input(), predicates_to_pull_up, updated_latest_join_node);
    const bool right_input_has_join_node =
        _place_join_node_recursively(node->right_input(), predicates_to_pull_up, updated_latest_join_node);

    switch (node->type) {
      case LQPNodeType::Predicate: {
        if (latest_join_node) {  // output has a JoinNode recursively
          const auto predicate_node = std::dynamic_pointer_cast<PredicateNode>(node);
          predicates_to_pull_up.emplace_back(predicate_node);
          lqp_remove_node(predicate_node);
        }
        break;
      }
      case LQPNodeType::Join: {
        if (!left_input_has_join_node && !right_input_has_join_node) {
          // TODO(Marcel) insert predicates as output of the node
          // TODO(Marcel) clear predicates_to_pull_up
        }
        return true;
      }
      default: {
        // nothing to do
      }
    }
    return left_input_has_join_node || right_input_has_join_node;
  }  // if(node)
  return false;
}

bool JoinIndexPlacementRule::_is_join_index_applicable_locally(const std::shared_ptr<JoinNode>& join_node) const {
  // - the JoinNode has no JoinNode as input recursively
  // - one of the join tables is smaller than the other table
  // - the smaller table has a maximim rowcount of 1/3*rowcount(larger table)
  // - the greater table has indizes for the join column
  // TODO(Marcel)
  return false;
}

}  // namespace opossum
