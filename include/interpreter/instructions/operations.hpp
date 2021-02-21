#pragma once

#include <functional>
#include <stdexcept>

#include "types.hpp"

namespace interpreter::instructions {

struct ZeroDivisionError : public ValueError {
  using ValueError::ValueError;
};

using OperationValue = std::variant<Value, Reference>;

template <typename T>
concept SameAsOperationValue = std::same_as<std::decay_t<T>, OperationValue>;

namespace op_type {
struct Op {};
struct Assign : Op {};
struct Plus : Op {};
struct Minus : Op {};
struct Or : Op {};
struct And : Op {};
struct Mul : Op {};
struct Div : Op {};
struct Mod : Op {};
struct Equals : Op {};
struct NotEquals : Op {};
struct Less : Op {};
struct Greater : Op {};
struct LessOrEq : Op {};
struct GreaterOrEq : Op {};

struct Not : Op {};
struct UnaryMinus : Op {};
struct UnaryPlus : Op {};
}  // namespace op_type

namespace details {

namespace operations {

struct NotAllowed {};

template <typename L, typename R>
struct Div {
  constexpr auto operator()(const L& lhs, const R& rhs) const {
    if (rhs == 0) {
      throw ZeroDivisionError{"zero division"};
    }
    return lhs / rhs;
  }
};

#define ADD_DEFAULT_ASSIGN_OPERATION(name, op)              \
  template <typename L, typename R>                         \
  struct name {                                             \
    constexpr auto operator()(L& lhs, const R& rhs) const { \
      return lhs op rhs;                                    \
    }                                                       \
  }

#define ADD_DEFAULT_UN_OPERATION(name, op)                       \
  template <typename T>                                          \
  struct name {                                                  \
    constexpr auto operator()(const T& t) const { return op t; } \
  }

#define ADD_DEFAULT_BIN_OPERATION(name, op)                       \
  template <typename L, typename R>                               \
  struct name {                                                   \
    constexpr auto operator()(const L& lhs, const R& rhs) const { \
      return lhs op rhs;                                          \
    }                                                             \
  }

ADD_DEFAULT_ASSIGN_OPERATION(Assign, =);

ADD_DEFAULT_UN_OPERATION(Not, !);
ADD_DEFAULT_UN_OPERATION(UnaryMinus, -);
ADD_DEFAULT_UN_OPERATION(UnaryPlus, +);

ADD_DEFAULT_BIN_OPERATION(Plus, +);
ADD_DEFAULT_BIN_OPERATION(Minus, -);
ADD_DEFAULT_BIN_OPERATION(Mul, *);
ADD_DEFAULT_BIN_OPERATION(Mod, %);
ADD_DEFAULT_BIN_OPERATION(Equals, ==);
ADD_DEFAULT_BIN_OPERATION(NotEquals, !=);
ADD_DEFAULT_BIN_OPERATION(Less, <);
ADD_DEFAULT_BIN_OPERATION(Greater, >);
ADD_DEFAULT_BIN_OPERATION(LessOrEq, <=);
ADD_DEFAULT_BIN_OPERATION(GreaterOrEq, >=);
ADD_DEFAULT_BIN_OPERATION(Or, ||);
ADD_DEFAULT_BIN_OPERATION(And, &&);

#undef ADD_DEFAULT_ASSIGN_OPERATION
#undef ADD_DEFAULT_BIN_OPERATION
#undef ADD_DEFAULT_UN_OPERATION

}  // namespace operations

}  // namespace details

template <typename Op, WeakValueT... Values>
struct Rule : details::operations::NotAllowed {};

// Sorry about that, no reflection for now
#define ADD_DEFAULT_RULE_1(op, t) \
  template <>                     \
  struct Rule<op_type::op, types::t> : details::operations::op<types::t> {}

// Sorry about that, no reflection for now
#define ADD_DEFAULT_RULE_2(l, op, r)           \
  template <>                                  \
  struct Rule<op_type::op, types::l, types::r> \
      : details::operations::op<types::l, types::r> {}

// arithmetical rules
ADD_DEFAULT_RULE_2(Int, Plus, Int);
ADD_DEFAULT_RULE_2(Int, Minus, Int);
ADD_DEFAULT_RULE_2(Int, Mul, Int);
ADD_DEFAULT_RULE_2(Int, Mod, Int);
ADD_DEFAULT_RULE_2(Int, Div, Int);

ADD_DEFAULT_RULE_2(Real, Plus, Real);
ADD_DEFAULT_RULE_2(Real, Minus, Real);
ADD_DEFAULT_RULE_2(Real, Mul, Real);
ADD_DEFAULT_RULE_2(Real, Div, Real);

ADD_DEFAULT_RULE_2(Real, Plus, Int);
ADD_DEFAULT_RULE_2(Real, Minus, Int);
ADD_DEFAULT_RULE_2(Real, Mul, Int);
ADD_DEFAULT_RULE_2(Real, Div, Int);

ADD_DEFAULT_RULE_2(Int, Plus, Real);
ADD_DEFAULT_RULE_2(Int, Minus, Real);
ADD_DEFAULT_RULE_2(Int, Mul, Real);
ADD_DEFAULT_RULE_2(Int, Div, Real);

ADD_DEFAULT_RULE_2(Str, Plus, Str);

// unary arithmetical rules
ADD_DEFAULT_RULE_1(UnaryPlus, Int);
ADD_DEFAULT_RULE_1(UnaryMinus, Int);

ADD_DEFAULT_RULE_1(UnaryPlus, Real);
ADD_DEFAULT_RULE_1(UnaryMinus, Real);

// compare rules
ADD_DEFAULT_RULE_2(Int, Equals, Int);
ADD_DEFAULT_RULE_2(Int, NotEquals, Int);
ADD_DEFAULT_RULE_2(Int, Less, Int);
ADD_DEFAULT_RULE_2(Int, Greater, Int);
ADD_DEFAULT_RULE_2(Int, LessOrEq, Int);
ADD_DEFAULT_RULE_2(Int, GreaterOrEq, Int);

ADD_DEFAULT_RULE_2(Real, Equals, Real);
ADD_DEFAULT_RULE_2(Real, NotEquals, Real);
ADD_DEFAULT_RULE_2(Real, Less, Real);
ADD_DEFAULT_RULE_2(Real, Greater, Real);
ADD_DEFAULT_RULE_2(Real, LessOrEq, Real);
ADD_DEFAULT_RULE_2(Real, GreaterOrEq, Real);

ADD_DEFAULT_RULE_2(Real, Equals, Int);
ADD_DEFAULT_RULE_2(Real, NotEquals, Int);
ADD_DEFAULT_RULE_2(Real, Less, Int);
ADD_DEFAULT_RULE_2(Real, Greater, Int);
ADD_DEFAULT_RULE_2(Real, LessOrEq, Int);
ADD_DEFAULT_RULE_2(Real, GreaterOrEq, Int);

ADD_DEFAULT_RULE_2(Int, Equals, Real);
ADD_DEFAULT_RULE_2(Int, NotEquals, Real);
ADD_DEFAULT_RULE_2(Int, Less, Real);
ADD_DEFAULT_RULE_2(Int, Greater, Real);
ADD_DEFAULT_RULE_2(Int, LessOrEq, Real);
ADD_DEFAULT_RULE_2(Int, GreaterOrEq, Real);

ADD_DEFAULT_RULE_2(Str, Equals, Str);
ADD_DEFAULT_RULE_2(Str, NotEquals, Str);
ADD_DEFAULT_RULE_2(Str, Less, Str);
ADD_DEFAULT_RULE_2(Str, Greater, Str);
ADD_DEFAULT_RULE_2(Str, LessOrEq, Str);
ADD_DEFAULT_RULE_2(Str, GreaterOrEq, Str);

// boolean operations
ADD_DEFAULT_RULE_2(Bool, And, Bool);
ADD_DEFAULT_RULE_2(Bool, Or, Bool);
ADD_DEFAULT_RULE_1(Not, Bool);

// assign rules
ADD_DEFAULT_RULE_2(Int&, Assign, Int);
ADD_DEFAULT_RULE_2(Real&, Assign, Real);
ADD_DEFAULT_RULE_2(Int&, Assign, Real);
ADD_DEFAULT_RULE_2(Real&, Assign, Int);
ADD_DEFAULT_RULE_2(Str&, Assign, Str);
ADD_DEFAULT_RULE_2(Bool&, Assign, Bool);

#undef ADD_DEFAULT_RULE_1
#undef ADD_DEFAULT_RULE_2

template <typename Op, WeakValueT... Types>
struct IsPerformable
    : std::conditional_t<std::is_base_of_v<details::operations::NotAllowed,
                                           Rule<Op, Types...>>,
                         std::false_type, std::true_type> {};

template <typename Op, WeakValueT... Values>
inline constexpr bool IsPerformableV = IsPerformable<Op, Values...>::value;

template <typename Op, WeakValueT... Types>
constexpr bool IsPerformableRule(Rule<Op, Types...>) {
  return IsPerformableV<Op, Types...>;
}

namespace details {

// TODO: looks wierd, try to make better solution
template <typename Op, size_t I, size_t... Left, size_t... Right,
          WeakValueT... Types>
[[nodiscard]] consteval auto GetDeducedPerformRuleImpl(
    std::index_sequence<Left...> left, std::index_sequence<Right...> right,
    utils::TypeTag<std::tuple<Types...>> tag) noexcept {
  using TupleType = std::tuple<Types...>;
  using CurrentType = std::tuple_element_t<I, TupleType>;
  using CurrentRule = Rule<Op, Types...>;

  if constexpr ((sizeof...(Right) == 0) && ValueT<CurrentType>) {
    return CurrentRule{};
  } else if constexpr (ValueT<CurrentType>) {
    return GetDeducedPerformRuleImpl<Op, I + 1>(
        std::index_sequence<Left..., I>{},
        []<size_t Head, size_t... Tail>(std::index_sequence<Head, Tail...>)
            ->std::index_sequence<Tail...> { return {}; }(right),
        tag);
  } else if constexpr (IsPerformableV<
                           Op, std::tuple_element_t<Left, TupleType>...,
                           std::decay_t<CurrentType>,
                           std::tuple_element_t<Right, TupleType>...>) {
    return Rule<Op, std::tuple_element_t<Left, TupleType>...,
                std::decay_t<CurrentType>,
                std::tuple_element_t<Right, TupleType>...>{};
  } else if constexpr (constexpr auto deduced = GetDeducedPerformRuleImpl<Op,
                                                                          I>(
                           left, right,
                           utils::TypeTag<std::tuple<
                               std::tuple_element_t<Left, TupleType>...,
                               std::decay_t<CurrentType>,
                               std::tuple_element_t<Right, TupleType>...>>{});
                       IsPerformableRule(deduced)) {
    return deduced;
  } else if constexpr (sizeof...(Right) != 0) {
    return GetDeducedPerformRuleImpl<Op, I + 1>(
        std::index_sequence<Left..., I>{},
        []<size_t Head, size_t... Tail>(std::index_sequence<Head, Tail...>)
            ->std::index_sequence<Tail...> { return {}; }(right),
        tag);
  } else {
    return CurrentRule{};
  }
}

template <typename Op, WeakValueT... Types>
[[nodiscard]] consteval auto GetPerformRule() noexcept {
  if constexpr (IsPerformableV<Op, Types...> || (sizeof...(Types) == 0)) {
    return Rule<Op, Types...>{};
  } else if constexpr (constexpr auto deduced =
                           GetDeducedPerformRuleImpl<Op, 0>(
                               std::index_sequence<>{},
                               []<size_t Head, size_t... Tail>(
                                   std::index_sequence<Head, Tail...>) {
                                 return std::index_sequence<Tail...>{};
                               }(std::index_sequence_for<Types...>{}),
                               utils::TypeTag<std::tuple<Types...>>{});
                       IsPerformableRule(deduced)) {
    return deduced;
  } else {
    return Rule<Op, Types...>{};
  }
}

}  // namespace details

template <typename T>
struct PerformTraits;

template <WeakValueT T>
struct PerformTraits<T> {
  using PerformType = typename TypeTraits<T>::Type;
};

template <WeakReferenceT T>
struct PerformTraits<T> {
  using PerformType =
      std::add_lvalue_reference_t<typename TypeTraits<T>::ValueType>;
};

template <typename T>
auto Unwrap();

template <ValueT T>
inline constexpr const T& Unwrap(const T& value) noexcept {
  return value;
}

template <ReferenceT T>
inline constexpr auto Unwrap(T value) noexcept
    -> std::add_lvalue_reference_t<typename TypeTraits<T>::ValueType> {
  return value.get();
}

template <typename Op>
struct Perform {
  template <WeakValueOrReferenceT... Values>
  constexpr OperationValue operator()(Values&&... values) const {
    constexpr auto rule = details::GetPerformRule<
        Op, typename PerformTraits<Values>::PerformType...>();
    if constexpr (IsPerformableRule(rule)) {
      return rule(Unwrap(values)...);
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }
};

template <typename Visitor, SameAsValueOrReference... Values>
auto VisitValues(Visitor&& visitor, Values&&... values) {
  return std::visit(std::forward<Visitor>(visitor),
                    std::forward<Values>(values)...);
}

template <typename Visitor, SameAsOperationValue... OperationValues>
auto VisitOperationValues(Visitor&& visitor,
                          OperationValues&&... operation_values) {
  return std::visit(
      [&visitor]<typename... Args>(Args && ... args) mutable {
        return std::visit(std::forward<Visitor>(visitor),
                          std::forward<Args>(args)...);
      },
      std::forward<OperationValues>(operation_values)...);
}

template <typename Op, SameAsOperationValue... Values>
constexpr OperationValue PerformOperation(Values... values) {
  return VisitOperationValues(Perform<Op>{}, std::move(values)...);
}

}  // namespace interpreter::instructions
