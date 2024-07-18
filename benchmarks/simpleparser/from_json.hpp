#pragma once
#include "simdjson/json_builder/json_builder.h"
#include <string>
#include <vector>
#include <experimental/meta> // Reflection header
#include "json_parser.hpp"

namespace simpleparser {
namespace json_builder {

using JsonValue = json_parser::JsonValue;
using JsonValueType = json_parser::JsonValueType;


template <class T>
void from_json(const JsonValue& j, T& t) requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>) {
    if (j.type != JsonValueType::Number) {
        throw std::runtime_error("Expected number type for arithmetic value");
    }
    t = static_cast<T>(j.number_value);
}

// Explicit template specialization for bool
void from_json(const JsonValue& j, bool& t) {
    if (j.type != JsonValueType::Boolean) {
        throw std::runtime_error("Expected boolean type for bool value");
    }
    t = j.bool_value;
}

void from_json(const JsonValue& j, std::string& t) {
    if (j.type != JsonValueType::String) {
        throw std::runtime_error("Expected string type for string value");
    }
    t = j.string_value;
}

template <class T>
void from_json(const JsonValue& j, std::vector<T>& t) requires simdjson::json_builder::ContainerButNotString<std::vector<T>>;

template <class T>
void from_json(const JsonValue& j, T& obj) requires(simdjson::json_builder::UserDefinedType<T> && !simdjson::json_builder::ContainerButNotString<T> && !std::is_same_v<T, std::string> && !std::is_same_v<T, std::string_view> && !std::is_same_v<T, const char *>) {
    if (j.type != JsonValueType::Object) {
        throw std::runtime_error("Expected object type for user-defined type value");
    }

    // Using reflection to iterate over data members
    [:simdjson::json_builder::expand(std::meta::nonstatic_data_members_of(^T)):] >> [&]<auto dm> {
        constexpr auto name = std::meta::identifier_of(dm);
        auto it = j.object_value.find(std::string(reinterpret_cast<const char*>(name.data()), name.size()));
        if (it != j.object_value.end()) {
            from_json(it->second, obj.[:dm:]);
        }
    };
}

template <class T>
void from_json(const JsonValue& j, std::vector<T>& t) requires simdjson::json_builder::ContainerButNotString<std::vector<T>> {
    if (j.type != JsonValueType::Array) {
        throw std::runtime_error("Expected array type for container value");
    }
    t.clear();
    for (auto& element : j.array_value) {
        T value;
        from_json(element, value);
        t.push_back(value);
    }
}


} // namespace json_builder
} // namespace simpleparser