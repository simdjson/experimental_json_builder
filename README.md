# Experimental JSON Builder based on reflection

This project seeks to develop a JSON builder library using the latest features of C++. 
That is, we see to build fast and convenient code to map your data structure to JSON strings.

Ultimately, this work might cover both serialization and deserialization.

We've seen different proposals forhttps://github.com/nlohmann/json?tab=readme-ov-file#simplify-your-life-with-macros C++ reflection, for now we will focus on the latest paper (https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2996r1.html#converting-a-struct-to-a-tuple) that is targeting C++26.

Some other references regarding other implementations of reflection in C++:
- https://github.com/matus-chochlik/mirror/blob/develop/example/mirror/print_struct.cpp (depends on llvm-reflection branch + mirror library)
- https://godbolt.org/z/eo3zrro4v (using boost/ext static reflection, latest version is not currently compiling)

## Why we are interested in (de)serializing using reflection?
If you take a look at one of the reference implementations of (de)serialization
using modern C++ (https://github.com/nlohmann/json), you will notice in the
examples that to deal with custom structs/classes, you need to use specify
those 2 methods:

```c++
using json = nlohmann::json;

namespace ns {
    void to_json(json& j, const person& p) {
        j = json{{"name", p.name}, {"address", p.address}, {"age", p.age}};
    }

    void from_json(const json& j, person& p) {
        j.at("name").get_to(p.name);
        j.at("address").get_to(p.address);
        j.at("age").get_to(p.age);
    }
} // namespace ns
```

Or use one of the Macros described
[here](https://github.com/nlohmann/json?tab=readme-ov-file#simplify-your-life-with-macros) in your class/struct.

By leveraging reflection, we can support for something as simple as:

```c++
int main() {
    X s1 = {.a = '1', .b = 10, .c = 0, .d = "test string\n\r\"", .e = {1, 2, 3}, .f = {"ab", "cd", "fg"}};

    std::cout << to_json_string(s1) << std::endl;
    return 0;
}
```

With ~200 lines of code (as you can see in the [toy_builder.cpp](builder/toy_builder.cpp)).

## Current status
For now, the only version of the compiler that supports our implementation is the EDG experimental reflection branch that is current available in the [Compiler Explorer](https://godbolt.org).

For now, we will be working on the deserializing function inside the compiler explorer, since no implementation of the C++ 26 reflection paper is publicly available to the public.

We are also interested in measuring this against current approaches (performance + compilation time).