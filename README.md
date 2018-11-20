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
``parser``'s call operator.

![It's pronounced eye-gor](https://github.com/bluescarni/igor/raw/master/igor.gif)
