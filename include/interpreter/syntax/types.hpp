#pragma once

#include <cstdint>
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

struct IntMapping : details::MakeMapping<VariableType::INT, IntT> {};
struct RealMapping : details::MakeMapping<VariableType::REAL, RealT> {};
struct BoolMapping : details::MakeMapping<VariableType::BOOL, BoolT> {};
struct StrMapping : details::MakeMapping<VariableType::STR, StrT> {};

}  // namespace mapping
}  // namespace types

using VariableValue =
    std::variant<types::IntT, types::RealT, types::BoolT, types::StrT>;

template <VariableType enum_type>
struct TypeByEnum {
  using type = typename utils::MapType<
      types::details::VarTypeEnumTag<enum_type>>::type::type;
};

template <typename T>
struct EnumByType {
  static constexpr VariableType value =
      utils::MapType<types::details::VarTypeTag<T>>::type::value;
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
  constexpr explicit VisitType(syntax::VariableType type) noexcept
      : type_(type) {}

  template <typename HandlerT>
  auto operator()(HandlerT&& handler) const {
    // TODO: pls do something smarter
    switch (type_) {
      case VariableType::INT:
        return std::forward<HandlerT>(handler)(
            utils::TypeTag<TypeByEnum<VariableType::INT>::type>{});
      case VariableType::REAL:
        return std::forward<HandlerT>(handler)(
            utils::TypeTag<TypeByEnum<VariableType::REAL>::type>{});
      case VariableType::BOOL:
        return std::forward<HandlerT>(handler)(
            utils::TypeTag<TypeByEnum<VariableType::BOOL>::type>{});
      case VariableType::STR:
        return std::forward<HandlerT>(handler)(
            utils::TypeTag<TypeByEnum<VariableType::STR>::type>{});
    }
    throw UnknownTypeError{"unknown type"};
  }

 private:
  VariableType type_;
};

// TODO: Should it be in visitor.hpp?
struct Constant {
  VariableType type;
  VariableValue value;
};

}  // namespace interpreter::syntax
