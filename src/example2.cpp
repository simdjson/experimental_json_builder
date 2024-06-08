#include "experimental_json_builder.hpp"
#include "from_json.hpp"
#include "string_builder.hpp"
#include "universal_formatter_misc.hpp"
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

int main() {
    std::string json_str = R"({"id": 1, "name": "example", "values": [1, 2, 3]})";
    json_parser::JsonParser parser(json_str);
    auto json_value = parser.parse();

    MyStruct my_struct;
    experimental_json_builder::from_json(json_value, my_struct);

    experimental_json_builder::StringBuilder sb;
    experimental_json_builder::fast_to_json_string(sb, my_struct);
    std::cout << sb.c_str() << std::endl;

    return 0;
}
