#pragma once

#include <stdexcept>
#include <string>
#include <type_traits>
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

// Types concepts
template <typename T>
concept ValueT = utils::IsVariantMember<T, Value>::value;
template <typename T>
concept WeakValueT = utils::IsVariantMember<std::decay_t<T>, Value>::value;
template <typename T>
concept ReferenceT = utils::IsVariantMember<T, Reference>::value;
template <typename T>
concept WeakReferenceT =
    utils::IsVariantMember<std::decay_t<T>, Reference>::value;
template <typename T>
concept ValueOrReferenceT = ValueT<T> || ReferenceT<T>;
template <typename T>
concept WeakValueOrReferenceT = WeakValueT<T> || WeakReferenceT<T>;

// Same as concepts
template <typename T>
concept SameAsValue = std::same_as<std::decay_t<T>, Value>;
template <typename T>
concept SameAsReference = std::same_as<std::decay_t<T>, Value>;
template <typename T>
concept SameAsValueOrReference = SameAsValue<T> || SameAsReference<T>;

// Type traits rules
template <typename T>
struct TypeTraits;

template <WeakValueT T>
struct TypeTraits<T> {
  using Type = std::decay_t<T>;
  using ValueType = Type;
  using ReferenceType = std::reference_wrapper<Type>;
};

template <WeakReferenceT T>
struct TypeTraits<T> {
  using Type = std::decay_t<T>;
  using ValueType = typename Type::type;
  using ReferenceType = Type;
};

// Helpers
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

}  // namespace interpreter::instructions
