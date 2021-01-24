#pragma once

#include <string>
#include <string_view>

namespace interpreter::utils {

namespace details {

template <typename T>
std::string ToString(T value) requires(std::integral<std::decay_t<T>> or
                                       std::floating_point<std::decay_t<T>>) {
  return std::to_string(value);
}

template <typename T>
std::string ToString(
    const T& value) requires std::is_convertible_v<T, std::string> {
  return value;
}

template <class Arg, class... Args>
std::string format(std::string_view fmt, const Arg& arg, const Args&... args) {
  std::string result;

  auto brace_index = fmt.find('{');
  if (brace_index == std::string_view::npos) {
    throw std::runtime_error{"bruh"};
  }

  result += fmt.substr(0, brace_index);
  result += ToString(arg);

  auto rest_fmt = fmt.substr(brace_index + 2);

  if constexpr (sizeof...(args) == 0) {
    return result + std::string(rest_fmt);
  } else {
    return result + format(rest_fmt, args...);
  }
}

}  // namespace details

// Waiting for C++20, but for now we use a simple implementation.
// Uncomment the line below when the compiler devs do their job
// using format = std::format;
template <class... Args>
std::string format(std::string_view fmt, const Args&... args) {
  if constexpr (sizeof...(args) == 0) {
    return std::string(fmt);
  } else {
    return details::format(fmt, args...);
  }
}

}  // namespace interpreter::utils
