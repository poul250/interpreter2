#pragma once

#include <array>
#include <functional>
#include <stdexcept>

#include "types.hpp"

namespace interpreter::ast {

struct TypeError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct TypeMissmatchError : public TypeError {
  using TypeError::TypeError;
};

struct UnknownTypeError : public TypeError {
  using TypeError::TypeError;
};

namespace details {

template <typename VisitorT, typename ReturnT, VariableType enum_value>
ReturnT VisitorCaller(VisitorT&& handler) {
  return std::invoke(std::forward<VisitorT>(handler),
                     utils::TypeTag<typename TypeByEnum<enum_value>::type>{});
}

template <typename VisitorT, typename ReturnT, size_t... I>
constexpr auto MakeCallers(std::index_sequence<I...>) {
  return std::array<std::add_pointer_t<ReturnT(VisitorT &&)>, sizeof...(I)>{
      VisitorCaller<VisitorT, ReturnT, static_cast<VariableType>(I)>...};
}

// looks weird, but is's ok
template <typename VisitorT>
using VisitorReturnType = std::result_of_t<VisitorT(
    utils::TypeTag<TypeByEnum<static_cast<VariableType>(0)>>)>;

template <typename VisitorT, typename ReturnT = VisitorReturnType<VisitorT>>
constexpr auto MakeVisitorTypeCallers() {
  return MakeCallers<VisitorT, ReturnT>(
      std::make_index_sequence<static_cast<size_t>(VariableType::_END)>{});
}

}  // namespace details

struct TypeChecker {
  template <typename T>
  inline void operator()(const T&) const {
    if (type != EnumByType<T>::value) {
      throw TypeMissmatchError{"Incorrect type"};
    }
  }
  VariableType type;
};

template <typename VisitorT>
auto VisitType(VisitorT&& visitor, VariableType type) {
  // oh my god this is the best i've ever done
  static constexpr auto callers = details::MakeVisitorTypeCallers<VisitorT>();

  return callers[static_cast<size_t>(type)](std::forward<VisitorT>(visitor));
}

}  // namespace interpreter::ast
