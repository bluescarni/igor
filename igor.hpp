// Copyright 2018 Francesco Biscani
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef IGOR_IGOR_HPP
#define IGOR_IGOR_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace igor {

inline namespace detail {

template <typename T>
using uncvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename Tag, typename T> struct tagged_container {
  using tag_type = Tag;
  T value;
};

} // namespace detail

template <typename Tag> struct named_argument {
  using tag_type = Tag;
  template <
      typename T,
      std::enable_if_t<!std::is_same_v<named_argument, uncvref_t<T>>, int> = 0>
  auto operator=(T &&x) const {
    return tagged_container<Tag, T &&>{std::forward<T>(x)};
  }
};

inline namespace detail {

template <typename T> struct is_tagged_container : std::false_type {};

template <typename Tag, typename T>
struct is_tagged_container<tagged_container<Tag, T>> : std::true_type {};

template <typename T> struct is_named_argument : std::false_type {};

template <typename Tag>
struct is_named_argument<named_argument<Tag>> : std::true_type {};

struct not_provided_t {};

inline constexpr auto not_provided = not_provided_t{};

} // namespace detail

template <typename... ParseArgs> class parser {
  static auto build_container_tuple_impl() { return std::make_tuple(); }
  template <typename Arg0, typename... Args>
  static auto build_container_tuple_impl(const Arg0 &arg0,
                                         const Args &... args) {
    if constexpr (is_tagged_container<Arg0>::value) {
      return std::tuple_cat(std::forward_as_tuple(arg0),
                            build_container_tuple_impl(args...));
    } else {
      return build_container_tuple_impl(args...);
    }
  }
  template <typename... Args>
  static auto build_container_tuple(const Args &... args) {
    return build_container_tuple_impl(args...);
  }
  using tuple_t =
      decltype(build_container_tuple(std::declval<const ParseArgs &>()...));

public:
  explicit parser(const ParseArgs &... parse_args)
      : m_nargs(build_container_tuple(parse_args...)) {}

private:
  template <std::size_t I, typename T>
  decltype(auto) fetch_one_impl(const T &narg) const {
    if constexpr (I == std::tuple_size_v<tuple_t>) {
      return static_cast<const not_provided_t &>(not_provided);
    } else if constexpr (std::is_same_v<typename uncvref_t<std::tuple_element_t<
                                            I, tuple_t>>::tag_type,
                                        typename T::tag_type>) {
      if constexpr (std::is_rvalue_reference_v<decltype(
                        std::get<I>(m_nargs).value)>) {
        return std::move(std::get<I>(m_nargs).value);
      } else {
        return std::get<I>(m_nargs).value;
      }
    } else {
      return fetch_one_impl<I + 1u>(narg);
    }
  }
  template <typename T> decltype(auto) fetch_one(const T &narg) const {
    return fetch_one_impl<0>(narg);
  }

public:
  template <typename... T>
  decltype(auto) operator()(const named_argument<T> &... nargs) const {
    if constexpr (sizeof...(T) == 0u) {
      return;
    } else if constexpr (sizeof...(T) == 1u) {
      return fetch_one(nargs...);
    } else {
      return std::forward_as_tuple(fetch_one(nargs)...);
    }
  }

private:
  tuple_t m_nargs;
};

template <typename T>
inline constexpr bool is_provided = !std::is_same_v<T, const not_provided_t &>;

} // namespace igor

#endif
