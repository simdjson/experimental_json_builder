# Experimental JSON Builder based on reflection

This project seeks to develop a JSON builder library using the latest features of C++. 
That is, we see to build fast and convenient code to map your data structure to JSON strings.

Ultimately, this work might cover both serialization and deserialization.

We've seen different proposals for C++ reflection, for now we will focus on the latest paper (https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2996r1.html#converting-a-struct-to-a-tuple) that is targeting C++26.

Some other references regarding other implementations of reflection in C++:
- https://github.com/matus-chochlik/mirror/blob/develop/example/mirror/print_struct.cpp (depends on llvm-reflection branch + mirror library)
- https://godbolt.org/z/eo3zrro4v (using boost/ext static reflection, latest version is not currently compiling)