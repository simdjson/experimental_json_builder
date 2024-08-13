#include "simdjson.h"
#include "simdjson/json_builder/json_builder.h"
#include <format>

struct kid {
  int age;
  std::string name;
  std::vector<std::string> toys;
};

namespace simdjson {
// Helper code because simdjson does not use reflect for now
template <typename T>
  requires(simdjson::json_builder::UserDefinedType<T>)
simdjson_result<T> tag_invoke(deserialize_tag, std::type_identity<T>,
                              auto &val) {
  ondemand::object obj;
  auto error = val.get_object().get(obj);
  if (error) {
    return error;
  }
  T t;
  for (auto keyval : obj) {
    std::string_view key;
    SIMDJSON_TRY(keyval.escaped_key().get(key));
    [:json_builder::expand(std::meta::nonstatic_data_members_of(^T)
                           ):] >> [&]<auto mem> {
      if (key == std::string_view(std::meta::identifier_of(mem))) {
        error = keyval.value().get(t.[:mem:]);
      }
    };
    if (error) {
      return error;
    }
  }
  return t;
}

} // namespace simdjson

/**

I am 12 years old
I have a car
I have a ball
My name is John
My JSON is {"age":12,"name":"John","toys":["car","ball"]}

*/
void demo() {
  simdjson::padded_string json_str =
      R"({"age": 12, "name": "John", "toys": ["car", "ball"]})"_padded;
  simdjson::ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  kid k = doc.get<kid>();
  std::print("I am {} years old\n", k.age);
  for (const auto &toy : k.toys) {
    std::print("I have a {}\n", toy);
  }
  std::print("My name is {}\n", k.name);

  std::print("My JSON is {}\n", simdjson::json_builder::to_json_string(k));
}
int main() {
  demo();
  return EXIT_SUCCESS;
}