#include "simdjson/json_builder/json_builder.h"
#include "simdjson/json_builder/string_builder.h"
#include <cstdlib>
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <vector>

struct Z {
  int x;
};

struct Y {
  int g;
  std::string h;
  std::vector<int> i;
  Z z;
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

int main() {
  X s1 = {.a = '1',
          .b = 10,
          .c = 0,
          .d = "test string\n\r\"",
          .e = {1, 2, 3},
          .f = {"ab", "cd", "fg"},
          .y = {.g = 100,
                .h = "test string\n\r\"",
                .i = {1, 2, 3},
                .z = {.x = 1000}}};

  simdjson::json_builder::string_builder sb;
  simdjson::json_builder::fast_to_json_string(sb, s1);
  std::cout << sb.c_str() << std::endl;
  return EXIT_SUCCESS;
}