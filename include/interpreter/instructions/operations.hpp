#pragma once

#include <functional>
#include <string>
#include <variant>

#include "interpreter/utils/types/types_variant.hpp"

namespace interpreter::instructions {

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
// struct Mul : Op{};
// struct Div : Op{};
// struct Mod : Op{};
struct Less : Op {};
struct Greater : Op {};
// struct Equal : Op{};
// struct NotEqual : Op{};
}  // namespace op_type

namespace details {

namespace operations {

struct NotAllowed {};

template <typename L, typename R>
struct Assign {
  constexpr auto operator()(L& lhs, const R& rhs) const { return lhs = rhs; }
};

template <typename L, typename R>
struct Plus {
  constexpr auto operator()(const L& lhs, const R& rhs) const {
    return lhs + rhs;
  }
};

template <typename L, typename R>
struct Minus {
  constexpr auto operator()(const L& lhs, const R& rhs) const {
    return lhs - rhs;
  }
};

template <typename L, typename R>
struct Less {
  constexpr auto operator()(const L& lhs, const R& rhs) const {
    return lhs < rhs;
  }
};

template <typename L, typename R>
struct Greater {
  constexpr auto operator()(const L& lhs, const R& rhs) const {
    return lhs < rhs;
  }
};

}  // namespace operations

}  // namespace details

template <typename L, typename Op, typename R>
struct Rule : details::operations::NotAllowed {};

// Sorry about that, no reflection for now
#define ADD_DEFAULT_RULE(l, op, r)             \
  template <>                                  \
  struct Rule<types::l, op_type::op, types::r> \
      : details::operations::op<types::l, types::r> {}

#define ADD_RULE(l, op, r, handler)                                            \
  template <>                                                                  \
  struct Rule<types::l, op_type::op, types::r> : handler<types::l, types::r> { \
  }

// arithmetical rules
ADD_DEFAULT_RULE(Int, Plus, Int);
ADD_DEFAULT_RULE(Int, Minus, Int);

ADD_DEFAULT_RULE(Real, Plus, Real);
ADD_DEFAULT_RULE(Real, Minus, Real);

ADD_DEFAULT_RULE(Real, Plus, Int);
ADD_DEFAULT_RULE(Real, Minus, Int);

ADD_DEFAULT_RULE(Int, Plus, Real);
ADD_DEFAULT_RULE(Int, Minus, Real);

ADD_DEFAULT_RULE(Str, Plus, Str);

// compare rules
ADD_DEFAULT_RULE(Int, Less, Int);
ADD_DEFAULT_RULE(Int, Greater, Int);

ADD_DEFAULT_RULE(Real, Less, Real);
ADD_DEFAULT_RULE(Real, Greater, Real);

ADD_DEFAULT_RULE(Real, Less, Int);
ADD_DEFAULT_RULE(Real, Greater, Int);

ADD_DEFAULT_RULE(Int, Less, Real);
ADD_DEFAULT_RULE(Int, Greater, Real);

ADD_DEFAULT_RULE(Str, Less, Str);
ADD_DEFAULT_RULE(Str, Greater, Str);

// assign rules
ADD_DEFAULT_RULE(Int&, Assign, Int);
ADD_DEFAULT_RULE(Real&, Assign, Real);
ADD_DEFAULT_RULE(Int&, Assign, Real);
ADD_DEFAULT_RULE(Real&, Assign, Int);
ADD_DEFAULT_RULE(Str&, Assign, Str);
ADD_DEFAULT_RULE(Bool&, Assign, Bool);

#undef ADD_RULE
#undef ADD_DEFAULT_RULE

template <typename L, typename Op, typename R>
struct IsPerformable
    : std::conditional_t<
          std::is_base_of_v<details::operations::NotAllowed, Rule<L, Op, R>>,
          std::false_type, std::true_type> {};

// for type deduction magic
template <typename Op>
struct Perform {
  template <typename L, typename R>
  constexpr OperationValue operator()(const L& lhs, const R& rhs) const {
    if constexpr (IsPerformable<std::decay_t<L>, Op, std::decay_t<R>>::value) {
      return Rule<std::decay_t<L>, Op, std::decay_t<R>>{}(lhs, rhs);
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }

  template <typename L, typename R>
  constexpr OperationValue operator()(std::reference_wrapper<L> lhs,
                                      const R& rhs) const {
    if constexpr (IsPerformable<std::decay_t<L>&, Op, std::decay_t<R>>::value) {
      return Rule<std::decay_t<L>&, Op, std::decay_t<R>>{}(lhs.get(), rhs);
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }

  template <typename L, typename R>
  constexpr OperationValue operator()(const L& lhs,
                                      std::reference_wrapper<R> rhs) const {
    if constexpr (IsPerformable<std::decay_t<L>, Op, std::decay_t<R>>::value) {
      return Rule<std::decay_t<L>, Op, std::decay_t<R>>{}(lhs, rhs.get());
    } else {
      // pls smt smarter
      throw "bruh";
    }
  }

  template <typename L, typename R>
  constexpr OperationValue operator()(std::reference_wrapper<L> lhs,
                                      std::reference_wrapper<R> rhs) const {
    if constexpr (IsPerformable<std::decay_t<L>, Op, std::decay_t<R>>::value) {
      return Rule<std::decay_t<L>, Op, std::decay_t<R>>{}(lhs.get(), rhs.get());
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
constexpr OperationValue PerformOperation(OperationValue lhs,
                                          OperationValue rhs) {
  return VisitOperationValues(Perform<Op>{}, std::move(lhs), std::move(rhs));
}

}  // namespace interpreter::instructions
