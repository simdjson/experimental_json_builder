#include "simdjson.h"
#include "simdjson/json_builder/json_builder.h"
#include "simdjson/json_builder/string_builder.h"
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <vector>
using namespace simdjson;

// todo: use reflection to generate
template <>
simdjson_inline simdjson_result<std::vector<int>>
simdjson::ondemand::value::get() noexcept {
  ondemand::array array;
  auto error = get_array().get(array);
  if (error) {
    return error;
  }
  std::vector<int> vec;
  for (auto v : array) {
    int64_t val;
    error = v.get_int64().get(val);
    if (error) {
      return error;
    }
    vec.push_back(int(val));
  }
  return vec;
}

// todo: use reflection to generate
template <>
simdjson_inline simdjson_result<std::vector<std::string>>
simdjson::ondemand::value::get() noexcept {
  ondemand::array array;
  auto error = get_array().get(array);
  if (error) {
    return error;
  }
  std::vector<std::string> vec;
  for (auto v : array) {
    std::string t;
    error = v.get_string(t);
    if (error) {
      return error;
    }
    vec.push_back(t);
  }
  return vec;
}

struct MyStruct {
  int id;
  std::string name;
  std::vector<int> values;
};

// todo: use reflection to generate
template <>
simdjson_inline simdjson_result<MyStruct>
simdjson::ondemand::value::get() noexcept {
  ondemand::object obj;
  auto error = get_object().get(obj);
  if (error) {
    return error;
  }
  MyStruct s;
  for (auto field : obj) {
    raw_json_string key;
    error = field.key().get(key);
    if (error) {
      return error;
    }
    if (key == "id") {
      int64_t id;
      error = field.value().get_int64().get(id);
      if (error) {
        return error;
      }
      s.id = int(id);
    } else if (key == "name") {
      error = field.value().get_string(s.name);
      if (error) {
        return error;
      }
    } else if (key == "values") {
      error = field.value().get<std::vector<int>>().get(s.values);
      if (error) {
        return error;
      }
    }
  }
  return s;
}

// todo: use reflection to generate
template <>
simdjson_inline simdjson_result<MyStruct>
simdjson::ondemand::document::get() & noexcept {
  ondemand::object obj;
  auto error = get_object().get(obj);
  if (error) {
    return error;
  }
  MyStruct s;
  for (auto field : obj) {
    raw_json_string key;
    error = field.key().get(key);
    if (error) {
      return error;
    }
    if (key == "id") {
      int64_t id;
      error = field.value().get_int64().get(id);
      if (error) {
        return error;
      }
      s.id = int(id);
    } else if (key == "name") {
      error = field.value().get_string(s.name);
      if (error) {
        return error;
      }
    } else if (key == "values") {
      error = field.value().get<std::vector<int>>().get(s.values);
      if (error) {
        return error;
      }
    }
  }
  return s;
}

struct Y {
  int g;
  std::string h;
  std::vector<int> i;
};

// todo: use reflection to generate
template <>
simdjson_inline simdjson_result<Y> simdjson::ondemand::value::get() noexcept {
  ondemand::object obj;
  auto error = get_object().get(obj);
  Y y;
  for (auto field : obj) {
    raw_json_string key;
    error = field.key().get(key);
    if (error) {
      return error;
    }
    if (key == "g") {
      int64_t g;
      error = field.value().get_int64().get(g);
      if (error) {
        return error;
      }
      y.g = int(g);
    } else if (key == "h") {
      error = field.value().get_string(y.h);
      if (error) {
        return error;
      }
    } else if (key == "i") {
      error = field.value().get<std::vector<int>>().get(y.i);
      if (error) {
        return error;
      }
    }
  }
  return y;
}

struct X {
  char a;
  int b;
  int c;
  std::string d;
  std::vector<int> e;
  std::vector<std::string> f;
  Y y;
};

// todo: use reflection to generate
template <>
simdjson_inline simdjson_result<X> simdjson::ondemand::value::get() noexcept {
  ondemand::object obj;
  auto error = get_object().get(obj);
  X x;
  for (auto field : obj) {
    raw_json_string key;
    error = field.key().get(key);
    if (error) {
      return error;
    }
    if (key == "a") {
      int64_t a;
      error = field.value().get_int64().get(a);
      if (error) {
        return error;
      }
      x.a = char(a);
    } else if (key == "b") {
      int64_t b;
      error = field.value().get_int64().get(b);
      if (error) {
        return error;
      }
      x.b = int(b);
    } else if (key == "c") {
      int64_t c;
      error = field.value().get_int64().get(c);
      if (error) {
        return error;
      }
      x.c = int(c);
    } else if (key == "d") {
      error = field.value().get_string(x.d);
      if (error) {
        return error;
      }
    } else if (key == "e") {
      error = field.value().get<std::vector<int>>().get(x.e);
      if (error) {
        return error;
      }
    } else if (key == "f") {
      error = field.value().get<std::vector<std::string>>().get(x.f);
      if (error) {
        return error;
      }
    } else if (key == "y") {
      error = field.value().get<Y>().get(x.y);
      if (error) {
        return error;
      }
    }
  }
  return x;
}

// todo: use reflection to generate
template <>
simdjson_inline simdjson_result<X>
simdjson::ondemand::document::get() & noexcept {
  ondemand::object obj;
  auto error = get_object().get(obj);
  X x;
  for (auto field : obj) {
    raw_json_string key;
    error = field.key().get(key);
    if (error) {
      return error;
    }
    if (key == "a") {
      int64_t a;
      error = field.value().get_int64().get(a);
      if (error) {
        return error;
      }
      x.a = char(a);
    } else if (key == "b") {
      int64_t b;
      error = field.value().get_int64().get(b);
      if (error) {
        return error;
      }
      x.b = int(b);
    } else if (key == "c") {
      int64_t c;
      error = field.value().get_int64().get(c);
      if (error) {
        return error;
      }
      x.c = int(c);
    } else if (key == "d") {
      error = field.value().get_string(x.d);
      if (error) {
        return error;
      }
    } else if (key == "e") {
      error = field.value().get<std::vector<int>>().get(x.e);
      if (error) {
        return error;
      }
    } else if (key == "f") {
      error = field.value().get<std::vector<std::string>>().get(x.f);
      if (error) {
        return error;
      }
    } else if (key == "y") {
      error = field.value().get<Y>().get(x.y);
      if (error) {
        return error;
      }
    }
  }
  return x;
}

int main() {
  std::string json_str = R"({"id": 1, "name": "example", "values": [1, 2, 3]})";
  ondemand::parser parser;
  ondemand::document doc = parser.iterate(json_str);

  MyStruct my_struct(doc);
  simdjson::json_builder::StringBuilder sb;
  simdjson::json_builder::fast_to_json_string(sb, my_struct);
  std::cout << sb.c_str() << std::endl;

  std::string json_str_nested =
      R"({"a":1,"b":10,"c":0,"d":"test string\n\r\"","e":[1,2,3],"f":["ab","cd","fg"],"y":{"g":100,"h":"test string\n\r\"","i":[1,2,3]}})";
  doc = parser.iterate(json_str_nested);

  X s1 = X(doc);
  simdjson::json_builder::StringBuilder sb2;
  simdjson::json_builder::fast_to_json_string(sb2, s1);
  std::cout << sb2.c_str() << std::endl;

  return 0;
}
