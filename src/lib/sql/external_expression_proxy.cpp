#include "external_expression_proxy.hpp"

#include "qualified_column_name_lookup.hpp"
#include "expression/external_expression.hpp"

namespace opossum {

ExternalExpressionProxy::ExternalExpressionProxy(const std::shared_ptr<QualifiedColumnNameLookup>& expression_lookup):
  _expression_lookup(expression_lookup) {}

std::shared_ptr<AbstractExpression> ExternalExpressionProxy::get(const QualifiedColumnName& qualified_column_name) {
  const auto external_expression = _expression_lookup->get(qualified_column_name);

  auto internal_expression_iter = _referenced_external_expressions.find(external_expression);
  if (internal_expression_iter == _referenced_external_expressions.end()) {
    const auto internal_expression = std::make_shared<ExternalExpression>();
    internal_expression_iter = _referenced_external_expressions.emplace(external_expression, internal_expression).first;
  }

  return internal_expression_iter->second;
}

const ExternalExpressionProxy::ExternalExpressions& ExternalExpressionProxy::referenced_external_expressions() const {
  return _referenced_external_expressions;
}

}  // namespace opossum
