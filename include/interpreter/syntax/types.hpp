#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

#include "interpreter/utils/type_map.hpp"

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

namespace mapping {

struct IntMapping : details::MakeMapping<VariableType::INT, types::IntT> {};
struct RealMapping : details::MakeMapping<VariableType::REAL, types::RealT> {};
struct BoolMapping : details::MakeMapping<VariableType::BOOL, types::BoolT> {};
struct StrMapping : details::MakeMapping<VariableType::STR, types::StrT> {};

}  // namespace mapping

using VariableValue =
    std::variant<types::IntT, types::RealT, types::BoolT, types::StrT>;

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

namespace details {

// TODO: pls do something better
template <typename CallbackT, typename ReturnT>
class CallbackBase {
 public:
  virtual ~CallbackBase() = default;
  virtual ReturnT Call(CallbackT&& handler) = 0;
};

template <VariableType enum_value, typename CallbackT, typename ReturnT>
class TypedCall : public CallbackBase<CallbackT, ReturnT> {
 public:
  inline ReturnT Call(CallbackT&& handler) {
    return std::invoke(std::forward<CallbackT>(handler),
                       utils::TypeTag<typename TypeByEnum<enum_value>::type>{});
  }
};

template <size_t I, typename CallbackT, typename ReturnT>
constexpr std::unique_ptr<CallbackBase<CallbackT, ReturnT>> MakeTypedCall() {
  return std::make_unique<
      TypedCall<static_cast<VariableType>(I), CallbackT, ReturnT>>();
}

template <typename CallbackT, typename ReturnT, size_t... I>
constexpr auto MakeCallbacks(std::integer_sequence<size_t, I...>) {
  return std::array<std::unique_ptr<CallbackBase<CallbackT, ReturnT>>,
                    sizeof...(I)>{MakeTypedCall<I, CallbackT, ReturnT>()...};
}

template <typename CallbackT, typename ReturnT = void>
constexpr auto MakeVariableTypeCallbacks() {
  return MakeCallbacks<CallbackT, ReturnT>(
      std::make_integer_sequence<size_t,
                                 static_cast<size_t>(VariableType::_END)>{});
}

}  // namespace details

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

template <>
struct TypeTraits<types::IntT> : IntegralTypeTraits<types::IntT> {};

template <>
struct TypeTraits<types::BoolT> : IntegralTypeTraits<types::BoolT> {};

template <>
struct TypeTraits<types::RealT> : IntegralTypeTraits<types::RealT> {};

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
    using AnyTypeTag = utils::TypeTag<TypeByEnum<static_cast<VariableType>(0)>>;
    using HandlerCallResult = std::result_of_t<HandlerT(AnyTypeTag)>;

    static auto type_callers =
        details::MakeVariableTypeCallbacks<HandlerT, HandlerCallResult>();

    return type_callers[static_cast<size_t>(type_)]->Call(
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
