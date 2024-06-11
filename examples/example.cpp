#include "../src/experimental_json_builder.hpp"
#include "../src/string_builder.hpp"
#include "../src/universal_formatter_misc.hpp"
#include <string>
#include <vector>
#include <format>
#include <print>
#include <iostream>

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

// template <> struct std::formatter<Y> : universal_formatter { }; -> not needed when dealing with structs
// todo -> test the limitations of both methods with private/protected members, as well as with inheritance
template <> struct std::formatter<X> : experimental_json_builder::universal_formatter { };

int main() {
  X s1 = {.a = '1',
          .b = 10,
          .c = 0,
          .d = "test string\n\r\"",
          .e = {1, 2, 3},
          .f = {"ab", "cd", "fg"},
          .y = {.g = 100, .h = "test string\n\r\"", .i = {1, 2, 3}}};

  experimental_json_builder::StringBuilder sb;
  experimental_json_builder::fast_to_json_string(sb, s1);
  std::cout << sb.c_str() << std::endl;
  
  // std::cout << experimental_json_builder::fast_to_json_string(sb,s1) << std::endl << std::endl;

  // std::println("{}", s1);

  return 0;
}