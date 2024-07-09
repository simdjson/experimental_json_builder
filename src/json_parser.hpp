#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <charconv>
#include <experimental/meta>

namespace json_parser {

enum class JsonValueType { Null, Boolean, Number, String, Array, Object };

struct JsonValue {
    JsonValueType type;
    std::string string_value;
    long double number_value; // This is not guaranteed to be able to represent any uint64_t value with full precision, this is an experimental parser.
    bool bool_value;
    std::vector<JsonValue> array_value;
    std::unordered_map<std::string, JsonValue> object_value;
};

class JsonParser {
public:
    JsonParser(const std::string &input) : input(input), pos(0) {}

    JsonValue parse() { return parse_value(); }

private:
    const std::string &input;
    size_t pos;

    void skip_whitespace() {
        while (pos < input.size() && isspace(input[pos])) {
            ++pos;
        }
    }

    JsonValue parse_value() {
        skip_whitespace();
        if (pos >= input.size()) {
            throw std::runtime_error("Unexpected end of input");
        }

        if (input[pos] == '{') {
            return parse_object();
        } else if (input[pos] == '[') {
            return parse_array();
        } else if (input[pos] == '"') {
            return parse_string();
        } else if (isdigit(input[pos]) || input[pos] == '-') {
            return parse_number();
        } else if (input.compare(pos, 4, "true") == 0) {
            pos += 4;
            return JsonValue{JsonValueType::Boolean, "", 0.0, true, {}, {}};
        } else if (input.compare(pos, 5, "false") == 0) {
            pos += 5;
            return JsonValue{JsonValueType::Boolean, "", 0.0, false, {}, {}};
        } else if (input.compare(pos, 4, "null") == 0) {
            pos += 4;
            return JsonValue{JsonValueType::Null, "", 0.0, false, {}, {}};
        } else {
            throw std::runtime_error("Invalid JSON value");
        }
    }

    JsonValue parse_object() {
        JsonValue result{JsonValueType::Object, "", 0.0, false, {}, {}};
        ++pos; // Skip '{'
        skip_whitespace();
        if (input[pos] == '}') {
            ++pos; // Empty object
            return result;
        }

        while (true) {
            skip_whitespace();
            auto key = parse_string().string_value;
            skip_whitespace();
            if (input[pos] != ':') {
                throw std::runtime_error("Expected ':' in object");
            }
            ++pos; // Skip ':'
            skip_whitespace();
            auto value = parse_value();
            result.object_value.emplace(std::move(key), std::move(value));

            skip_whitespace();
            if (input[pos] == '}') {
                ++pos;
                break;
            } else if (input[pos] != ',') {
                throw std::runtime_error("Expected ',' or '}' in object");
            }
            ++pos; // Skip ','
        }

        return result;
    }

    JsonValue parse_array() {
        JsonValue result{JsonValueType::Array, "", 0.0, false, {}, {}};
        ++pos; // Skip '['
        skip_whitespace();
        if (input[pos] == ']') {
            ++pos; // Empty array
            return result;
        }

        while (true) {
            skip_whitespace();
            result.array_value.push_back(parse_value());
            skip_whitespace();
            if (input[pos] == ']') {
                ++pos;
                break;
            } else if (input[pos] != ',') {
                throw std::runtime_error("Expected ',' or ']' in array");
            }
            ++pos; // Skip ','
        }

        return result;
    }

    JsonValue parse_string() {
        ++pos; // Skip '"'
        std::string result;
        while (pos < input.size() && input[pos] != '"') {
            if (input[pos] == '\\') {
                ++pos;
                if (pos >= input.size()) {
                    throw std::runtime_error("Unexpected end of input in string escape");
                }
                switch (input[pos]) {
                case '"':
                    result += '"';
                    break;
                case '\\':
                    result += '\\';
                    break;
                case '/':
                    result += '/';
                    break;
                case 'b':
                    result += '\b';
                    break;
                case 'f':
                    result += '\f';
                    break;
                case 'n':
                    result += '\n';
                    break;
                case 'r':
                    result += '\r';
                    break;
                case 't':
                    result += '\t';
                    break;
                case 'u':
                    throw std::runtime_error("Unicode escape sequences not supported");
                default:
                    throw std::runtime_error("Invalid escape character");
                }
            } else {
                result += input[pos];
            }
            ++pos;
        }
        if (pos >= input.size()) {
            throw std::runtime_error("Unexpected end of input in string");
        }
        ++pos; // Skip closing '"'
        return JsonValue{JsonValueType::String, result, 0.0, false, {}, {}};
    }


    JsonValue parse_number() {
        size_t start_pos = pos;
        if (input[pos] == '-') {
            ++pos;
        }
        while (pos < input.size() && isdigit(input[pos])) {
            ++pos;
        }
        if (pos < input.size() && input[pos] == '.') {
            ++pos;
            while (pos < input.size() && isdigit(input[pos])) {
                ++pos;
            }
        }
        if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E')) {
            ++pos;
            if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
                ++pos;
            }
            while (pos < input.size() && isdigit(input[pos])) {
                ++pos;
            }
        }

        // Extract the substring representing the number
        std::string number_str = input.substr(start_pos, pos - start_pos);

        // Convert the substring to a double using stringstream
        std::stringstream ss(number_str);
        long double number_value;
        ss >> number_value;

        if (ss.fail()) {
            throw std::runtime_error("Failed to parse number: " + number_str);
        }

        return JsonValue{JsonValueType::Number, "", number_value, false, {}, {}};
    }

};

} // namespace json_parser
