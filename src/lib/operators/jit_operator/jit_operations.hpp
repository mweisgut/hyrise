#pragma once

#include <boost/preprocessor/seq/for_each_product.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

#include <cmath>

#include "jit_types.hpp"
#include "operators/table_scan/like_table_scan_impl.hpp"
#include "resolve_type.hpp"

namespace opossum {

/* This file contains the type dispatching mechanisms that allow generic operations on JitTupleValues.
 *
 * Each binary operation takes three JitTupleValues as parameters: a left input (lhs), a right input (rhs) and an
 * output (result). Each value has one of the supported data types and can be
 * nullable or non-nullable. This leaves us with (number_of_datatypes * 2) ^ 2 combinations for each operation.
 *
 * To make things easier, all arithmetic and comparison operations can be handled the same way:
 * A set of generic lambdas defines type-independent versions of these operations. These lambdas can be passed to the
 * "jit_compute" function to perform the actual computation. The lambdas work on raw, concrete values. It is the
 * responsibility of "jit_compute" to take care of NULL values, unpack input values and pack the result value.
 * This way all NULL-value semantics are kept in one place. If either of the inputs is NULL, the result of the
 * computation is also NULL. If neither input is NULL, the computation lambda is called.
 *
 * Inside "jit_compute", a switch statement (generated by the preprocessor) dispatches the 36 data-type combinations
 * and calls the lambda with the appropriately typed parameters. Invalid type combinations (e.g. adding an int32_t
 * to a std::string) are handled via the SFINAE pattern (substitution failure is not an error): The lambdas fail the
 * template substitution if they cannot perform their operation on a given combination of input types. In this case,
 * the "InvalidTypeCatcher" provides a default implementation that throws an exception.
 *
 * The generic lambdas can also be passed to the "jit_compute_type" function. The function uses the same dispatching
 * mechanisms. But instead of executing a computation, it only determines the result type the computation would
 * have if it were carried out. This functionality is used to determine the type of intermediate values and
 * computed output columns.
 *
 * Logical operators, IsNull and IsNotNull must be handled separately, since their NULL value semantics are
 * different (i.e. a NULL as either input does not result in the output being NULL as well).
 */

#define JIT_GET_ENUM_VALUE(index, s) APPEND_ENUM_NAMESPACE(_, _, BOOST_PP_TUPLE_ELEM(3, 1, BOOST_PP_SEQ_ELEM(index, s)))
#define JIT_GET_DATA_TYPE(index, s) BOOST_PP_TUPLE_ELEM(3, 0, BOOST_PP_SEQ_ELEM(index, s))

#define JIT_COMPUTE_CASE(r, types)                                                                                   \
  case static_cast<uint8_t>(JIT_GET_ENUM_VALUE(0, types)) << 8 | static_cast<uint8_t>(JIT_GET_ENUM_VALUE(1, types)): \
    catching_func(lhs.get<JIT_GET_DATA_TYPE(0, types)>(context), rhs.get<JIT_GET_DATA_TYPE(1, types)>(context),      \
                  result);                                                                                           \
    break;

#define JIT_COMPUTE_TYPE_CASE(r, types)                                                                              \
  case static_cast<uint8_t>(JIT_GET_ENUM_VALUE(0, types)) << 8 | static_cast<uint8_t>(JIT_GET_ENUM_VALUE(1, types)): \
    return catching_func(JIT_GET_DATA_TYPE(0, types)(), JIT_GET_DATA_TYPE(1, types)());

/* Arithmetic operators */
const auto jit_addition = [](const auto a, const auto b) -> decltype(a + b) { return a + b; };
const auto jit_subtraction = [](const auto a, const auto b) -> decltype(a - b) { return a - b; };
const auto jit_multiplication = [](const auto a, const auto b) -> decltype(a * b) { return a * b; };
const auto jit_division = [](const auto a, const auto b) -> decltype(a / b) { return a / b; };
const auto jit_modulo = [](const auto a, const auto b) -> decltype(a % b) { return a % b; };
const auto jit_power = [](const auto a, const auto b) -> decltype(std::pow(a, b)) { return std::pow(a, b); };

/* Comparison operators */
const auto jit_equals = [](const auto a, const auto b) -> decltype(a == b) { return a == b; };
const auto jit_not_equals = [](const auto a, const auto b) -> decltype(a != b) { return a != b; };
const auto jit_less_than = [](const auto a, const auto b) -> decltype(a < b) { return a < b; };
const auto jit_less_than_equals = [](const auto a, const auto b) -> decltype(a <= b) { return a <= b; };
const auto jit_greater_than = [](const auto a, const auto b) -> decltype(a > b) { return a > b; };
const auto jit_greater_than_equals = [](const auto a, const auto b) -> decltype(a >= b) { return a >= b; };

const auto jit_like = [](const std::string a, const std::string b) -> bool {
  const auto regex_string = LikeTableScanImpl::sqllike_to_regex(b);
  const auto regex = std::regex{regex_string, std::regex_constants::icase};
  return std::regex_match(a, regex);
};

const auto jit_not_like = [](const std::string a, const std::string b) -> bool {
  const auto regex_string = LikeTableScanImpl::sqllike_to_regex(b);
  const auto regex = std::regex{regex_string, std::regex_constants::icase};
  return !std::regex_match(a, regex);
};

// The InvalidTypeCatcher acts as a fallback implementation, if template specialization
// fails for a type combination.
template <typename Functor, typename Result>
struct InvalidTypeCatcher : Functor {
  explicit InvalidTypeCatcher(Functor f) : Functor(std::move(f)) {}

  using Functor::operator();

  template <typename... Ts>
  Result operator()(const Ts...) const {
    throw "";
    //Fail("Invalid combination of types for operation.");
  }
};

// We do not want to inline here, because:
// These function tend to get quite complex due to the large switch statement. If we inline this function, this means a
// lot of work for the JIT compiler. If we let the JIT compiler do the inlining instead, it is able to prune the
// function to the relevant case during inlining. This allows for faster jitting.
template <typename T>
void jit_compute(const T& op_func, const JitTupleValue& lhs, const JitTupleValue& rhs, const JitTupleValue& result,
                 JitRuntimeContext& context) {
  // Handle NULL values and return if either input is NULL.
  const bool result_is_null = lhs.is_null(context) || rhs.is_null(context);
  result.set_is_null(result_is_null, context);
  if (result_is_null) {
    return;
  }

  // This lambda calls the op_func (a lambda that performs the actual computation) with type arguments and stores
  // the result.
  const auto store_result_wrapper = [&](const auto& typed_lhs, const auto& typed_rhs, auto& result) -> decltype(
      op_func(typed_lhs, typed_rhs), void()) {
    using ResultType = decltype(op_func(typed_lhs, typed_rhs));
    result.template set<ResultType>(op_func(typed_lhs, typed_rhs), context);
  };

  const auto catching_func = InvalidTypeCatcher<decltype(store_result_wrapper), void>(store_result_wrapper);

  // The type information from the lhs and rhs are combined into a single value for dispatching without nesting.
  const auto combined_types = static_cast<uint8_t>(lhs.data_type()) << 8 | static_cast<uint8_t>(rhs.data_type());
  switch (combined_types) { BOOST_PP_SEQ_FOR_EACH_PRODUCT(JIT_COMPUTE_CASE, (JIT_DATA_TYPE_INFO)(JIT_DATA_TYPE_INFO)) }
}

template <typename T>
DataType jit_compute_type(const T& op_func, const DataType lhs, const DataType rhs) {
  // This lambda calls the op_func (a lambda that could performs the actual computation) and determines the return type
  // of that lambda.
  const auto determine_return_type_wrapper = [&](const auto& typed_lhs, const auto& typed_rhs) -> decltype(
      op_func(typed_lhs, typed_rhs), DataType()) {
    using ResultType = decltype(op_func(typed_lhs, typed_rhs));
    // This templated function returns the DataType enum value for a given ResultType.
    return data_type_from_type<ResultType>();
  };

  const auto catching_func =
      InvalidTypeCatcher<decltype(determine_return_type_wrapper), DataType>(determine_return_type_wrapper);

  // The type information from the lhs and rhs are combined into a single value for dispatching without nesting.
  const auto combined_types = static_cast<uint8_t>(lhs) << 8 | static_cast<uint8_t>(rhs);
  switch (combined_types) {
    BOOST_PP_SEQ_FOR_EACH_PRODUCT(JIT_COMPUTE_TYPE_CASE, (JIT_DATA_TYPE_INFO)(JIT_DATA_TYPE_INFO))
    default:
      return DataType::Null;  // unreachable
  }
}

void jit_not(const JitTupleValue& lhs, const JitTupleValue& result, JitRuntimeContext& context);
void jit_and(const JitTupleValue& lhs, const JitTupleValue& rhs, const JitTupleValue& result,
             JitRuntimeContext& context);
void jit_or(const JitTupleValue& lhs, const JitTupleValue& rhs, const JitTupleValue& result,
            JitRuntimeContext& context);
void jit_is_null(const JitTupleValue& lhs, const JitTupleValue& result, JitRuntimeContext& context);
void jit_is_not_null(const JitTupleValue& lhs, const JitTupleValue& result, JitRuntimeContext& context);

// cleanup
#undef JIT_GET_ENUM_VALUE
#undef JIT_GET_DATA_TYPE
#undef JIT_COMPUTE_CASE
#undef JIT_COMPUTE_TYPE_CASE

}  // namespace opossum
