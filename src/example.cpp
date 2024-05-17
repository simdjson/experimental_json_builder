#include "json_utils.hpp"
#include "simpler_reflection.hpp"
#include <string>
#include <vector>
#include <format>
#include <print>

struct Y {
  int g;
  std::string h;
  std::vector<int> i; 
};

struct X {
  char a;
  int b;
  int c;
  std::string d;
  std::vector<int> e;
  std::vector<std::string> f;
  Y y;
};

template <> struct std::formatter<Y> : universal_formatter { };
template <> struct std::formatter<X> : universal_formatter { };

int main() {
  X s1 = {.a = '1',
          .b = 10,
          .c = 0,
          .d = "test string\n\r\"",
          .e = {1, 2, 3},
          .f = {"ab", "cd", "fg"},
          .y = {.g = 100, .h = "test string\n\r\"", .i = {1, 2, 3}}};

  std::cout << experimental_json_builder::to_json_string(s1) << std::endl << std::endl;

  std::println("{}", s1);

  return 0;
}
