#include <iostream>
#include <string>

#include "igor.hpp"

using namespace igor;

// Use code starts here
// =================================

// Define argument 'biz'.
struct biz_tag {};
inline constexpr auto biz = named_argument<biz_tag>{};

// Define argument 'baz'.
struct baz_tag {};
inline constexpr auto baz = named_argument<baz_tag>{};

// Define argument 'buz'.
struct buz_tag {};
inline constexpr auto buz = named_argument<buz_tag>{};

struct move_only {
  move_only(move_only &&) = default;
  move_only(const move_only &) = delete;
};

// Generic function that can take biz and baz as named arguments.
template <typename... Args> void barz(Args &&... args) {
  const parser p(args...);
  decltype(auto) buz_arg = p(buz);
  if constexpr (is_provided<decltype(buz_arg)>) {
    std::cout << "buz provided\n";
    static_assert(std::is_same_v<move_only &&, decltype(buz_arg)>);
  } else {
    std::cout << "buz not provided\n";
  }
}

template <typename... Args> auto multer(Args &&... args) {
  parser p(args...);
  auto [a, b] = p(baz, biz);
  return a * b;
}

int multer_int(int n, int m) { return multer(baz = n, biz = m); }

int multer_int2(int n, int m) { return n * m; }

int main() { barz(baz = 5, biz = "hello", buz = move_only{}); }
