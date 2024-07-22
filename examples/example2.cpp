#include "simdjson.h"
#include "simdjson/json_builder/json_builder.h"
#include "simdjson/json_builder/string_builder.h"
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <type_traits>
#include <vector>
using namespace simdjson;

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

template <typename T>
concept PushableContainer =
    requires(T a, typename T::value_type val) { a.push_back(val); } &&
    !std::is_same_v<T, std::string> && !std::is_same_v<T, std::string_view> &&
    !std::is_same_v<T, const char *>;

template <class T>
simdjson_inline simdjson_result<T> simdjson::ondemand::value::get() noexcept {
  ondemand::array array;
  auto error = get_array().get(array);
  if (error) {
    return error;
  }
  T container;
  for (auto v : array) {
    typename T::value_type val;
    if constexpr (std::is_same_v<typename T::value_type, double>) {
      error = v.get_double().get(val);
    } else if constexpr (std::is_same_v<typename T::value_type, bool>) {
      error = v.get_bool().get(val);
    } else if constexpr (std::is_same_v<typename T::value_type, uint64_t>) {
      error = v.get_uint64().get(val);
    } else if constexpr (std::is_same_v<typename T::value_type, int64_t>) {
      error = v.get_int64().get(val);
    } else if constexpr (std::is_same_v<typename T::value_type, int>) {
      int64_t temp_val;
      error = v.get_int64().get(temp_val);
      if (!error) {
        val = static_cast<int>(temp_val);
      }
    } else if constexpr (std::is_same_v<typename T::value_type, std::string>) {
      std::string str_val;
      error = v.get_string(str_val);
      if (!error) {
        val = std::move(str_val);
      }
    } else if constexpr (std::is_same_v<typename T::value_type,
                                        std::string_view>) {
      std::string_view str_view_val;
      error = v.get_string(str_view_val, false);
      if (!error) {
        val = str_view_val;
      }
    } else if constexpr (std::is_same_v<typename T::value_type,
                                        simdjson::ondemand::raw_json_string>) {
      simdjson::ondemand::raw_json_string raw_val;
      error = v.get_raw_json_string().get(raw_val);
      if (!error) {
        val = raw_val;
      }
    } else {
      static_assert(!sizeof(T), "Unsupported value type in the container.");
    }

    if (error) {
      return error;
    }
    container.push_back(std::move(val));
  }
  return container;
}

template <typename T>
simdjson_inline simdjson_result<T>
simdjson::ondemand::document::get() & noexcept {
  if constexpr (PushableContainer<T>) {
    ondemand::array array;
    auto error = get_array().get(array);
    if (error) {
      return error;
    }
    T container;
    for (auto v : array) {
      typename T::value_type val;
      if constexpr (std::is_same_v<typename T::value_type, double>) {
        error = v.get_double().get(val);
      } else if constexpr (std::is_same_v<typename T::value_type, bool>) {
        error = v.get_bool().get(val);
      } else if constexpr (std::is_same_v<typename T::value_type, uint64_t>) {
        error = v.get_uint64().get(val);
      } else if constexpr (std::is_same_v<typename T::value_type, int64_t>) {
        error = v.get_int64().get(val);
      } else if constexpr (std::is_same_v<typename T::value_type, int>) {
        int64_t temp_val;
        error = v.get_int64().get(temp_val);
        if (!error) {
          val = static_cast<int>(temp_val);
        }
      } else if constexpr (std::is_same_v<typename T::value_type, std::string>) {
        std::string str_val;
        error = v.get_string(str_val);
        if (!error) {
          val = std::move(str_val);
        }
      } else if constexpr (std::is_same_v<typename T::value_type,
                                          std::string_view>) {
        std::string_view str_view_val;
        error = v.get_string(str_view_val, false);
        if (!error) {
          val = str_view_val;
        }
      } else if constexpr (std::is_same_v<typename T::value_type,
                                          simdjson::ondemand::raw_json_string>) {
        simdjson::ondemand::raw_json_string raw_val;
        error = v.get_raw_json_string().get(raw_val);
        if (!error) {
          val = raw_val;
        }
      } else {
        static_assert(!sizeof(T), "Unsupported value type in the container.");
      }

      if (error) {
        return error;
      }
      container.push_back(std::move(val));
    }
    return container;
  } else {
    static_assert(!sizeof(T), "Unsupported container type.");
  }
}

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
      // error = field.value().get<Y>().get(x.y);
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
      // error = field.value().get<Y>().get(x.y);
      if (error) {
        return error;
      }
    }
  }
  return x;
}

void demo() {
  std::vector<std::string> vec;
  std::string json_str = R"(["a", "b", "c"])";
  ondemand::parser parser;
  ondemand::document doc = parser.iterate(json_str);
  std::vector<std::string> result = doc.get<std::vector<std::string>>();
  for (auto x : result) {
    std::cout << x << std::endl;
  }
}

int main() {
  demo();

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
