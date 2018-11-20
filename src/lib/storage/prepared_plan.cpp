#include "prepared_plan.hpp"

#include "expression/expression_utils.hpp"
#include "expression/lqp_select_expression.hpp"
#include "expression/parameter_expression.hpp"
#include "logical_query_plan/abstract_lqp_node.hpp"
#include "logical_query_plan/lqp_utils.hpp"

namespace {

using namespace opossum;  // NOLINT

void lqp_bind_placeholders_impl(const std::shared_ptr<AbstractLQPNode>& lqp,
                                const std::unordered_map<ParameterID, std::shared_ptr<AbstractExpression>>& parameters,
                                std::unordered_set<std::shared_ptr<AbstractLQPNode>>& visited_nodes);

void expression_bind_placeholders_impl(
    std::shared_ptr<AbstractExpression>& expression,
    const std::unordered_map<ParameterID, std::shared_ptr<AbstractExpression>>& parameters,
    std::unordered_set<std::shared_ptr<AbstractLQPNode>>& visited_nodes) {
  visit_expression(expression, [&](auto& sub_expression) {
    if (const auto parameter_expression = std::dynamic_pointer_cast<ParameterExpression>(sub_expression);
        parameter_expression &&
        parameter_expression->parameter_expression_type == ParameterExpressionType::ValuePlaceholder) {
      const auto parameter_iter = parameters.find(parameter_expression->parameter_id);
      Assert(parameter_iter != parameters.end(),
             "No expression specified for ValuePlaceholder. This should have been caught earlier");
      sub_expression = parameter_iter->second;

      return ExpressionVisitation::DoNotVisitArguments;
    } else if (const auto select_expression = std::dynamic_pointer_cast<LQPSelectExpression>(sub_expression)) {
      lqp_bind_placeholders_impl(select_expression->lqp, parameters, visited_nodes);
    }

    return ExpressionVisitation::VisitArguments;
  });
}

void lqp_bind_placeholders_impl(const std::shared_ptr<AbstractLQPNode>& lqp,
                                const std::unordered_map<ParameterID, std::shared_ptr<AbstractExpression>>& parameters,
                                std::unordered_set<std::shared_ptr<AbstractLQPNode>>& visited_nodes) {
  visit_lqp(lqp, [&](const auto& node) {
    if (!visited_nodes.emplace(node).second) return LQPVisitation::DoNotVisitInputs;

    for (auto& expression : node->node_expressions) {
      expression_bind_placeholders_impl(expression, parameters, visited_nodes);
    }

    return LQPVisitation::VisitInputs;
  });
}

}  // namespace

namespace opossum {

PreparedPlan::PreparedPlan(const std::shared_ptr<AbstractLQPNode>& lqp, const std::vector<ParameterID>& parameter_ids)
    : lqp(lqp), parameter_ids(parameter_ids) {}

std::shared_ptr<PreparedPlan> PreparedPlan::deep_copy() const {
  const auto lqp_copy = lqp->deep_copy();
  return std::make_shared<PreparedPlan>(lqp_copy, parameter_ids);
}

void PreparedPlan::print(std::ostream& stream) const {
  stream << "ParameterIDs: [";
  for (auto parameter_idx = size_t{0}; parameter_idx < parameter_ids.size(); ++parameter_idx) {
    stream << parameter_ids[parameter_idx];
    if (parameter_idx + 1 < parameter_ids.size()) stream << ", ";
  }
  stream << "]\n";
  lqp->print(stream);
}

std::shared_ptr<AbstractLQPNode> PreparedPlan::instantiate(
    const std::vector<std::shared_ptr<AbstractExpression>>& parameters) const {
  Assert(parameters.size() == parameter_ids.size(), "Incorrect number of parameters supplied");

  auto parameters_by_id = std::unordered_map<ParameterID, std::shared_ptr<AbstractExpression>>{};
  for (auto parameter_idx = size_t{0}; parameter_idx < parameters.size(); ++parameter_idx) {
    const auto parameter_id = parameter_ids[parameter_idx];
    parameters_by_id.emplace(parameter_id, parameters[parameter_idx]);
  }

  auto instantiated_lqp = lqp->deep_copy();

  auto visited_nodes = std::unordered_set<std::shared_ptr<AbstractLQPNode>>{};
  lqp_bind_placeholders_impl(instantiated_lqp, parameters_by_id, visited_nodes);

  return instantiated_lqp;
}

bool PreparedPlan::operator==(const PreparedPlan& rhs) const {
  return *lqp == *rhs.lqp && parameter_ids == rhs.parameter_ids;
}

}  // namespace opossum
