#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

#include "interpreter/utils/type_map.hpp"

// TODO: move all of weird stuff in another header
namespace interpreter::syntax {

struct TypeError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct TypeMissmatchError : public TypeError {
  using TypeError::TypeError;
};

struct UnknownTypeError : public TypeError {
  using TypeError::TypeError;
};

template <typename T>
struct IntegralTypeTraits {
  using ValueType = T;
  using Reference = T&;
  using ConstReference = T;
};

template <typename T>
struct NonIntegralTypeTraits {
  using ValueType = T;
  using Reference = T&;
  using ConstReference = const T&;
};

template <typename T>
struct TypeTraits : NonIntegralTypeTraits<T> {};

enum class VariableType;

namespace details {

template <VariableType enum_value>
struct VarTypeEnumTag : utils::EnumValueTag<VariableType, enum_value> {};

template <typename T>
struct VarTypeTag : utils::TypeTag<T> {};

template <VariableType enum_value, typename T>
using MakeMapping =
    std::pair<utils::AddMapping<utils::DeclareType<VarTypeEnumTag<enum_value>>,
                                VarTypeTag<T>>,
              utils::AddMapping<utils::DeclareType<VarTypeTag<T>>,
                                VarTypeEnumTag<enum_value>>>;
}  // namespace details

template <VariableType enum_value>
struct TypeByEnum {
  using type =
      typename utils::MapType<details::VarTypeEnumTag<enum_value>>::type::type;
};

template <typename T>
struct EnumByType {
  static constexpr VariableType value =
      utils::MapType<details::VarTypeTag<T>>::type::value;
};

struct TypeChecker {
  template <typename T>
  inline void operator()(const T&) const {
    if (type != EnumByType<T>::value) {
      throw TypeMissmatchError{"Incorrect type"};
    }
  }
  VariableType type;
};

enum class VariableType {
  INT,
  REAL,
  BOOL,
  STR,
  _END,
};

namespace types {

using IntT = std::int32_t;
using RealT = double;
using BoolT = bool;
using StrT = std::string;

}  // namespace types

namespace mapping {

struct IntMapping : details::MakeMapping<VariableType::INT, types::IntT> {};
struct RealMapping : details::MakeMapping<VariableType::REAL, types::RealT> {};
struct BoolMapping : details::MakeMapping<VariableType::BOOL, types::BoolT> {};
struct StrMapping : details::MakeMapping<VariableType::STR, types::StrT> {};

}  // namespace mapping

// TODO: automate it
using VariableValue =
    std::variant<types::IntT, types::RealT, types::BoolT, types::StrT>;

template <>
struct TypeTraits<types::IntT> : IntegralTypeTraits<types::IntT> {};

template <>
struct TypeTraits<types::BoolT> : IntegralTypeTraits<types::BoolT> {};

template <>
struct TypeTraits<types::RealT> : IntegralTypeTraits<types::RealT> {};

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

// TODO: It should be in visitor.hpp. Moving is blocked,
// because instructions.hpp is using this structure.
struct Constant {
  VariableType type;
  VariableValue value;
};

}  // namespace interpreter::syntax
