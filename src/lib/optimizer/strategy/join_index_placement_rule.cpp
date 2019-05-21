#include "join_index_placement_rule.hpp"
#include "expression/expression_utils.hpp"
#include "expression/lqp_column_expression.hpp"
#include "logical_query_plan/abstract_lqp_node.hpp"
#include "logical_query_plan/join_node.hpp"
#include "logical_query_plan/logical_plan_root_node.hpp"
#include "logical_query_plan/lqp_column_reference.hpp"
#include "logical_query_plan/lqp_utils.hpp"
#include "logical_query_plan/predicate_node.hpp"
#include "operators/operator_join_predicate.hpp"
#include "statistics/table_statistics.hpp"
#include "types.hpp"

namespace opossum {

// TODO(Marcel) we have to do further experiments to find the threshold or
// generally the conditions for using an index join.
constexpr float INDEX_JOIN_RATIO_THRESHOLD = 0.01f;

std::string JoinIndexPlacementRule::name() const { return "JoinIndex Placement Rule"; }

void JoinIndexPlacementRule::apply_to(const std::shared_ptr<AbstractLQPNode>& node) const {
  // The traversal functions require the existence of a root of the LQP, so make sure we have that
  const auto root_node = node->type == LQPNodeType::Root ? node : LogicalPlanRootNode::make(node);

  std::vector<std::shared_ptr<PredicateNode>> left_predicates_to_pull_up;
  std::vector<std::shared_ptr<PredicateNode>> right_predicates_to_pull_up;

  _place_join_node_recursively(root_node, LQPInputSide::Left, left_predicates_to_pull_up, right_predicates_to_pull_up);
}

bool JoinIndexPlacementRule::_place_join_node_recursively(
    const std::shared_ptr<AbstractLQPNode>& node, const LQPInputSide input_side,
    std::vector<std::shared_ptr<PredicateNode>>& left_predicates_to_pull_up,
    std::vector<std::shared_ptr<PredicateNode>>& right_predicates_to_pull_up,
    const std::shared_ptr<JoinNode>& latest_join_node) const {
  const auto input_node = node->input(input_side);

  if (input_node) {
    std::shared_ptr<JoinNode> updated_latest_join_node = std::dynamic_pointer_cast<JoinNode>(input_node);

    if (updated_latest_join_node) {
      left_predicates_to_pull_up.clear();
      right_predicates_to_pull_up.clear();
    } else {
      updated_latest_join_node = latest_join_node;
    }

    const bool is_join_in_left_subtree =
        _place_join_node_recursively(input_node, LQPInputSide::Left, left_predicates_to_pull_up,
                                     right_predicates_to_pull_up, updated_latest_join_node);
    const bool is_join_in_right_subtree =
        _place_join_node_recursively(input_node, LQPInputSide::Right, left_predicates_to_pull_up,
                                     right_predicates_to_pull_up, updated_latest_join_node);

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
          if (input_side == LQPInputSide::Left) {
            left_predicates_to_pull_up.emplace_back(predicate_node);
          } else if (input_side == LQPInputSide::Right) {
            right_predicates_to_pull_up.emplace_back(predicate_node);
          }
        }
        break;
      }
      case LQPNodeType::Join: {
        if (!is_join_in_subtrees && (!left_predicates_to_pull_up.empty() || !right_predicates_to_pull_up.empty())) {
          const auto applicability_result = _is_join_index_applicable_locally(updated_latest_join_node);
          if (applicability_result.primary_index_side != JoinInputSide::None) {
            updated_latest_join_node->set_index_primary_table_side(applicability_result.primary_index_side);

            // the JoinNode has no JoinNode as input recursively
            Assert(input_node->output_count() == 1, "A join node is expected to have exactly one output node.");

            // build predicates chain
            const auto pull_up_chain_root = LogicalPlanRootNode::make();
            std::shared_ptr<AbstractLQPNode> pull_up_chain_end = pull_up_chain_root;

            if (applicability_result.pull_up_left_predicates) {
              for (const auto& predicate_node : left_predicates_to_pull_up) {
                lqp_remove_node(predicate_node);
                pull_up_chain_end->set_left_input(predicate_node);
                pull_up_chain_end = predicate_node;
              }
            }

            if (applicability_result.pull_up_right_predicates) {
              for (const auto& predicate_node : right_predicates_to_pull_up) {
                lqp_remove_node(predicate_node);
                pull_up_chain_end->set_left_input(predicate_node);
                pull_up_chain_end = predicate_node;
              }
            }

            // link the chain into the LQP
            node->set_input(input_side, pull_up_chain_root->left_input());
            pull_up_chain_end->set_left_input(updated_latest_join_node);
            left_predicates_to_pull_up.clear();
            right_predicates_to_pull_up.clear();
          }
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

JoinIndexApplicabilityResult JoinIndexPlacementRule::_is_join_index_applicable_locally(
    const std::shared_ptr<JoinNode>& join_node) const {
  const auto& left_input_node = join_node->left_input();
  const auto& right_input_node = join_node->right_input();

  // Let's get the StoredTable nodes of the join input node chains.
  // Since this function is only called, when no additional join node exists as input recursively,
  // we can assume that each input node has only one left input node.
  std::shared_ptr<const AbstractLQPNode> left_input_stored_table_node;
  std::shared_ptr<const AbstractLQPNode> right_input_stored_table_node;

  Assert(left_input_node && right_input_node, "A JoinNode is expected to have two input nodes.");
  Assert(!left_input_node->column_expressions().empty(), "Column expressions must not be empty.");
  Assert(!right_input_node->column_expressions().empty(), "Column expressions must not be empty.");

  if(const auto lqp_column_expression = std::dynamic_pointer_cast<LQPColumnExpression>(left_input_node->column_expressions().front())){
    left_input_stored_table_node = lqp_column_expression->column_reference.original_node();
  }else{
    return JoinIndexApplicabilityResult{JoinInputSide::None, false, false};
  }

  if(const auto lqp_column_expression = std::dynamic_pointer_cast<LQPColumnExpression>(right_input_node->column_expressions().front())){
    right_input_stored_table_node = lqp_column_expression->column_reference.original_node();
  }else{
    return JoinIndexApplicabilityResult{JoinInputSide::None, false, false};
  }

  const auto predicate_expressions = join_node->join_predicates();
  // for now, only place a JoinIndex for inner single predicate joins
  if (join_node->join_mode == JoinMode::Inner && predicate_expressions.size() == 1) {
    auto join_predicate = OperatorJoinPredicate::from_expression(*predicate_expressions[0], *join_node->left_input(),
                                                                 *join_node->right_input());
    if (join_predicate) {
      const auto left_stored_table_row_count = left_input_stored_table_node->get_statistics()->row_count();
      const auto right_stored_table_row_count = right_input_stored_table_node->get_statistics()->row_count();
      const auto left_row_count = left_input_node->get_statistics()->row_count();
      const auto right_row_count = right_input_node->get_statistics()->row_count();
      // case 1: left_stored_table_row_count < THRESHOLD * right_stored_table_row_count
      // case 2: right_stored_table_row_count < THRESHOLD * left_stored_table_row_count
      // case 3: left_row_count < THRESHOLD * right_stored_table_row_count
      // case 4: right_row_count < THRESHOLD * left_stored_table_row_count
      std::cout << "case 1 ratio: " << left_stored_table_row_count / right_stored_table_row_count << "\n";
      std::cout << "case 2 ratio: " << right_stored_table_row_count / left_stored_table_row_count << "\n";
      std::cout << "case 3 ratio: " << left_row_count / right_stored_table_row_count << "\n";
      std::cout << "case 4 ratio: " << right_row_count / left_stored_table_row_count << "\n";

      if (left_stored_table_row_count < INDEX_JOIN_RATIO_THRESHOLD * right_stored_table_row_count &&
          _is_index_on_join_column(right_input_stored_table_node, join_predicate->column_ids.second)) {
        std::cout << "CASE 1"
                  << "\n";
        return JoinIndexApplicabilityResult{JoinInputSide::Right, true, true};

      } else if (right_stored_table_row_count < INDEX_JOIN_RATIO_THRESHOLD * left_stored_table_row_count &&
                 _is_index_on_join_column(left_input_stored_table_node, join_predicate->column_ids.first)) {
        std::cout << "CASE 2"
                  << "\n";
        return JoinIndexApplicabilityResult{JoinInputSide::Left, true, true};

      } else if (left_row_count < INDEX_JOIN_RATIO_THRESHOLD * right_stored_table_row_count &&
                 _is_index_on_join_column(right_input_stored_table_node, join_predicate->column_ids.second)) {
        std::cout << "CASE 3"
                  << "\n";
        return JoinIndexApplicabilityResult{JoinInputSide::Right, false, true};

      } else if (right_row_count < INDEX_JOIN_RATIO_THRESHOLD * left_stored_table_row_count &&
                 _is_index_on_join_column(left_input_stored_table_node, join_predicate->column_ids.first)) {
        std::cout << "CASE 4"
                  << "\n";
        return JoinIndexApplicabilityResult{JoinInputSide::Left, true, false};
      }
    }
  }
  return JoinIndexApplicabilityResult{JoinInputSide::None, false, false};
}

bool JoinIndexPlacementRule::_is_index_on_join_column(const std::shared_ptr<const AbstractLQPNode>& stored_table_node,
                                                      const ColumnID join_column_id) const {
  Assert(stored_table_node->type == LQPNodeType::StoredTable, "Passed node has to be of type StoredTable");
  std::cout << "is index on join column id " << join_column_id << "?\n";
  for (const auto& index_statistics : stored_table_node->get_statistics()->index_statistics()) {
    const auto index_column_ids = index_statistics.column_ids;
    std::cout << "index" << std::endl;
    if (index_column_ids.size() == 1) {
      std::cout << "index col ids: " << index_column_ids[0] << "\n";
    }
    if (index_column_ids.size() == 1 && index_column_ids[0] == join_column_id) {
      return true;
    }
  }

  std::cout << "NO INDEX AVAILABLE"
            << "\n";
  return false;
}

}  // namespace opossum
