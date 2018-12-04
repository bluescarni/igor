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

#include <igor/igor.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <type_traits>

using namespace igor;

IGOR_MAKE_NAMED_ARGUMENT(arg1);
IGOR_MAKE_NAMED_ARGUMENT(arg2);
IGOR_MAKE_NAMED_ARGUMENT(arg3);

template <typename... Args>
inline auto f_00(Args &&... args)
{
    parser p{args...};
    constexpr bool check = p.has_all(arg1, arg2);
    REQUIRE(check);
    REQUIRE(!p.has(arg3));
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg1))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg2))>);
    return p(arg1) + p(arg2);
}

template <typename... Args>
inline auto f_01(Args &&... args)
{
    parser p{args...};
    REQUIRE(p.has_all(arg1, arg2));
    REQUIRE(!p.has(arg3));
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg1))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg2))>);
    decltype(auto) a = p(arg1);
    decltype(auto) b = p(arg2);
    REQUIRE(std::is_rvalue_reference_v<decltype(a)>);
    REQUIRE(std::is_rvalue_reference_v<decltype(b)>);
    return a + b;
}

template <typename... Args>
inline auto f_02(Args &&... args)
{
    parser p{args...};
    REQUIRE(p.has_all(arg1, arg2));
    REQUIRE(!p.has(arg3));
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg1))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg2))>);
    auto [a, b] = p(arg1, arg2);
    REQUIRE(std::is_rvalue_reference_v<decltype(a)>);
    REQUIRE(std::is_rvalue_reference_v<decltype(b)>);
    return a + b;
}

template <typename... Args>
inline auto f_03(Args &&... args)
{
    parser p{args...};
    REQUIRE(p.has_all(arg1, arg2));
    REQUIRE(p.has(arg3));
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg1))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg2))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg3))>);
    constexpr bool check = p.has_other_than(arg1, arg2);
    REQUIRE(check);
    auto &&a = p(arg1);
    auto &&b = p(arg2);
    REQUIRE(std::is_rvalue_reference_v<decltype(a)>);
    REQUIRE(std::is_rvalue_reference_v<decltype(b)>);
    return a + b;
}

TEST_CASE("test_00")
{
    REQUIRE(f_00(arg1 = 5, arg2 = 6) == 11);
    REQUIRE(f_00(arg2 = -5, arg1 = 6) == 1);

    REQUIRE(f_01(arg1 = 5, arg2 = 6) == 11);
    REQUIRE(f_01(arg2 = -5, arg1 = 6) == 1);

    REQUIRE(f_02(arg1 = 5, arg2 = 6) == 11);
    REQUIRE(f_02(arg2 = -5, arg1 = 6) == 1);

    REQUIRE(f_03(arg1 = 5, arg3 = -1.2, arg2 = 6) == 11);
    REQUIRE(f_03(arg3 = 5., arg2 = -5, arg1 = 6) == 1);
}

template <typename... Args>
inline auto unnamed_00(Args &&... args)
{
    parser p{args...};
    constexpr bool hua = p.has_unnamed_arguments();
    return hua;
}

TEST_CASE("test_has_unnamed_args")
{
    REQUIRE(!unnamed_00());
    REQUIRE(unnamed_00(1));
    REQUIRE(unnamed_00(1, 2.));
    REQUIRE(unnamed_00(1, 2., "dasd"));
    REQUIRE(unnamed_00(1, arg1 = 5));
    REQUIRE(unnamed_00(arg3 = 6, 7.));
    REQUIRE(unnamed_00(arg3 = 6, 7., arg1 = ""));
    REQUIRE(!unnamed_00(arg1 = 4));
    REQUIRE(!unnamed_00(arg2 = 7, arg1 = ""));
    REQUIRE(!unnamed_00(arg3 = 7., arg1 = "dasda"));
}

template <typename... Args>
inline auto other_than_00(Args &&... args)
{
    parser p{args...};
    constexpr bool hot = p.has_other_than(arg1, arg3);
    return hot;
}

TEST_CASE("test_has_other_than")
{
    REQUIRE(!other_than_00());
    REQUIRE(!other_than_00(arg1 = 5));
    REQUIRE(!other_than_00(arg3 = 7.8));
    REQUIRE(!other_than_00(arg3 = "", arg1 = 1u));
    REQUIRE(other_than_00(arg3 = "", arg1 = 1u, arg2 = nullptr));
    REQUIRE(other_than_00(5, arg3 = "", arg1 = 1u, arg2 = nullptr));
    REQUIRE(other_than_00(arg3 = "", arg1 = 1u, arg2 = nullptr, 6));
    REQUIRE(other_than_00(arg1 = 1u, arg2 = nullptr));
    REQUIRE(other_than_00(arg2 = nullptr));
    REQUIRE(!other_than_00(42));
}

template <typename... Args>
inline auto p_has(Args &&...)
{
    constexpr bool pc = has<Args...>(arg1);
    return pc;
}

TEST_CASE("test_pack_has")
{
    REQUIRE(!p_has());
    REQUIRE(!p_has(1));
    REQUIRE(!p_has("hello"));
    REQUIRE(p_has(arg1 = 5, "hello"));
    REQUIRE(p_has(arg1 = 6.5));
    REQUIRE(p_has(1.5, arg1 = "hello"));
}
