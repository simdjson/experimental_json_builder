#include "json_utils.hpp"
#include <string>
#include <vector>

struct X {
  char a;
  int b;
  int c;
  std::string d;
  std::vector<int> e;
  std::vector<std::string> f;
};

int main() {
  X s1 = {.a = '1',
          .b = 10,
          .c = 0,
          .d = "test string\n\r\"",
          .e = {1, 2, 3},
          .f = {"ab", "cd", "fg"}};

  std::cout << experimental_json_builder::to_json_string(s1) << std::endl;
  return 0;
}
