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

#include <igor/igor.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace igor;

constexpr auto arg1 = make_named_argument();

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
    // NOTE: I think GCC has a bug here.
#if defined(__clang__) || defined(_MSC_VER)
    REQUIRE(!igor::valid_descr_validator<invalid_validator{}, int>);
#endif
    REQUIRE(igor::valid_descr_validator<valid_validator{}, int>);
}

#if 0 

template <typename... KwArgs>
    requires(validate<KwArgs...>(
        config<descr<arg1, []<typename T>() { return std::integral<std::remove_cvref_t<T>>; }>{}>{}))
void foo(const KwArgs &...)
{
}

struct flap {
    template <typename>
    bool operator()() const
    {
        return true;
    }
};

TEST_CASE("validation")
{
    foo(arg1 = 1);
    REQUIRE(!valid_descr_validator<flap{}, int>);
}

#endif
