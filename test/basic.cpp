// Copyright 2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the igor library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <igor/igor.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <type_traits>

using namespace igor;

IGOR_MAKE_NAMED_ARGUMENT(arg1);
IGOR_MAKE_NAMED_ARGUMENT(arg2);
IGOR_MAKE_NAMED_ARGUMENT(arg3);

template <typename... Args>
auto f_00(Args &&... args)
{
    parser p{args...};
    constexpr bool check = p.has(arg1, arg2);
    REQUIRE(check);
    REQUIRE(!p.has(arg3));
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg1))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg2))>);
    return p(arg1) + p(arg2);
}

template <typename... Args>
auto f_01(Args &&... args)
{
    parser p{args...};
    REQUIRE(p.has(arg1, arg2));
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
auto f_02(Args &&... args)
{
    parser p{args...};
    REQUIRE(p.has(arg1, arg2));
    REQUIRE(!p.has(arg3));
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg1))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg2))>);
    auto [a, b] = p(arg1, arg2);
    return a + b;
}

template <typename... Args>
auto f_03(Args &&... args)
{
    parser p{args...};
    REQUIRE(p.has(arg1, arg2));
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
