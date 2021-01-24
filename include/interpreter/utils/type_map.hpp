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

template <typename>
struct DeclareType {
  friend constexpr auto _Get(DeclareType);
};

template <typename A, typename T>
struct AddMapping {
  friend constexpr auto _Get(A) { return T{}; }
};

template <typename T>
struct MapType {
  using type = decltype(_Get(DeclareType<T>{}));
};

}  // namespace interpreter::utils
