// Copyright 2018-2025 Francesco Biscani
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

#include <concepts>
#include <type_traits>

#include <igor/igor.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace igor;

constexpr auto arg1 = make_named_argument();
constexpr auto arg2 = make_named_argument();
constexpr auto arg3 = make_named_argument();
constexpr auto arg4 = make_named_argument();

// Invalid validator (non-constexpr call operator).
struct invalid_validator {
    template <typename>
    bool operator()() const
    {
        return true;
    }
};

struct valid_validator {
    template <typename>
    constexpr bool operator()() const
    {
        return true;
    }
};

TEST_CASE("valid descr validator concept")
{
    // NOTE: I think GCC has a bug here: concept will evaluate to true despite the fact that the invalid validator's
    // call operator is not usable in a constant expression.
#if defined(__clang__) || defined(_MSC_VER)
    REQUIRE(!valid_descr_validator<invalid_validator{}, int>);
#endif
    REQUIRE(valid_descr_validator<valid_validator{}, int>);
}

constexpr auto cfg_simple_validation
    = config<descr<arg1>{.required = true}, descr<arg2>{},
             descr<arg3, []<typename T>() { return std::integral<std::remove_cvref_t<T>>; }>{}>{};

template <typename... KwArgs>
bool simple_validation(const KwArgs &...)
{
    return validate<cfg_simple_validation, KwArgs...>;
}

TEST_CASE("simple validation")
{
    REQUIRE(!simple_validation());
    REQUIRE(simple_validation(arg1 = 1));
    REQUIRE(!simple_validation(arg2 = 1));
    REQUIRE(simple_validation(arg1 = 1, arg2 = 2));
    REQUIRE(simple_validation(arg1 = 1, arg3 = 2, arg2 = 2));
    REQUIRE(!simple_validation(arg1 = 1, arg3 = 2.1, arg2 = 2));
    REQUIRE(!simple_validation(arg1 = 1, arg3 = 2, arg2 = 2, 123));
    REQUIRE(!simple_validation(arg1 = 1, arg3 = 2, arg2 = 2, arg4 = 5));
}

constexpr auto cfg_allow_unnamed
    = config<descr<arg1>{.required = true}, descr<arg2>{},
             descr<arg3, []<typename T>() { return std::integral<std::remove_cvref_t<T>>; }>{}>{.allow_unnamed = true};

template <typename... KwArgs>
bool allow_unnamed_validation(const KwArgs &...)
{
    return validate<cfg_allow_unnamed, KwArgs...>;
}

TEST_CASE("allow unnamed")
{
    REQUIRE(allow_unnamed_validation(arg1 = 1, arg3 = 2, arg2 = 2, 123));
}

constexpr auto cfg_allow_extra
    = config<descr<arg1>{.required = true}, descr<arg2>{},
             descr<arg3, []<typename T>() { return std::integral<std::remove_cvref_t<T>>; }>{}>{.allow_extra = true};

template <typename... KwArgs>
bool allow_extra_validation(const KwArgs &...)
{
    return validate<cfg_allow_extra, KwArgs...>;
}

TEST_CASE("allow extra")
{
    REQUIRE(allow_extra_validation(arg1 = 1, arg3 = 2, arg2 = 2, arg4 = 5));
}

constexpr auto cfg_wrong_validator
    = config<descr<arg1>{.required = true}, descr<arg2>{}, descr<arg3, []() {}>{}>{.allow_extra = true};

template <typename... KwArgs>
bool wrong_validator_validation(const KwArgs &...)
{
    return validate<cfg_wrong_validator, KwArgs...>;
}

TEST_CASE("wrong validator")
{
    REQUIRE(!wrong_validator_validation(arg1 = 1, arg3 = 2, arg2 = 2, arg4 = 5));
}
