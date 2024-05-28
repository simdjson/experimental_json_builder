#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <cstring>
#include <format>

namespace experimental_json_builder {

class StringBuilder {
public:
    StringBuilder(size_t initial_capacity = 2629189250)
        : buffer(initial_capacity), position(0) {}

    void append(char c) {
        /* Ignoring size-checks for now
        if (position >= buffer.size()) {
            buffer.resize(buffer.size() * 2);
        }*/
        buffer[position++] = c;
    }

    /*
    void append(const std::string& str) {
        append(str.data(), str.size());
    }*/

    
    void append(std::string_view str) {
        append(str.data(), str.size());
    }

    void append(const char* str, size_t len) {
        /*if (position + len >= buffer.size()) {
            buffer.resize((position + len) * 2);
        }*/

        std::memcpy(buffer.data() + position, str, len);
        position += len;
    }

    /* Not interested in format right now, since it didn't work in first attempts
    template<typename... Args>
    void append_format(const std::string& format, Args&&... args) {
        std::string formatted = std::vformat(format, std::make_format_args(std::forward<Args>(args)...));
        append(formatted);
    }*/

    std::string str() const {
        return std::string(buffer.data(), position);
    }

    const char* c_str() {
        /* Ignore size-checks for now
        if (position >= buffer.size()) {
            buffer.resize(buffer.size() + 1);
        }*/
        buffer[position] = '\0'; // Ensure null-termination
        return buffer.data();
    }

    size_t size() const {
        return position;
    }

private:
    std::vector<char> buffer;
    size_t position;
};

} // namespace experimental_json_builder