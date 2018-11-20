# igor

igor (pronounced *eye-gor*) is a small, unpretentious, self-contained and header-only C++17
library implementing named function arguments (AKA, keyword arguments, named parameters, etc.).
A minimal example:

```c++
#include <iostream>
#include <string>

#include <igor/igor.hpp>

using namespace igor;

// Create a named argument called "arg1".
struct arg1_tag {};
inline constexpr auto arg1 = named_argument<arg1_tag>{};

// You can also use a macro (ew, gross!) for brevity.
IGOR_MAKE_NAMED_ARGUMENT(arg2);

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

    std::cout << "The ultimate answer is: " << adder(arg2 = 20, arg1 = 22) << '\n';

    std::cout << adder(arg1 = "hello, "s, arg2 = "world") << '\n';

    return 0;
}
```

## How does it work?

Glad that you asked! A ``parser`` object identifies named arguments upon construction, and stores internally
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

If a named argument is not provided, ``parser``'s call operator on that named argument will return
a ``const`` reference to a global object of the special type ``not_provided_t``:

```c++
#include <type_traits>

template <typename ... Args>
void missing_arg(Args && ... args)
{
    parser p{args...};

    auto [a, b] = p(arg1, arg2);

    if (!p.has(arg1)) {
        assert(std::is_same_v<decltype(a), const not_provided_t &>);
    }
}
```

![It's pronounced eye-gor](https://github.com/bluescarni/igor/raw/master/igor.gif)
