# igor

![language](https://img.shields.io/badge/language-C%2B%2B20-red.svg?style=for-the-badge)
![license](https://img.shields.io/badge/license-MIT-blue.svg?style=for-the-badge)

igor (pronounced *eye-gor*) is a small, unpretentious, self-contained and header-only C++20
library implementing named function arguments (AKA keyword arguments, named parameters, etc.).
A minimal example:

```c++
#include <iostream>
#include <string>

#include <igor/igor.hpp>

using namespace igor;

// Create named arguments called "arg1" and "arg2".
inline constexpr auto arg1 = make_named_argument();
inline constexpr auto arg2 = make_named_argument();

// A variadic function accepting named arguments.
template <typename ... Args>
auto adder(Args && ... args)
{
    parser p{args...};
    return p(arg1) + p(arg2);
}

int main()
{
    using namespace std::literals;

    // This will print "hello, world".
    std::cout << adder(arg1 = "hello, "s, arg2 = "world") << '\n';

    // This will print "42".
    std::cout << "The ultimate answer is: " << adder(arg2 = 20, arg1 = 22) << '\n';

    return 0;
}
```

igor is partly inspired by Python's ``kwargs`` machinery, and it is meant for use in conjunction with
variadic templates. If you cannot (or do not want to) use templates, igor might not be the best fit
for your needs, and you might want to investigate other libraries such as
[Boost.Parameter](https://www.boost.org/doc/libs/1_68_0/libs/parameter/doc/html/index.html) or
[argo](https://github.com/rmpowell77/LIAW_2017_param).

## How does it work?

A ``parser`` object identifies named arguments upon construction, and stores internally
references to the values associated to the named arguments. Later, these references can be fecthed via
``parser``'s call operator. Named arguments can be passed in any order and they can be associated to values
of any type.

## How do I check which named arguments were provided?

Like this:

```c++
template <typename ... Args>
void arg_check(Args && ... args)
{
    parser p{args...};

    if (p.has(arg1)) {
        std::cout << "arg1 was provided\n";
    } else {
        std::cout << "arg1 was NOT provided\n";
    }

    if (p.has(arg2)) {
        std::cout << "arg2 was provided\n";
    } else {
        std::cout << "arg2 was NOT provided\n";
    }
}
```

If a named argument is not provided, ``parser``'s call operator will return
a ``const`` reference to a global object of the special type ``not_provided_t``:

```c++
#include <cassert>
#include <concepts>

template <typename ... Args>
void missing_arg(Args && ... args)
{
    parser p{args...};

    // Look ma, structured bindings!
    auto [a, b] = p(arg1, arg2);

    if (!p.has(arg1)) {
        assert(std::same_as<decltype(a), const not_provided_t &>);
    }
}
```

## How do I use igor in generic code?

igor is ``if constexpr`` friendly, thus you can easily do compile-time dispatching based on the
presence of a specific named argument and/or its type:

```c++
#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>

template <typename ... Args>
void arg_dispatch(Args && ... args)
{
    parser p{args...};

    auto [a, b] = p(arg1, arg2);

    if constexpr (p.has(arg1)) {
        if constexpr (std::same_as<std::remove_cvref_t<decltype(a)>, int>) {
            std::cout << "arg1 is an int. arg1 + 2 is: " << (a + 2) << ".\n";
        } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(a)>, std::string>) {
            std::cout << "arg1 is a string. arg1 has a size of " << a.size() << ".\n";
        } else {
            std::cout << "arg1 is some type other than int or string.\n";
        }
    }
}
```

## Does it work with move-only types?

Yes. A ``parser`` perfectly forwards references to the values associated to named arguments, and thus you
are free to use ``std::move()`` and ``std::forward()`` as you like. For instance:

```c++
#include <utility>

struct move_only
{
    move_only() = default;
    move_only(move_only &&) = default;

    // Remove the copy constructor.
    move_only(const move_only &) = delete;
};

template <typename ... Args>
void move_argument(Args && ... args)
{
    parser p{args...};

    move_only inner{std::move(p(arg1))};
}

int main()
{
    move_only mo;
    move_argument(arg1 = std::move(mo));
    move_argument(arg1 = move_only{});
}

```

An example of perfect forwarding:

```c++
#include <utility>

template <typename T, typename U>
void inner_function(T &&x, U &&y)
{
    // Do something with x and y...
}

template <typename ... Args>
void outer_function(Args && ... args)
{
    parser p{args...};

    auto [a, b] = p(arg1, arg2);

    inner_function(std::forward<decltype(a)>(a), std::forward<decltype(b)>(b));
}

int main()
{
    outer_function(arg2 = 3, arg1 = 6.7);
}
```

## Does it work with ``std::initializer_list``?

Yes it does:

```c++
#include <initializer_list>

template <typename ... Args>
void init_list(Args && ... args)
{
    parser p{args...};

    auto [a, b] = p(arg1, arg2);

    std::cout << "First init list: ";
    for (auto v: a) {
        std::cout << v << " ";
    }

    std::cout << "\nSecond init list: ";
    for (auto v: b) {
        std::cout << v << " ";
    }
    std::cout << '\n';
}

int main()
{
    init_list(arg1 = {1, 2, 3}, arg2 = {"a", "b", "c"});
}
```

## How does the assembly look?

Pretty good. One of igor's design goals is to make the handling of named arguments as efficient
as possible. Ideally, functions with and without named arguments should compile to identical binary code.
You can see that, at least in a couple of simple examples, this is indeed the case: https://godbolt.org/z/c3r9xa
(e.g., look for the ``add_int()`` and ``add_int_igor()`` functions in the generated assembly).

## I am convinced. How do I get it?

If you are in a hurry, just download ``igor.hpp`` and chuck it somewhere. igor depends only on the standard library and it
is contained in a single header file.

Otherwise, you can install it via the usual CMake spells.

## Why "igor"?

Igor is the trusty manservant of Dr. Frankenstein (pronounced *Fronkonsteen*) in the 1974 comedy classic "Young Frankenstein".

![It's pronounced eye-gor](https://github.com/bluescarni/igor/raw/master/igor.gif)

Or perhaps it is a reference to Igor Stravinsky.
