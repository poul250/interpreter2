#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "interpreter/utils/types/types.hpp"
#include "interpreter/utils/types/types_map.hpp"

// TODO: move all of weird stuff in another header
namespace interpreter::ast {

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

// TODO: automate it
using VariableValue =
    std::variant<types::IntT, types::RealT, types::BoolT, types::StrT>;

namespace details {

template <VariableType enum_value>
struct VarTypeEnumTag : utils::EnumValueTag<VariableType, enum_value> {};

template <typename T>
struct VarTypeTag : utils::TypeTag<T> {};

template <VariableType enum_value, typename T>
using MakeMapping =
    utils::AddBijectiveMapping<VarTypeEnumTag<enum_value>, VarTypeTag<T>>;

}  // namespace details

namespace mapping {

struct IntMapping : details::MakeMapping<VariableType::INT, types::IntT> {};
struct RealMapping : details::MakeMapping<VariableType::REAL, types::RealT> {};
struct BoolMapping : details::MakeMapping<VariableType::BOOL, types::BoolT> {};
struct StrMapping : details::MakeMapping<VariableType::STR, types::StrT> {};

}  // namespace mapping

template <typename T>
struct TypeTraits : utils::NonIntegralTypeTraits<T> {};

template <>
struct TypeTraits<types::IntT> : utils::IntegralTypeTraits<types::IntT> {};

template <>
struct TypeTraits<types::BoolT> : utils::IntegralTypeTraits<types::BoolT> {};

template <>
struct TypeTraits<types::RealT> : utils::IntegralTypeTraits<types::RealT> {};

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

}  // namespace interpreter::ast
