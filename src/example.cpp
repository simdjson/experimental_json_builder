#include "json_utils.hpp"
#include <string>
#include <vector>

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

int main() {
  X s1 = {.a = '1',
          .b = 10,
          .c = 0,
          .d = "test string\n\r\"",
          .e = {1, 2, 3},
          .f = {"ab", "cd", "fg"},
          .y = {.g = 100, .h = "test string\n\r\"", .i = {1, 2, 3}}};

  std::cout << experimental_json_builder::to_json_string(s1) << std::endl;
  return 0;
}
