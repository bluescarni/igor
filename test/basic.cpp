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
#include <initializer_list>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <igor/igor.hpp>

#include <catch2/catch_test_macros.hpp>

// clang-format off
// NOLINTBEGIN(misc-use-internal-linkage,google-build-using-namespace,cppcoreguidelines-avoid-do-while,misc-use-anonymous-namespace,cert-err58-cpp)
// clang-format on

using namespace igor;

constexpr auto arg1 = make_named_argument();
IGOR_MAKE_NAMED_ARGUMENT(arg2);
constexpr auto arg3 = make_named_argument();
constexpr auto arg4 = make_named_argument<const char *&&>();
constexpr auto arg5 = make_named_argument<const double &>();

template <typename... Args>
inline auto f_00(const Args &...args)
{
    const parser p{args...};
    constexpr bool check = p.has_all(arg1, arg2);
    REQUIRE(check);
    REQUIRE(!p.has(arg3));
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg1))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg2))>);
    return p(arg1) + p(arg2);
}

template <typename... Args>
inline auto f_01(const Args &...args)
{
    const parser p{args...};
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
inline auto f_02(const Args &...args)
{
    const parser p{args...};
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
inline auto f_03(const Args &...args)
{
    const parser p{args...};
    REQUIRE(p.has_all(arg1, arg5, arg2));
    REQUIRE(p.has(arg3));
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg1))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg2))>);
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg3))>);
    REQUIRE(std::is_lvalue_reference_v<decltype(p(arg5))>);
    constexpr bool check = p.has_other_than(arg1, arg2);
    REQUIRE(check);
    auto &&a = p(arg1);
    auto &&b = p(arg2);
    REQUIRE(std::is_rvalue_reference_v<decltype(a)>);
    REQUIRE(std::is_rvalue_reference_v<decltype(b)>);
    return a + b;
}

template <typename... Args>
inline auto f_04(const Args &...args)
{
    const parser p{args...};
    constexpr auto ha = p.has_any(arg1, arg3, arg5);
    return ha;
}

template <typename... Args>
inline auto f_05(const Args &...args)
{
    const parser p{args...};
    REQUIRE(p.has(arg1));
    REQUIRE(std::is_lvalue_reference_v<decltype(p(arg1))>);
}

TEST_CASE("test_has")
{
    REQUIRE(f_00(arg1 = 5, arg2 = 6) == 11);
    REQUIRE(f_00(arg2 = -5, arg1 = 6) == 1);

    REQUIRE(f_01(arg1 = 5, arg2 = 6) == 11);
    REQUIRE(f_01(arg2 = -5, arg1 = 6) == 1);

    REQUIRE(f_02(arg1 = 5, arg2 = 6) == 11);
    REQUIRE(f_02(arg2 = -5, arg1 = 6) == 1);

    REQUIRE(f_03(arg1 = 5, arg5 = {0.0}, arg3 = -1.2, arg2 = 6) == 11);
    REQUIRE(f_03(arg3 = 5., arg2 = -5, arg1 = 6, arg5 = {0.0}) == 1);

    REQUIRE(f_04(arg1 = 5));
    REQUIRE(f_04(arg3 = 5.6, arg1 = 5));
    REQUIRE(f_04(arg2 = "", arg1 = 5));
    REQUIRE(f_04(arg3 = "dsdas"));
    REQUIRE(f_04(arg5 = {0.0}));
    REQUIRE(!f_04(arg2 = "dsdas"));
    REQUIRE(!f_04(arg4 = {nullptr}));
    REQUIRE(!f_04());

    {
        int n = 5;
        f_05(arg1 = n);
        std::string s = "hello";
        f_05(arg1 = s);
    }
}

template <typename... Args>
inline auto unnamed_00(const Args &...args)
{
    const parser p{args...};
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
inline auto other_than_00(const Args &...args)
{
    const parser p{args...};
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

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
struct move_only {
    move_only() = default;
    move_only(move_only &&) = default;

    // Remove the copy constructor.
    move_only(const move_only &) = delete;
};

template <typename... Args>
inline void move_argument(const Args &...args)
{
    const parser p{args...};
    REQUIRE(std::is_rvalue_reference_v<decltype(p(arg1))>);
    [[maybe_unused]] const move_only inner{std::move(p(arg1))};
}

TEST_CASE("test_move_only")
{
    move_only mo;
    move_argument(arg1 = std::move(mo));
    move_argument(arg1 = move_only{});
}

template <typename... Args>
inline auto test_init_list(const Args &...args)
{
    const parser p{args...};
    return p(arg1);
}

TEST_CASE("test_init_list")
{
    REQUIRE((std::vector(test_init_list(arg1 = {1, 2, 3, 4})) == std::vector{1, 2, 3, 4}));
}

template <typename T, typename U>
inline void inner(T &&, U &&)
{
    REQUIRE(std::is_rvalue_reference_v<T &&>);
    REQUIRE(!std::is_const_v<std::remove_reference_t<T &&>>);
    REQUIRE(std::is_lvalue_reference_v<U &&>);
    REQUIRE(std::is_const_v<std::remove_reference_t<U &&>>);
}

template <typename... Args>
inline void outer(const Args &...args)
{
    const parser p{args...};
    auto [a, b] = p(arg1, arg2);
    inner(std::forward<decltype(a)>(a), std::forward<decltype(b)>(b));
}

TEST_CASE("test_perfect_forward")
{
    const std::string f = "foo";
    outer(arg1 = 5, arg2 = f);
    outer(arg2 = f, arg1 = move_only{});
}

template <typename... Args>
inline bool not_provided_test(const Args &...args)
{
    const parser p{args...};
    auto [a, b] = p(arg1, arg2);
    if constexpr (std::is_same_v<decltype(a), const not_provided_t &>) {
        return &a == &not_provided;
    } else {
        return false;
    }
}

TEST_CASE("test_not_provided")
{
    REQUIRE(not_provided_test());
    REQUIRE(not_provided_test(arg2 = 5));
    REQUIRE(not_provided_test(arg3 = "dsada", arg2 = 5));
    REQUIRE(!not_provided_test(arg1 = 5.));
    REQUIRE(!not_provided_test(arg3 = 6, arg1 = 5.));
}

template <typename... Args>
inline bool has_duplicates_test(const Args &...args)
{
    const parser p{args...};
    return static_cast<bool>(p.has_duplicates());
}

TEST_CASE("test_has_duplicates")
{
    REQUIRE(!has_duplicates_test());
    REQUIRE(!has_duplicates_test(1));
    REQUIRE(!has_duplicates_test(1, "adsda"));
    REQUIRE(!has_duplicates_test(1, "adsda", 3.5));
    REQUIRE(!has_duplicates_test(arg1 = 5, "adsda", arg3 = 56.));
    REQUIRE(!has_duplicates_test(arg1 = 5, arg2 = "dasda", arg3 = 56.));
    REQUIRE(has_duplicates_test(arg1 = 5, arg1 = 6));
    REQUIRE(has_duplicates_test(arg2 = 4, arg2 = 56, arg1 = 5, arg1 = 6));
    REQUIRE(has_duplicates_test(arg1 = 4, arg2 = 56, arg2 = 5, arg1 = 6));
    REQUIRE(has_duplicates_test(arg1 = 4, arg2 = 56, arg2 = 5, arg1 = 6, arg3 = 5.6));
    REQUIRE(has_duplicates_test(arg3 = "Hello", arg1 = 4, arg2 = 56, arg2 = 5, arg1 = 6));
}

template <typename... Args>
constexpr auto sum(const Args &...args)
{
    const parser p{args...};

    if constexpr (!p.has_all(arg1, arg2, arg3)) {
        return -1;
    } else {
        auto [a1, a2, a3] = p(arg1, arg2, arg3);
        return a1 + (a2 * a3);
    }
}

TEST_CASE("constexprness")
{
    constexpr auto resultOfSum = sum(arg2 = 8, arg1 = 0.5, arg3 = 7);
    constexpr auto notEnoughArgs = sum(arg3 = 4, arg1 = 6);

    REQUIRE(resultOfSum == 56.5);
    REQUIRE(notEnoughArgs == -1);
}

template <typename... Args>
inline bool has_only_cstring_allowed_test(const Args &...args)
{
    const parser p{args...};
    REQUIRE((!p.has(arg4) || std::is_same_v<decltype(p(arg4)), const char *&&>));
    return p.has(arg4);
}

template <typename... Args>
inline bool has_only_cdoubleref_allowed_test(const Args &...args)
{
    const parser p{args...};
    REQUIRE((!p.has(arg5) || std::is_same_v<decltype(p(arg5)), const double &>));
    return p.has(arg5);
}

TEST_CASE("explicit_typed_arguments")
{
    const char *testStr = "hello";
    double d = 0.0;
    const double cd = d;

    // "hello" is const char[6] &&, not const char*&&
    REQUIRE(!std::is_assignable_v<decltype(arg4), int>);
    REQUIRE(!std::is_assignable_v<decltype(arg4), decltype("hello")>);
    REQUIRE(!std::is_assignable_v<decltype(arg4), decltype((testStr))>);
    REQUIRE(has_only_cstring_allowed_test(arg4 = static_cast<const char *>("hello")));
    REQUIRE(has_only_cstring_allowed_test(arg4 = {"hello"}));
    // NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg,bugprone-use-after-move,hicpp-invalid-access-moved)
    REQUIRE(has_only_cstring_allowed_test(arg4 = std::move(testStr)));
    REQUIRE(!has_only_cstring_allowed_test());

    REQUIRE(!std::is_assignable_v<decltype(arg5), const char *>);
    REQUIRE(!std::is_assignable_v<decltype(arg5), int>);
    REQUIRE(!std::is_assignable_v<decltype(arg5), double>);
    REQUIRE(!std::is_assignable_v<decltype(arg5), double &>);
    REQUIRE(has_only_cdoubleref_allowed_test(arg5 = {0}));
    REQUIRE(has_only_cdoubleref_allowed_test(arg5 = {0.0}));
    REQUIRE(has_only_cdoubleref_allowed_test(arg5 = {d}));
    REQUIRE(has_only_cdoubleref_allowed_test(arg5 = std::as_const(d)));
    REQUIRE(has_only_cdoubleref_allowed_test(arg5 = cd));
}

template <typename... Args>
inline auto repeated_args(const Args &...args)
{
    const parser p{args...};

    return p(arg1);
}

TEST_CASE("repeated_arguments")
{
    REQUIRE(repeated_args(arg1 = 5) == 5);
    REQUIRE(repeated_args(arg1 = 5, arg1 = 6) == 5);
    REQUIRE(repeated_args(arg1 = 5, arg1 = 6, arg1 = 7) == 5);
}

template <typename... Args>
decltype(auto) as_const_inner(const Args &...args)
{
    const parser p{args...};

    return p(arg1);
}

template <typename... Args>
decltype(auto) as_const_outer(const Args &...args)
{
    return as_const_inner(as_const(args...));
}

TEST_CASE("as_const")
{
    REQUIRE((std::same_as<const int &, decltype(as_const_outer(arg1 = 5))>));
    // NOLINTNEXTLINE(misc-const-correctness)
    std::string foo = "hello world";
    REQUIRE((std::same_as<const std::string &, decltype(as_const_outer(arg1 = foo))>));
}

TEST_CASE("reject")
{
    REQUIRE(reject() == std::tuple{});
    REQUIRE((std::same_as<decltype(reject(1, 2.3)), std::tuple<int &&, double &&>>));

    {
        const std::string foo = "hello world";
        double x = 42;
        auto ret = reject(foo, x);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret) == &x);
        REQUIRE((std::same_as<decltype(ret), std::tuple<const std::string &, double &>>));
    }

    {
        const std::string foo = "hello world";
        double x = 42;
        auto ret = reject(foo, x);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret) == &x);
        REQUIRE((std::same_as<decltype(ret), std::tuple<const std::string &, double &>>));
    }

    {
        const std::string foo = "hello world";
        double x = 42;
        int n = 25;
        const auto tref = (arg1 = n);
        auto ret = reject<arg2>(foo, tref, x, arg2 = 6.7f);
        REQUIRE(std::tuple_size_v<decltype(ret)> == 3u);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret).value == &n);
        REQUIRE(&std::get<2>(ret) == &x);
    }

    {
        const std::string foo = "hello world";
        double x = 42;
        int n = 25;
        const auto tref1 = (arg1 = n);
        auto f = 6.7f;
        const auto tref2 = (arg2 = f);
        auto ret = reject<arg1, arg2>(foo, tref1, x, tref2);
        REQUIRE(std::tuple_size_v<decltype(ret)> == 2u);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret) == &x);
    }
}

TEST_CASE("reject cfg")
{
    constexpr auto cfg1 = config<descr<arg1>{}, descr<arg2>{}>{};

    {
        const std::string foo = "hello world";
        auto ret = reject<cfg1>(foo, arg2 = 6.7f);
        REQUIRE(std::tuple_size_v<decltype(ret)> == 1u);
        REQUIRE(&std::get<0>(ret) == &foo);
    }

    {
        const std::string foo = "hello world";
        int n = 6;
        auto ret = reject<cfg1>(foo, arg2 = 6.7f, arg1 = 5, n);
        REQUIRE(std::tuple_size_v<decltype(ret)> == 2u);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret) == &n);
    }
}

TEST_CASE("filter")
{
    REQUIRE(filter() == std::tuple{});
    REQUIRE((std::same_as<decltype(filter(1, 2.3)), std::tuple<int &&, double &&>>));

    {
        const std::string foo = "hello world";
        double x = 42;
        auto ret = filter(foo, x);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret) == &x);
        REQUIRE((std::same_as<decltype(ret), std::tuple<const std::string &, double &>>));
    }

    {
        const std::string foo = "hello world";
        double x = 42;
        auto ret = filter(foo, x);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret) == &x);
        REQUIRE((std::same_as<decltype(ret), std::tuple<const std::string &, double &>>));
    }

    {
        const std::string foo = "hello world";
        double x = 42;
        int n = 25;
        const auto tref = (arg1 = n);
        auto ret = filter<arg1>(foo, tref, x, arg2 = 6.7f);
        REQUIRE(std::tuple_size_v<decltype(ret)> == 3u);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret).value == &n);
        REQUIRE(&std::get<2>(ret) == &x);
    }

    {
        const std::string foo = "hello world";
        double x = 42;
        int n = 25;
        const auto tref1 = (arg1 = n);
        auto f = 6.7f;
        const auto tref2 = (arg2 = f);
        auto ret = filter<arg2>(foo, tref1, x, tref2);
        REQUIRE(std::tuple_size_v<decltype(ret)> == 3u);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret) == &x);
        REQUIRE(&std::get<2>(ret).value == &f);
    }
}

TEST_CASE("filter cfg")
{
    constexpr auto cfg1 = config<descr<arg1>{}, descr<arg2>{}>{};

    {
        const std::string foo = "hello world";
        auto tmp = 6.7f;
        const auto tref = (arg2 = tmp);
        auto ret = filter<cfg1>(foo, tref);
        REQUIRE(std::tuple_size_v<decltype(ret)> == 2u);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret).value == &tmp);
    }

    {
        const std::string foo = "hello world";
        auto tmp1 = 42;
        const auto tref1 = (arg1 = tmp1);
        auto tmp2 = 6.7f;
        const auto tref2 = (arg2 = tmp2);
        int n = 6;
        auto ret = filter<cfg1>(foo, tref2, tref1, n);
        REQUIRE(std::tuple_size_v<decltype(ret)> == 4u);
        REQUIRE(&std::get<0>(ret) == &foo);
        REQUIRE(&std::get<1>(ret).value == &tmp2);
        REQUIRE(&std::get<2>(ret).value == &tmp1);
        REQUIRE(&std::get<3>(ret) == &n);
    }
}

TEST_CASE("default value")
{
    {
        const auto f = []<typename... KwArgs>(const KwArgs &...kw_args) {
            const parser p{kw_args...};
            return p(arg1, 42);
        };

        REQUIRE(f(arg1 = -1.) == -1.);
        REQUIRE(f() == 42);
    }

    {
        const auto f = []<typename... KwArgs>(const KwArgs &...kw_args) {
            const parser p{kw_args...};
            return p(arg1, std::vector{1, 2, 3});
        };

        REQUIRE(f(arg1 = -1.) == -1.);
        REQUIRE(f() == std::vector{1, 2, 3});
    }
}

TEST_CASE("reject_invoke")
{
    static constexpr auto cfg1 = config<descr<arg1>{}, descr<arg2>{}>{};

    {
        const auto f = [](const auto &...kw_args) {
            return reject_invoke<cfg1>(
                [](const auto &...fargs) {
                    const auto impl = []<typename T>(const T &arg) {
                        if constexpr (detail::is_tagged_ref_any<T>::value) {
                            return arg.value;
                        } else {
                            return arg;
                        }
                    };

                    return std::make_tuple(impl(fargs)...);
                },
                kw_args...);
        };

        REQUIRE(f(arg1 = 1, 2, 3, arg2 = 4) == std::tuple{2, 3});
        REQUIRE(f(arg1 = 1, arg3 = 12, 2, 3, arg2 = 4) == std::tuple{12, 2, 3});
    }

    // NOTE: same test with a set of named arguments in input, rather than a config instance.
    {
        const auto f = [](const auto &...kw_args) {
            return reject_invoke<arg1, arg2>(
                [](const auto &...fargs) {
                    const auto impl = []<typename T>(const T &arg) {
                        if constexpr (detail::is_tagged_ref_any<T>::value) {
                            return arg.value;
                        } else {
                            return arg;
                        }
                    };

                    return std::make_tuple(impl(fargs)...);
                },
                kw_args...);
        };

        REQUIRE(f(arg1 = 1, 2, 3, arg2 = 4) == std::tuple{2, 3});
        REQUIRE(f(arg1 = 1, arg3 = 12, 2, 3, arg2 = 4) == std::tuple{12, 2, 3});
    }

    {
        // NOTE: here we want to check that we are correctly perfectly forwarding the result of the functor passed to
        // reject_invoke().
        const std::string s = "hello world";

        const auto f = [&s](const auto &...kw_args) -> decltype(auto) {
            return reject_invoke<cfg1>([&s](const auto &...) -> decltype(auto) { return std::as_const(s); },
                                       kw_args...);
        };

        REQUIRE(&f(arg1 = 1, 2, 3, arg2 = 4) == &s);
        REQUIRE(&f(arg1 = 1, arg3 = 12, 2, 3, arg2 = 4) == &s);
    }
}

TEST_CASE("filter_invoke")
{
    static constexpr auto cfg1 = config<descr<arg1>{}, descr<arg2>{}>{};

    {
        const auto f = [](const auto &...kw_args) {
            return filter_invoke<cfg1>(
                [](const auto &...fargs) {
                    const auto impl = []<typename T>(const T &arg) {
                        if constexpr (detail::is_tagged_ref_any<T>::value) {
                            return arg.value;
                        } else {
                            return arg;
                        }
                    };

                    return std::make_tuple(impl(fargs)...);
                },
                kw_args...);
        };

        REQUIRE(f(arg1 = 1, 2, 3, arg2 = 4) == std::tuple{1, 2, 3, 4});
        REQUIRE(f(arg1 = 1, arg3 = 12, 2, 3, arg2 = 4) == std::tuple{1, 2, 3, 4});
    }

    // NOTE: same test with a set of named arguments in input, rather than a config instance.
    {
        const auto f = [](const auto &...kw_args) {
            return filter_invoke<arg2, arg1>(
                [](const auto &...fargs) {
                    const auto impl = []<typename T>(const T &arg) {
                        if constexpr (detail::is_tagged_ref_any<T>::value) {
                            return arg.value;
                        } else {
                            return arg;
                        }
                    };

                    return std::make_tuple(impl(fargs)...);
                },
                kw_args...);
        };

        REQUIRE(f(arg1 = 1, 2, 3, arg2 = 4) == std::tuple{1, 2, 3, 4});
        REQUIRE(f(arg1 = 1, arg3 = 12, 2, 3, arg2 = 4) == std::tuple{1, 2, 3, 4});
    }

    {
        // NOTE: here we want to check that we are correctly perfectly forwarding the result of the functor passed to
        // filter_invoke().
        const std::string s = "hello world";

        const auto f = [&s](const auto &...kw_args) -> decltype(auto) {
            return filter_invoke<cfg1>([&s](const auto &...) -> decltype(auto) { return std::as_const(s); },
                                       kw_args...);
        };

        REQUIRE(&f(arg1 = 1, 2, 3, arg2 = 4) == &s);
        REQUIRE(&f(arg1 = 1, arg3 = 12, 2, 3, arg2 = 4) == &s);
    }
}

// clang-format off
// NOLINTEND(misc-use-internal-linkage,google-build-using-namespace,cppcoreguidelines-avoid-do-while,misc-use-anonymous-namespace,cert-err58-cpp)
// clang-format on
