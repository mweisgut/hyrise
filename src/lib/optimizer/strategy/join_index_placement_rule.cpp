#include "join_index_placement_rule.hpp"
#include "expression/expression_utils.hpp"
#include "logical_query_plan/abstract_lqp_node.hpp"
#include "logical_query_plan/join_node.hpp"
#include "logical_query_plan/logical_plan_root_node.hpp"
#include "logical_query_plan/lqp_utils.hpp"
#include "logical_query_plan/predicate_node.hpp"
#include "statistics/table_statistics.hpp"

namespace opossum {

std::string JoinIndexPlacementRule::name() const { return "JoinIndex Placement Rule"; }

void JoinIndexPlacementRule::apply_to(const std::shared_ptr<AbstractLQPNode>& node) const {
  // Start at the root node and go down the hierarchy
  // If a JoinNode arises
  //   If JoinIndex is applicable
  //     Pull all predicates of the following lower hierarchy (chain of input nodes) over the JoinNode

  // The traversal functions require the existence of a root of the LQP, so make sure we have that
  const auto root_node = node->type == LQPNodeType::Root ? node : LogicalPlanRootNode::make(node);

  const auto expected_column_order = root_node->column_expressions();

  std::vector<std::shared_ptr<PredicateNode>> predicates_to_pull_up;
  _place_join_node_recursively(root_node, LQPInputSide::Left, predicates_to_pull_up);

  // Join ordering might change the output column order, let's fix that
  if (!expressions_equal(expected_column_order, root_node->left_input()->column_expressions())) {
    const auto column_order_correction_node = ProjectionNode::make(expected_column_order, root_node->left_input());
    root_node->set_left_input(column_order_correction_node);
  }

  std::cout << *root_node << std::endl;
}

bool JoinIndexPlacementRule::_place_join_node_recursively(
    const std::shared_ptr<AbstractLQPNode>& node, const LQPInputSide input_side,
    std::vector<std::shared_ptr<PredicateNode>>& predicates_to_pull_up,
    const std::shared_ptr<JoinNode>& latest_join_node) const {
  const auto input_node = node->input(input_side);
  if (!input_node) return false;  // Allow calling without checks

  if (input_node) {
    std::shared_ptr<JoinNode> updated_latest_join_node;

    if (!(updated_latest_join_node = std::dynamic_pointer_cast<JoinNode>(input_node))) {
      updated_latest_join_node = latest_join_node;
    }

    const bool left_input_has_join_node =
        _place_join_node_recursively(input_node, LQPInputSide::Left, predicates_to_pull_up, updated_latest_join_node);
    const bool right_input_has_join_node =
        _place_join_node_recursively(input_node, LQPInputSide::Right, predicates_to_pull_up, updated_latest_join_node);

    switch (input_node->type) {
      case LQPNodeType::Predicate: {
        if (latest_join_node) {  // output has a JoinNode recursively
          const auto predicate_node = std::dynamic_pointer_cast<PredicateNode>(input_node);
          predicates_to_pull_up.emplace_back(predicate_node);
          lqp_remove_node(predicate_node);
        }
        break;
      }
      case LQPNodeType::Join: {
        const auto& input_join_node = std::dynamic_pointer_cast<JoinNode>(input_node);
        if (!left_input_has_join_node && !right_input_has_join_node &&
            _is_join_index_applicable_locally(input_join_node) && !predicates_to_pull_up.empty()) {
          // the JoinNode has no JoinNode as input recursively
          Assert(input_node->output_count() == 1, "A join node is expected to have exactly one output node.");
          auto output_node = input_node->outputs().front();
          for (const auto& predicate : predicates_to_pull_up) {
            output_node->set_input(input_side, predicate);
            output_node = predicate;
          }
          output_node->set_input(input_side, input_node);
          predicates_to_pull_up.clear();
          // TODO(Marcel): Add a flag to the Join Node which helps the LQPTranslator
          // to decide whether an index join shall be used.
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
  const auto& left_input_node = join_node->left_input();
  const auto& right_input_node = join_node->right_input();
  Assert(left_input_node && right_input_node, "A JoinNode is expected to have two input nodes.");
  const auto& left_input_row_count = left_input_node->get_statistics()->approx_valid_row_count();
  const auto& right_input_row_count = right_input_node->get_statistics()->approx_valid_row_count();

  std::cout << "JoinIndex applicable?"
            << "\n";
  // std::cout << "left row count: " << left_input_row_count << "\n";
  // std::cout << "right row count: " << right_input_row_count << "\n";

  // TODO(Marcel) we have to do further experiments to find the threshold or
  // generally the conditions for using an index join.
  float size_factor = 0.2f;
  if (left_input_row_count < size_factor * right_input_row_count ||
      right_input_row_count < size_factor * left_input_row_count) {
    // - one of the join tables is smaller than the other table
    // - the smaller table has a maximim rowcount of size_factor*rowcount(larger table)
    if (true) {
      // - the greater table has indizes for the join column
      // TODO(Marcel)
      std::cout << "YES!" << std::endl;
      return true;
    }
  }
  return false;
}

}  // namespace opossum
