#pragma once

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

template <typename HandlerT, typename ReturnT, VariableType enum_value>
ReturnT HandlerCaller(HandlerT&& handler) {
  return std::invoke(std::forward<HandlerT>(handler),
                     utils::TypeTag<typename TypeByEnum<enum_value>::type>{});
}

template <typename HandlerT, typename ReturnT, size_t... I>
constexpr auto MakeCallers(std::index_sequence<I...>) {
  return std::array<std::add_pointer_t<ReturnT(HandlerT &&)>, sizeof...(I)>{
      HandlerCaller<HandlerT, ReturnT, static_cast<VariableType>(I)>...};
}

// looks weird, but is's ok
template <typename Handler>
using HandlerReturnType = std::result_of_t<Handler(
    utils::TypeTag<TypeByEnum<static_cast<VariableType>(0)>>)>;

template <typename HandlerT, typename ReturnT = HandlerReturnType<HandlerT>>
constexpr auto MakeHandlerTypeCallers() {
  return MakeCallers<HandlerT, ReturnT>(
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

class VisitType {
 public:
  constexpr explicit VisitType(VariableType type) noexcept : type_(type) {}

  template <typename HandlerT>
  auto operator()(HandlerT&& handler) const {
    // oh my god this is the best i've ever done
    static constexpr auto callbacks =
        details::MakeHandlerTypeCallers<HandlerT>();

    return callbacks[static_cast<size_t>(type_)](
        std::forward<HandlerT>(handler));
  }

 private:
  VariableType type_;
};

}  // namespace interpreter::ast
