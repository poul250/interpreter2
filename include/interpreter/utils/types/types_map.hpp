#pragma once

namespace interpreter::utils {

namespace details {

// Poluhin's magic
template <typename>
struct DeclareType {
  friend constexpr auto _Get(DeclareType);
};

template <typename A, typename T>
struct DefineType {
  friend constexpr auto _Get(A) { return T{}; }
};

}  // namespace details

template<typename From, typename To>
using AddMapping = details::DefineType<details::DeclareType<From>, To>;

template <typename T>
struct MapType {
  using type = decltype(_Get(details::DeclareType<T>{}));
};

template <typename A, typename B>
struct AddBijectiveMapping : AddMapping<A, B>, AddMapping<B, A> {};

}  // namespace interpreter::utils
