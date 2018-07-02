#include "sql_identifier_context_proxy.hpp"

#include "parameter_id_allocator.hpp"
#include "sql_identifier_context.hpp"

namespace opossum {

SQLIdentifierContextProxy::SQLIdentifierContextProxy(
    const std::shared_ptr<SQLIdentifierContext>& wrapped_context,
    const std::shared_ptr<ParameterIDAllocator>& parameter_id_allocator,
    const std::shared_ptr<SQLIdentifierContextProxy>& outer_context_proxy)
    : _wrapped_context(wrapped_context),
      _parameter_id_allocator(parameter_id_allocator),
      _outer_context_proxy(outer_context_proxy) {}

std::shared_ptr<AbstractExpression> SQLIdentifierContextProxy::resolve_identifier_relaxed(
    const SQLIdentifier& identifier) {
  auto expression = _wrapped_context->resolve_identifier_relaxed(identifier);
  if (expression) {
    const auto expression_iter = _accessed_expressions.find(expression);
    auto parameter_id = ParameterID{0};
    if (expression_iter == _accessed_expressions.end()) {
      // Allocate a new ParameterID for this Expression
      parameter_id = _parameter_id_allocator->allocate();
      _accessed_expressions.emplace(expression, parameter_id);
    } else {
      parameter_id = expression_iter->second;
    }

    return std::make_shared<ParameterExpression>(parameter_id, *expression);
  } else {
    if (_outer_context_proxy) return _outer_context_proxy->resolve_identifier_relaxed(identifier);
  }

  return nullptr;
}

const ExpressionUnorderedMap<ParameterID>& SQLIdentifierContextProxy::accessed_expressions() const {
  return _accessed_expressions;
}

}  // namespace opossum
