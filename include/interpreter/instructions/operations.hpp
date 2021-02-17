#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <variant>

#include "interpreter/utils/types/types_variant.hpp"

namespace interpreter::instructions {

struct ValueError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

namespace types {

using Bool = bool;
using Int = std::int32_t;
using Real = double;
using Str = std::string;

}  // namespace types

using Value = std::variant<types::Bool, types::Int, types::Real, types::Str>;
using Reference = utils::ReferenceVariant<Value>;
using OperationValue = std::variant<Value, Reference>;

namespace op_type {
struct Op {};
struct Assign : Op {};
struct Plus : Op {};
struct Minus : Op {};
struct Or : Op {};
struct And : Op {};
struct Mul : Op {};
// struct Div : Op{};
struct Mod : Op {};
struct Not : Op {};
struct Equals : Op {};
struct Less : Op {};
struct Greater : Op {};
struct LessOrEq : Op {};
struct GreaterOrEq : Op {};
// struct NotEquals : Op{};
}  // namespace op_type

namespace details {

namespace operations {

struct NotAllowed {};

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

ADD_DEFAULT_BIN_OPERATION(Plus, +);
ADD_DEFAULT_BIN_OPERATION(Minus, -);
ADD_DEFAULT_BIN_OPERATION(Mul, *);
ADD_DEFAULT_BIN_OPERATION(Mod, %);
ADD_DEFAULT_BIN_OPERATION(Equals, ==);
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

// TODO: pls variadic template
template <typename Op, typename T>
struct UnRule : details::operations::NotAllowed {};

template <typename L, typename Op, typename R>
struct Rule : details::operations::NotAllowed {};

// Sorry about that, no reflection for now
#define ADD_DEFAULT_RULE(l, op, r)             \
  template <>                                  \
  struct Rule<types::l, op_type::op, types::r> \
      : details::operations::op<types::l, types::r> {}

// Sorry about that, no reflection for now
#define ADD_DEFAULT_UN_RULE(op, t) \
  template <>                      \
  struct UnRule<op_type::op, types::t> : details::operations::op<types::t> {}

#define ADD_RULE(l, op, r, handler)                                            \
  template <>                                                                  \
  struct Rule<types::l, op_type::op, types::r> : handler<types::l, types::r> { \
  }

// arithmetical rules
ADD_DEFAULT_RULE(Int, Plus, Int);
ADD_DEFAULT_RULE(Int, Minus, Int);
ADD_DEFAULT_RULE(Int, Mul, Int);
ADD_DEFAULT_RULE(Int, Mod, Int);

ADD_DEFAULT_RULE(Real, Plus, Real);
ADD_DEFAULT_RULE(Real, Minus, Real);
ADD_DEFAULT_RULE(Real, Mul, Real);

ADD_DEFAULT_RULE(Real, Plus, Int);
ADD_DEFAULT_RULE(Real, Minus, Int);
ADD_DEFAULT_RULE(Real, Mul, Int);

ADD_DEFAULT_RULE(Int, Plus, Real);
ADD_DEFAULT_RULE(Int, Minus, Real);
ADD_DEFAULT_RULE(Int, Mul, Real);

ADD_DEFAULT_RULE(Str, Plus, Str);

// unary arithmetical rules
// TODO: add them ;)

// compare rules
ADD_DEFAULT_RULE(Int, Equals, Int);
ADD_DEFAULT_RULE(Int, Less, Int);
ADD_DEFAULT_RULE(Int, Greater, Int);
ADD_DEFAULT_RULE(Int, LessOrEq, Int);
ADD_DEFAULT_RULE(Int, GreaterOrEq, Int);

ADD_DEFAULT_RULE(Real, Equals, Real);
ADD_DEFAULT_RULE(Real, Less, Real);
ADD_DEFAULT_RULE(Real, Greater, Real);
ADD_DEFAULT_RULE(Real, LessOrEq, Real);
ADD_DEFAULT_RULE(Real, GreaterOrEq, Real);

ADD_DEFAULT_RULE(Real, Equals, Int);
ADD_DEFAULT_RULE(Real, Less, Int);
ADD_DEFAULT_RULE(Real, Greater, Int);
ADD_DEFAULT_RULE(Real, LessOrEq, Int);
ADD_DEFAULT_RULE(Real, GreaterOrEq, Int);

ADD_DEFAULT_RULE(Int, Equals, Real);
ADD_DEFAULT_RULE(Int, Less, Real);
ADD_DEFAULT_RULE(Int, Greater, Real);
ADD_DEFAULT_RULE(Int, LessOrEq, Real);
ADD_DEFAULT_RULE(Int, GreaterOrEq, Real);

ADD_DEFAULT_RULE(Str, Equals, Str);
ADD_DEFAULT_RULE(Str, Less, Str);
ADD_DEFAULT_RULE(Str, Greater, Str);
ADD_DEFAULT_RULE(Str, LessOrEq, Str);
ADD_DEFAULT_RULE(Str, GreaterOrEq, Str);

// boolean operations
ADD_DEFAULT_RULE(Bool, And, Bool);
ADD_DEFAULT_RULE(Bool, Or, Bool);
ADD_DEFAULT_UN_RULE(Not, Bool);

// assign rules
ADD_DEFAULT_RULE(Int&, Assign, Int);
ADD_DEFAULT_RULE(Real&, Assign, Real);
ADD_DEFAULT_RULE(Int&, Assign, Real);
ADD_DEFAULT_RULE(Real&, Assign, Int);
ADD_DEFAULT_RULE(Str&, Assign, Str);
ADD_DEFAULT_RULE(Bool&, Assign, Bool);

#undef ADD_RULE
#undef ADD_DEFAULT_RULE

// TODO: should I add the bool rule?
struct ToBoolVisitor {
  template <typename T>
  constexpr bool operator()(const T& value) const {
    if constexpr (std::is_same_v<std::decay_t<T>, types::Bool>) {
      return value;
    } else {
      throw ValueError{"Failed to cast to bool"};
    }
  }
  template <typename T>
  constexpr bool operator()(std::reference_wrapper<T> value) const {
    if constexpr (std::is_same_v<std::decay_t<T>, types::Bool>) {
      return value;
    } else {
      throw ValueError{"Failed to cast to bool"};
    }
  }
};

// TODO: pls do variadic template
template <typename L, typename Op, typename R>
struct IsPerformable
    : std::conditional_t<
          std::is_base_of_v<details::operations::NotAllowed, Rule<L, Op, R>>,
          std::false_type, std::true_type> {};

template <typename Op, typename T>
struct IsUnPerformable
    : std::conditional_t<
          std::is_base_of_v<details::operations::NotAllowed, UnRule<Op, T>>,
          std::false_type, std::true_type> {};

template <typename L, typename Op, typename R>
inline constexpr bool IsPerformableV = IsPerformable<L, Op, R>::value;

template <typename Op, typename T>
inline constexpr bool IsUnPerformableV = IsUnPerformable<Op, T>::value;

// think about how optimize this
// TODO: pls variadic templates
template <typename Op>
struct Perform {
  template <typename T>
  constexpr OperationValue operator()(const T& value) const {
    if constexpr (IsUnPerformableV<Op, std::decay_t<T>>) {
      return UnRule<Op, std::decay_t<T>>{}(value);
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }

  template <typename T>
  constexpr OperationValue operator()(std::reference_wrapper<T> ref) const {
    if constexpr (IsUnPerformableV<Op, std::decay_t<T>>) {
      return UnRule<Op, std::decay_t<T>>{}(ref.get());
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }

  template <typename L, typename R>
  constexpr OperationValue operator()(const L& lhs, const R& rhs) const {
    if constexpr (IsPerformableV<std::decay_t<L>, Op, std::decay_t<R>>) {
      return Rule<std::decay_t<L>, Op, std::decay_t<R>>{}(lhs, rhs);
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }

  template <typename L, typename R>
  constexpr OperationValue operator()(std::reference_wrapper<L> wrapped_lhs,
                                      const R& rhs) const {
    using Lhs = std::decay_t<L>;
    using Rhs = std::decay_t<R>;
    L& lhs = wrapped_lhs.get();

    if constexpr (IsPerformableV<Lhs&, Op, Rhs>) {
      return Rule<Lhs&, Op, Rhs>{}(lhs, rhs);
    } else if constexpr (IsPerformableV<Lhs, Op, Rhs>) {
      return Rule<Lhs, Op, Rhs>{}(lhs, rhs);
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }

  template <typename L, typename R>
  constexpr OperationValue operator()(
      const L& lhs, std::reference_wrapper<R> wrapped_rhs) const {
    using Lhs = std::decay_t<L>;
    using Rhs = std::decay_t<R>;
    R& rhs = wrapped_rhs.get();

    if constexpr (IsPerformableV<Lhs, Op, Rhs&>) {
      return Rule<Lhs, Op, Rhs&>{}(lhs, rhs);
    } else if constexpr (IsPerformableV<Lhs, Op, Rhs>) {
      return Rule<Lhs, Op, Rhs>{}(lhs, rhs);
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }

  template <typename L, typename R>
  constexpr OperationValue operator()(
      std::reference_wrapper<L> wrapped_lhs,
      std::reference_wrapper<R> wrapped_rhs) const {
    using Lhs = std::decay_t<L>;
    using Rhs = std::decay_t<R>;
    L& lhs = wrapped_lhs.get();
    R& rhs = wrapped_rhs.get();

    if constexpr (IsPerformableV<Lhs&, Op, Rhs&>) {
      return Rule<Lhs&, Op, Rhs&>{}(lhs, rhs);
    } else if constexpr (IsPerformableV<Lhs&, Op, Rhs>) {
      return Rule<Lhs&, Op, Rhs>{}(lhs, rhs);
    } else if constexpr (IsPerformableV<Lhs, Op, Rhs&>) {
      return Rule<Lhs, Op, Rhs&>{}(lhs, rhs);
    } else if constexpr (IsPerformableV<Lhs, Op, Rhs>) {
      return Rule<Lhs, Op, Rhs>{}(lhs, rhs);
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }
};

template <typename Visitor, typename... Values>
auto VisitValues(Visitor&& visitor, Values&&... values) {
  return std::visit(std::forward<Visitor>(visitor),
                    std::forward<Values>(values)...);
}

template <typename Visitor, typename... OperationValues>
auto VisitOperationValues(Visitor&& visitor,
                          OperationValues&&... operation_values) {
  return std::visit(
      [&visitor]<typename... Args>(Args && ... args) mutable {
        return std::visit(std::forward<Visitor>(visitor),
                          std::forward<Args>(args)...);
      },
      std::forward<OperationValues>(operation_values)...);
}

template <typename Op>
constexpr OperationValue PerformOperation(OperationValue value) {
  return VisitOperationValues(Perform<Op>{}, std::move(value));
}

template <typename Op>
constexpr OperationValue PerformOperation(OperationValue lhs,
                                          OperationValue rhs) {
  return VisitOperationValues(Perform<Op>{}, std::move(lhs), std::move(rhs));
}

}  // namespace interpreter::instructions
