#pragma once

#include <functional>
#include <type_traits>
#include <variant>

#include "types.hpp"

namespace interpreter::utils {

namespace details {

template <typename... Types>
auto ReferenceVariantHelper(TypeTag<std::variant<Types...>>) {
  return TypeTag<
      std::variant<std::reference_wrapper<std::decay_t<Types>>...>>{};
}

template <typename Variant>
using ReferenceVariantImpl =
    typename decltype(ReferenceVariantHelper(TypeTag<Variant>{}))::type;

}  // namespace details

template <typename Variant>
using ReferenceVariant = details::ReferenceVariantImpl<Variant>;

template <typename T, typename VariantT>
struct IsVariantMember;

template <typename T, typename... Types>
struct IsVariantMember<T, std::variant<Types...>>
    : std::disjunction<std::is_same<T, Types>...> {};

}  // namespace interpreter::utils
