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
#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <utility>

namespace igor
{

inline namespace detail
{

template <typename T>
using uncvref_t = ::std::remove_cv_t<::std::remove_reference_t<T>>;

template <typename Tag, typename T>
struct tagged_container {
    using tag_type = Tag;
    T value;
};

} // namespace detail

template <typename Tag>
struct named_argument {
    using tag_type = Tag;
    template <typename T, ::std::enable_if_t<!::std::is_same_v<named_argument, uncvref_t<T>>, int> = 0>
    auto operator=(T &&x) const
    {
        return tagged_container<Tag, T &&>{::std::forward<T>(x)};
    }
    template <typename T>
    auto operator=(const ::std::initializer_list<T> &l) const
    {
        return tagged_container<Tag, const ::std::initializer_list<T> &>{l};
    }
    template <typename T>
    auto operator=(::std::initializer_list<T> &l) const
    {
        return tagged_container<Tag, ::std::initializer_list<T> &>{l};
    }
    template <typename T>
    auto operator=(::std::initializer_list<T> &&l) const
    {
        return tagged_container<Tag, ::std::initializer_list<T> &&>{::std::move(l)};
    }
    template <typename T>
    auto operator=(const ::std::initializer_list<T> &&l) const
    {
        return tagged_container<Tag, const ::std::initializer_list<T> &&>{::std::move(l)};
    }
};

inline namespace detail
{

struct not_provided_t {
};

inline constexpr auto not_provided = not_provided_t{};

template <typename Tag, typename T>
struct is_provided_impl : ::std::false_type {
};

template <typename Tag1, typename Tag2, typename T>
struct is_provided_impl<Tag1, tagged_container<Tag2, T>> : ::std::is_same<Tag1, Tag2> {
};

inline auto build_parser_tuple()
{
    return ::std::make_tuple();
}

template <typename Tag, typename T, typename... Args>
inline auto build_parser_tuple(const tagged_container<Tag, T> &arg0, const Args &... args)
{
    return ::std::tuple_cat(::std::forward_as_tuple(arg0), build_parser_tuple(args...));
}

template <typename Arg0, typename... Args>
inline auto build_parser_tuple(const Arg0 &, const Args &... args)
{
    return build_parser_tuple(args...);
}

} // namespace detail

template <typename... ParseArgs>
class parser
{
    using tuple_t = decltype(build_parser_tuple(::std::declval<const ParseArgs &>()...));

public:
    explicit parser(const ParseArgs &... parse_args) : m_nargs(build_parser_tuple(parse_args...)) {}

private:
    template <::std::size_t I, typename T>
    decltype(auto) fetch_one_impl(const T &narg) const
    {
        if constexpr (I == ::std::tuple_size_v<tuple_t>) {
            return static_cast<const not_provided_t &>(not_provided);
        } else if constexpr (::std::is_same_v<typename uncvref_t<::std::tuple_element_t<I, tuple_t>>::tag_type,
                                              typename T::tag_type>) {
            if constexpr (::std::is_rvalue_reference_v<decltype(::std::get<I>(m_nargs).value)>) {
                return ::std::move(::std::get<I>(m_nargs).value);
            } else {
                return ::std::get<I>(m_nargs).value;
            }
        } else {
            return fetch_one_impl<I + 1u>(narg);
        }
    }
    template <typename T>
    decltype(auto) fetch_one(const T &narg) const
    {
        return fetch_one_impl<0>(narg);
    }

public:
    template <typename... Tags>
    decltype(auto) operator()([[maybe_unused]] const named_argument<Tags> &... nargs) const
    {
        if constexpr (sizeof...(Tags) == 0u) {
            return;
        } else if constexpr (sizeof...(Tags) == 1u) {
            return fetch_one(nargs...);
        } else {
            return ::std::forward_as_tuple(fetch_one(nargs)...);
        }
    }

private:
    template <typename Tag>
    static constexpr bool is_provided([[maybe_unused]] const named_argument<Tag> &narg)
    {
        return ::std::disjunction_v<is_provided_impl<Tag, uncvref_t<ParseArgs>>...>;
    }

public:
    template <typename... Tags>
    static constexpr bool has([[maybe_unused]] const named_argument<Tags> &... nargs)
    {
        if constexpr (sizeof...(Tags) > 0u) {
            return (... && is_provided(nargs));
        } else {
            return false;
        }
    }
    template <typename... Tags>
    static constexpr bool has_extra(const named_argument<Tags> &... nargs)
    {
        return (::std::size_t(0) + ... + static_cast<::std::size_t>(is_provided(nargs))) < sizeof...(ParseArgs);
    }

private:
    tuple_t m_nargs;
};

} // namespace igor

#define IGOR_MAKE_NAMED_ARG(name)                                                                                      \
    struct name##_tag {                                                                                                \
    };                                                                                                                 \
    inline constexpr auto name = ::igor::named_argument<name##_tag> {}

#endif
