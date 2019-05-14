#include "join_index_placement_rule.hpp"
#include "expression/expression_utils.hpp"
#include "logical_query_plan/abstract_lqp_node.hpp"
#include "logical_query_plan/join_node.hpp"
#include "logical_query_plan/logical_plan_root_node.hpp"
#include "logical_query_plan/lqp_utils.hpp"
#include "logical_query_plan/predicate_node.hpp"
#include "operators/operator_join_predicate.hpp"
#include "statistics/table_statistics.hpp"
#include "types.hpp"

namespace opossum {

std::string JoinIndexPlacementRule::name() const { return "JoinIndex Placement Rule"; }

void JoinIndexPlacementRule::apply_to(const std::shared_ptr<AbstractLQPNode>& node) const {
  // The traversal functions require the existence of a root of the LQP, so make sure we have that
  const auto root_node = node->type == LQPNodeType::Root ? node : LogicalPlanRootNode::make(node);

  std::vector<std::shared_ptr<PredicateNode>> predicates_to_pull_up;

  _place_join_node_recursively(root_node, LQPInputSide::Left, predicates_to_pull_up);
}

bool JoinIndexPlacementRule::_place_join_node_recursively(
    const std::shared_ptr<AbstractLQPNode>& node, const LQPInputSide input_side,
    std::vector<std::shared_ptr<PredicateNode>>& predicates_to_pull_up,
    const std::shared_ptr<JoinNode>& latest_join_node) const {
  const auto input_node = node->input(input_side);

  if (input_node) {
    std::shared_ptr<JoinNode> updated_latest_join_node = std::dynamic_pointer_cast<JoinNode>(input_node);

    if (updated_latest_join_node) {
      predicates_to_pull_up.clear();
    } else {
      updated_latest_join_node = latest_join_node;
    }

    const bool is_join_in_left_subtree =
        _place_join_node_recursively(input_node, LQPInputSide::Left, predicates_to_pull_up, updated_latest_join_node);
    const bool is_join_in_right_subtree =
        _place_join_node_recursively(input_node, LQPInputSide::Right, predicates_to_pull_up, updated_latest_join_node);

    bool is_join_in_subtrees = is_join_in_left_subtree || is_join_in_right_subtree;

    switch (input_node->type) {
      case LQPNodeType::DummyTable: {
        // work around with very small overheaded for #1500 to not use the cost estimators in function
        // "_is_join_index_applicable_locally" on an LQP containing a DummyTableNode
        return true;
      }
      case LQPNodeType::Predicate: {
        if (latest_join_node && !is_join_in_subtrees) {
          const auto predicate_node = std::dynamic_pointer_cast<PredicateNode>(input_node);
          predicates_to_pull_up.emplace_back(predicate_node);
        }
        break;
      }
      case LQPNodeType::Join: {
        if (!is_join_in_subtrees && !predicates_to_pull_up.empty() &&
            _is_join_index_applicable_locally(updated_latest_join_node)) {
          // the JoinNode has no JoinNode as input recursively
          Assert(input_node->output_count() == 1, "A join node is expected to have exactly one output node.");

          // build predicate chain
          const auto predicate_chain_root = LogicalPlanRootNode::make();
          std::shared_ptr<AbstractLQPNode> predicate_chain_end = predicate_chain_root;

          for (const auto& predicate_node : predicates_to_pull_up) {
            lqp_remove_node(predicate_node);
            predicate_chain_end->set_left_input(predicate_node);
            predicate_chain_end = predicate_node;
          }

          // link the chain into the LQP
          node->set_input(input_side, predicate_chain_root->left_input());
          predicate_chain_end->set_left_input(updated_latest_join_node);
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
    return is_join_in_left_subtree || is_join_in_right_subtree;
  }  // if(input_node)
  return false;
}

bool JoinIndexPlacementRule::_is_join_index_applicable_locally(const std::shared_ptr<JoinNode>& join_node) const {
  const auto& left_input_node = join_node->left_input();
  const auto& right_input_node = join_node->right_input();

  Assert(left_input_node && right_input_node, "A JoinNode is expected to have two input nodes.");
  const auto& left_input_table_statistics = left_input_node->get_statistics();
  const auto& right_input_table_statistics = right_input_node->get_statistics();

  const auto& left_input_row_count = left_input_table_statistics->approx_valid_row_count();
  const auto& right_input_row_count = right_input_table_statistics->approx_valid_row_count();

  // TODO(Marcel) we have to do further experiments to find the threshold or
  // generally the conditions for using an index join.
  float size_factor = 0.2f;
  const auto predicate_expressions = join_node->join_predicates();
  // for now, only place a JoinIndex for inner single predicate joins
  if (join_node->join_mode == JoinMode::Inner && predicate_expressions.size() == 1) {
    auto join_predicate = OperatorJoinPredicate::from_expression(*predicate_expressions[0], *join_node->left_input(),
                                                                 *join_node->right_input());
    if (join_predicate) {
      if (left_input_row_count < size_factor * right_input_row_count &&
          _is_index_on_join_column(*right_input_table_statistics, join_predicate->column_ids.second)) {
        std::cout << "INDEX JOIN APPLICABLE!" << std::endl;
        return true;
      } else if (right_input_row_count < size_factor * left_input_row_count &&
                 _is_index_on_join_column(*left_input_table_statistics, join_predicate->column_ids.first)) {
        std::cout << "INDEX JOIN APPLICABLE!" << std::endl;
        return true;
      }
    }

    // const auto has_index_for_join_column = [&](const auto& table_statistics, ){
    //   for(const auto)
    // }
  }
  return false;
}

bool JoinIndexPlacementRule::_is_index_on_join_column(const TableStatistics& indexed_table_statistics,
                                                      const ColumnID join_column_id) const {
  for (const auto& index_statistics : indexed_table_statistics.index_statistics()) {
    const auto index_column_ids = index_statistics.column_ids;
    if (index_column_ids.size() == 1 && index_column_ids[0] == join_column_id) {
      return true;
    }
  }
  return false;
}

}  // namespace opossum
