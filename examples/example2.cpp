#include "../src/experimental_json_builder.hpp"
#include "../src/from_json.hpp"
#include "../src/string_builder.hpp"
#include <string>
#include <vector>
#include <format>
#include <print>
#include <iostream>

struct MyStruct {
    int id;
    std::string name;
    std::vector<int> values;
};

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
    std::string json_str = R"({"id": 1, "name": "example", "values": [1, 2, 3]})";
    json_parser::JsonParser parser(json_str);
    auto json_value = parser.parse();

    MyStruct my_struct;
    experimental_json_builder::from_json(json_value, my_struct);

    experimental_json_builder::StringBuilder sb;
    experimental_json_builder::fast_to_json_string(sb, my_struct);
    std::cout << sb.c_str() << std::endl;

    std::string json_str_nested = R"({"a":1,"b":10,"c":0,"d":"test string\n\r\"","e":[1,2,3],"f":["ab","cd","fg"],"y":{"g":100,"h":"test string\n\r\"","i":[1,2,3]}})";
    json_parser::JsonParser parser2(json_str_nested);
    auto json_value2 = parser2.parse();

    X s1;
    experimental_json_builder::from_json(json_value2, s1);
    experimental_json_builder::StringBuilder sb2;
    experimental_json_builder::fast_to_json_string(sb2, s1);
    std::cout << sb2.c_str() << std::endl;
    
    return 0;
}
