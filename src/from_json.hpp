#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <charconv>
#include <experimental/meta> // Reflection header
#include "experimental_json_builder.hpp" // We might want to extract the concepts + metaprogramming to a separate header and include here and in the builder
#include "json_parser.hpp"

namespace experimental_json_builder {

using JsonValue = json_parser::JsonValue;
using JsonValueType = json_parser::JsonValueType;

template <typename T>
void from_json(const JsonValue& j, T& t);

template <typename T>
void from_json(const JsonValue& j, T& t) requires std::is_arithmetic_v<T> {
    if (j.type != JsonValueType::Number) {
        throw std::runtime_error("Expected number type for arithmetic value");
    }
    t = static_cast<T>(j.number_value);
}

void from_json(const JsonValue& j, std::string& t) {
    if (j.type != JsonValueType::String) {
        throw std::runtime_error("Expected string type for string value");
    }
    t = j.string_value;
}

template <typename T>
void from_json(const JsonValue& j, std::vector<T>& vec) {
    if (j.type != JsonValueType::Array) {
        throw std::runtime_error("Expected array type for vector value");
    }
    vec.clear();
    for (const auto& element : j.array_value) {
        T value;
        from_json(element, value);
        vec.push_back(value);
    }
}

template <typename T>
void from_json(const JsonValue& j, T& obj) requires(UserDefinedType<T>) {
    if (j.type != JsonValueType::Object) {
        throw std::runtime_error("Expected object type for user-defined type value");
    }
    [:expand(std::meta::nonstatic_data_members_of(^T)):] >> [&]<auto dm> {
        constexpr auto name = std::meta::name_of(dm);
        auto it = j.object_value.find(name.data());
        if (it != j.object_value.end()) {
            from_json(it->second, obj.[:dm:]);
        }
    };
}

} // namespace experimental_json_builder
