#pragma once

namespace interpreter::utils {

template <typename T>
struct TypeTag {
  using type = T;
};

template <typename EnumType, EnumType EnumValue>
struct EnumValueTag {
  static constexpr EnumType value = EnumValue;
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

}  // namespace interpreter::utils
