#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "SQLParser.h"

#include "all_parameter_variant.hpp"
#include "qualified_column_name_lookup.hpp"

#include "expression/abstract_expression.hpp"
#include "logical_query_plan/abstract_lqp_node.hpp"

namespace opossum {

class AggregateNode;
class QualifiedColumnNameLookup;
class LQPExpression;

struct SQLTranslationState final {
  std::shared_ptr<AbstractLQPNode> lqp;
  std::shared_ptr<QualifiedColumnNameLookup> name_lookup;
};

/**
* Produces an LQP (Logical Query Plan), as defined in src/logical_query_plan/, from an hsql::SQLParseResult.
 *
 * The elements of the vector returned by SQLTranslator::translate_parse_result(const hsql::SQLParserResult&)
 * point to the root/result nodes of the LQPs.
 *
 * An LQP can either be handed to the Optimizer, or it can be directly turned into Operators by the LQPTranslator.
 */
class SQLTranslator final : public Noncopyable {
 public:
  /**
   * @param validate If set to false, does not add validate nodes to the resulting tree.
   */
  explicit constexpr SQLTranslator(bool validate = true) : _validate{validate} {}

  std::vector<std::shared_ptr<AbstractLQPNode>> translate_parse_result(const hsql::SQLParserResult& result);

  std::shared_ptr<AbstractLQPNode> translate_statement(const hsql::SQLStatement& statement);

  std::shared_ptr<AbstractLQPNode> translate_select(const hsql::SelectStatement &select);

 protected:
  SQLTranslationState _translate_table_ref(const hsql::TableRef& hsql_table_ref,
                                           const std::shared_ptr<QualifiedColumnNameLookup>& name_lookup);

  std::shared_ptr<AbstractLQPNode> _translate_select_groupby_having(const hsql::SelectStatement &select,
                                                                    const std::shared_ptr<AbstractLQPNode> &input_node);

  std::shared_ptr<AbstractLQPNode> _translate_order_by(const std::vector<hsql::OrderDescription*>& order_list,
                                                       std::shared_ptr<AbstractLQPNode> current_node);

  std::shared_ptr<AbstractLQPNode> _translate_join(const hsql::JoinDefinition& select);

  std::shared_ptr<AbstractLQPNode> _translate_natural_join(const hsql::JoinDefinition& select);

  std::shared_ptr<AbstractLQPNode> _translate_cross_product(const std::vector<hsql::TableRef*>& tables);

  std::shared_ptr<AbstractLQPNode> _translate_insert(const hsql::InsertStatement& insert);

  std::shared_ptr<AbstractLQPNode> _translate_delete(const hsql::DeleteStatement& del);

  std::shared_ptr<AbstractLQPNode> _translate_update(const hsql::UpdateStatement& update);

  std::shared_ptr<AbstractLQPNode> _translate_create(const hsql::CreateStatement& update);

  std::shared_ptr<AbstractLQPNode> _translate_drop(const hsql::DropStatement& update);

  /**
   * Translate column renamings such as
   *    SELECT (<SUBSELECT>) (name_column_a, name_column_b) ....
   * OR
   *    SELECT ... FROM foo AS bar(name_column_a, name_column_b) ...
   */
  std::shared_ptr<AbstractLQPNode> _translate_column_renamings(const std::shared_ptr<AbstractLQPNode> &node,
                                                               const hsql::TableRef &table);

  std::shared_ptr<AbstractLQPNode> _translate_predicate_expression(
  const std::shared_ptr<AbstractExpression> &expression, std::shared_ptr<AbstractLQPNode> current_node) const;

  std::shared_ptr<AbstractLQPNode> _prune_expressions(const std::shared_ptr<AbstractLQPNode>& node,
                                                              const std::vector<std::shared_ptr<AbstractExpression>>& expressions) const;

  std::shared_ptr<AbstractLQPNode> _add_expression_if_unavailable(const std::shared_ptr<AbstractLQPNode>& node,
                                                              const std::shared_ptr<AbstractExpression>& expression) const;

  std::shared_ptr<AbstractLQPNode> _translate_show(const hsql::ShowStatement& show_statement);

  std::shared_ptr<AbstractLQPNode> _validate_if_active(const std::shared_ptr<AbstractLQPNode>& input_node);

  std::vector<std::shared_ptr<LQPExpression>> _retrieve_having_aggregates(
      const hsql::Expr& expr, const std::shared_ptr<AbstractLQPNode>& input_node);

 private:
  const bool _validate;
};

}  // namespace opossum
