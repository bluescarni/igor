// Copyright 2018-2020 Francesco Biscani
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

#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <utility>

namespace igor
{

namespace detail
{

// This is the class used to store a reference to a function argument. An object of this type is returned by
// named_argument's assignment operator.
template <typename Tag, typename T>
    requires(std::is_reference_v<T>)
struct tagged_ref {
    using tag_type = Tag;
    T value;
};

} // namespace detail

// Helper to turn a tagged reference into another tagged reference containing a const reference to the original
// reference. This is useful in order to enforce const reference access semantics to an argument (in the same spirit as
// std::as_const()).
template <typename Tag, typename T>
auto as_const_kwarg(const detail::tagged_ref<Tag, T> &tc)
{
    return detail::tagged_ref<Tag, decltype(std::as_const(tc.value))>{std::as_const(tc.value)};
}

// Class to represent a named argument.
template <typename Tag, typename ExplicitType = void>
struct named_argument {
    using tag_type = Tag;

    template <typename T>
    // NOTE: make sure this does not interfere with the copy/move assignment operators.
        requires(!std::same_as<named_argument, std::remove_cvref_t<T>>)
    constexpr auto operator=(T &&x) const
    {
        return detail::tagged_ref<Tag, T &&>{std::forward<T>(x)};
    }

    // Add overloads for std::initializer_list as well.
    template <typename T>
    constexpr auto operator=(const std::initializer_list<T> &l) const
    {
        return detail::tagged_ref<Tag, const std::initializer_list<T> &>{l};
    }
    template <typename T>
    constexpr auto operator=(std::initializer_list<T> &l) const
    {
        return detail::tagged_ref<Tag, std::initializer_list<T> &>{l};
    }
    template <typename T>
    constexpr auto operator=(std::initializer_list<T> &&l) const
    {
        return detail::tagged_ref<Tag, std::initializer_list<T> &&>{std::move(l)};
    }
    template <typename T>
    constexpr auto operator=(const std::initializer_list<T> &&l) const
    {
        return detail::tagged_ref<Tag, const std::initializer_list<T> &&>{std::move(l)};
    }
};

template <typename Tag, typename ExplicitType>
    requires(std::is_reference_v<ExplicitType>)
struct named_argument<Tag, ExplicitType> {
    using tag_type = Tag;

    using value_type = ExplicitType;

    // NOTE: disable implicit conversion, deduced type needs to be the same as explicit type.
    template <typename T>
        requires std::same_as<T &&, ExplicitType>
    constexpr auto operator=(T &&x) const
    {
        return detail::tagged_ref<Tag, ExplicitType>{std::forward<T>(x)};
    }

    // NOTE: enable implicit conversion with curly braces
    // and copy-list/aggregate initialization with double curly braces.
    constexpr auto operator=(detail::tagged_ref<Tag, ExplicitType> &&tc) const
    {
        return std::move(tc);
    }

    template <typename T>
        requires(!std::same_as<T &&, ExplicitType>)
    auto operator=(T &&) const = delete; // please use {...} to typed argument implicit conversion
};

// Type representing a named argument which
// was not provided in a function call.
struct not_provided_t {
};

// Non-provided named arguments will return a const reference
// to this global object.
inline constexpr not_provided_t not_provided;

namespace detail
{

// Type trait to detect if T is a tagged reference with tag Tag (and any type as second parameter).
template <typename Tag, typename T>
struct is_tagged_ref : std::false_type {
};

template <typename Tag, typename T>
struct is_tagged_ref<Tag, tagged_ref<Tag, T>> : std::true_type {
};

// Type trait to detect if T is a tagged reference (regardless of the tag type or the type
// of the second parameter).
template <typename T>
struct is_tagged_ref_any : std::false_type {
};

template <typename Tag, typename T>
struct is_tagged_ref_any<tagged_ref<Tag, T>> : std::true_type {
};

template <typename>
struct is_named_argument_any : std::false_type {
};

template <typename Tag, typename ExplicitType>
struct is_named_argument_any<named_argument<Tag, ExplicitType>> : std::true_type {
};

// Implementation of parsers' constructor.
//
// This function will take a set of input arguments (as const ref) and will filter out the named arguments (which are
// returned as a tuple of const references).
template <typename... Args>
constexpr inline auto build_parser_tuple(const Args &...args)
{
    [[maybe_unused]] const auto filter_na = []<typename T>(const T &x) {
        if constexpr (is_tagged_ref_any<T>::value) {
            return std::forward_as_tuple(x);
        } else {
            return std::tuple{};
        }
    };

    return std::tuple_cat(filter_na(args)...);
}

} // namespace detail

template <auto narg, typename... Args>
concept hasso = requires() {
    requires(detail::is_named_argument_any<std::remove_cv_t<decltype(narg)>>::value);
    requires(...
             || detail::is_tagged_ref<typename std::remove_cv_t<decltype(narg)>::tag_type,
                                      std::remove_cvref_t<Args>>::value);
};

// NOTE: implement some of the parser functionality as free functions, which will then be wrapped by static constexpr
// member functions in the parser class. These free functions can be used where a parser object is not available (e.g.,
// in a requires clause).
template <typename... Args, typename Tag, typename ExplicitType>
consteval bool has([[maybe_unused]] const named_argument<Tag, ExplicitType> &narg)
{
    return (... || detail::is_tagged_ref<Tag, std::remove_cvref_t<Args>>::value);
}

template <typename... Args, typename... Tags, typename... ExplicitTypes>
consteval bool has_all(const named_argument<Tags, ExplicitTypes> &...nargs)
{
    return (... && igor::has<Args...>(nargs));
}

template <typename... Args, typename... Tags, typename... ExplicitTypes>
consteval bool has_any(const named_argument<Tags, ExplicitTypes> &...nargs)
{
    return (... || igor::has<Args...>(nargs));
}

template <typename... Args>
consteval bool has_unnamed_arguments()
{
    return (... || !detail::is_tagged_ref_any<std::remove_cvref_t<Args>>::value);
}

template <typename... Args, typename... Tags, typename... ExplicitTypes>
consteval bool has_other_than(const named_argument<Tags, ExplicitTypes> &...nargs)
{
    // NOTE: the first fold expression will return how many of the nargs
    // are in the pack. The second fold expression will return the total number
    // of named arguments in the pack.
    return (std::size_t(0) + ... + static_cast<std::size_t>(igor::has<Args...>(nargs)))
           < (std::size_t(0) + ...
              + static_cast<std::size_t>(detail::is_tagged_ref_any<std::remove_cvref_t<Args>>::value));
}

namespace detail
{

// Check if T is a named argument which appears more than once in Args.
template <typename T, typename... Args>
consteval bool is_repeated_named_argument()
{
    if constexpr (is_tagged_ref_any<std::remove_cvref_t<T>>::value) {
        return (std::size_t(0) + ...
                + static_cast<std::size_t>(std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<Args>>))
               > 1u;
    } else {
        return false;
    }
}

} // namespace detail

template <typename... Args>
consteval bool has_duplicates()
{
    return (... || detail::is_repeated_named_argument<Args, Args...>());
}

// Parser for named arguments in a function call.
template <typename... ParseArgs>
class parser
{
    using tuple_t = decltype(detail::build_parser_tuple(std::declval<const ParseArgs &>()...));

public:
    constexpr explicit parser(const ParseArgs &...parse_args) : m_nargs(detail::build_parser_tuple(parse_args...)) {}

private:
    // Fetch the value associated to the input named
    // argument narg. If narg is not present, this will
    // return a const ref to a global not_provided_t object.
    template <std::size_t I, typename Tag, typename ExplicitType>
    constexpr decltype(auto) fetch_one_impl([[maybe_unused]] const named_argument<Tag, ExplicitType> &narg) const
    {
        if constexpr (I == std::tuple_size_v<tuple_t>) {
            return static_cast<const not_provided_t &>(not_provided);
        } else if constexpr (std::is_same_v<typename std::remove_cvref_t<std::tuple_element_t<I, tuple_t>>::tag_type,
                                            Tag>) {
            if constexpr (std::is_rvalue_reference_v<decltype(std::get<I>(m_nargs).value)>) {
                return std::move(std::get<I>(m_nargs).value);
            } else {
                return std::get<I>(m_nargs).value;
            }
        } else {
            return fetch_one_impl<I + 1u>(narg);
        }
    }

public:
    // Get references to the values associated to the input named arguments.
    template <typename... Tags, typename... ExplicitTypes>
    constexpr decltype(auto) operator()([[maybe_unused]] const named_argument<Tags, ExplicitTypes> &...nargs) const
    {
        if constexpr (sizeof...(Tags) == 0u) {
            return;
        } else if constexpr (sizeof...(Tags) == 1u) {
            return this->fetch_one_impl<0>(nargs...);
        } else {
            return std::forward_as_tuple(this->fetch_one_impl<0>(nargs)...);
        }
    }
    // Check if the input named argument na is present in the parser.
    template <typename Tag, typename ExplicitType>
    static consteval bool has(const named_argument<Tag, ExplicitType> &narg)
    {
        return igor::has<ParseArgs...>(narg);
    }
    // Check if all the input named arguments nargs are present in the parser.
    template <typename... Tags, typename... ExplicitTypes>
    static consteval bool has_all(const named_argument<Tags, ExplicitTypes> &...nargs)
    {
        return igor::has_all<ParseArgs...>(nargs...);
    }
    // Check if at least one of the input named arguments nargs is present in the parser.
    template <typename... Tags, typename... ExplicitTypes>
    static consteval bool has_any(const named_argument<Tags, ExplicitTypes> &...nargs)
    {
        return igor::has_any<ParseArgs...>(nargs...);
    }
    // Detect the presence of unnamed arguments.
    static consteval bool has_unnamed_arguments()
    {
        return igor::has_unnamed_arguments<ParseArgs...>();
    }
    // Check if the parser contains named arguments other than nargs.
    template <typename... Tags, typename... ExplicitTypes>
    static consteval bool has_other_than(const named_argument<Tags, ExplicitTypes> &...nargs)
    {
        return igor::has_other_than<ParseArgs...>(nargs...);
    }
    // Check if the parser contains duplicate named arguments (that is, check
    // if at least one named argument appears more than once).
    static consteval bool has_duplicates()
    {
        return igor::has_duplicates<ParseArgs...>();
    }

private:
    tuple_t m_nargs;
};

template <typename ExplicitType = void, typename T = decltype([] {})>
consteval auto make_named_argument()
{
    return named_argument<T, ExplicitType>{};
}

} // namespace igor

#endif
