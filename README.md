# Experimental JSON Builder based on reflection

This project seeks to develop a JSON builder library using the latest features of C++. 
That is, we see to build fast and convenient code to map your data structure to JSON strings.

Ultimately, this work might cover both serialization and deserialization.

We've seen different proposals for C++ reflection, for now we will focus on the [latest p2996 paper](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2996r8.html) that is targeting C++26.


Table of Contents
-----------------

* [Why we are interested in (de)serializing using reflection?](#why-we-are-interested-in-deserializing-using-reflection)
* [Current status](#current-status)
* [Instructions for running the toy_builder example locally (using docker)](#instructions-for-running-the-toy_builder-example-locally-using-docker)
* [Instructions for running it natively on macOS](#instructions-for-running-it-natively-on-macos)
   + [1. Install Homebrew](#1-install-homebrew)
   + [2. Install Necessary Packages](#2-install-necessary-packages)
   + [3. Clone the Clang Source Code](#3-clone-the-clang-source-code)
   + [4. Create a Build Directory](#4-create-a-build-directory)
   + [5. Configure the Build](#5-configure-the-build)
   + [6. Build LLVM/Clang](#6-build-llvmclang)
   + [7. Install LLVM/Clang](#7-install-llvmclang)
   + [8. Configure the Dynamic Linker](#8-configure-the-dynamic-linker)
   + [9. Verify the Installation](#9-verify-the-installation)
   + [10. Running the project](#10-running-the-project)
   + [Running tests](#running-tests)
   + [Additional comments regarding running natively on macOS](#additional-comments-regarding-running-natively-on-macos)
* [Profiling](#profiling)
* [References](#references)


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

By leveraging compile-time reflection, we can support for something as simple as:

```cpp
int main() {
    X s1 = {.a = '1', .b = 10, .c = 0, .d = "test string\n\r\"", .e = {1, 2, 3}, .f = {"ab", "cd", "fg"}};

    std::cout << to_json_string(s1) << std::endl;
    return 0;
}
```

We can make the following example work at compile time (see examples/demo.cpp):
```cpp
struct kid {
  int age;
  std::string name;
  std::vector<std::string> toys;
};

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
```



The benchmark results show that our serialization speed can be 20/30x faster than [nlohmann](https://github.com/nlohmann/json).

## Current status
There are 2 versions of compiler that aim to support the C++26 reflection paper ([p2996](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2996r4.html)).

1. [Clang-p2996 llvm branch](https://github.com/bloomberg/clang-p2996/tree/p2996) that open-source and available in the [Compiler Explorer](https://godbolt.org/z/eoEej3E6j).
2. EDG reflection branch that is only publicly available in the [Compiler Explorer](https://godbolt.org).

For now, we will resort to #1, since it is open-source. A docker container is available with this particular branch (thanks to Daniel Lemire) at [docker_programming_station repo](https://github.com/lemire/docker_programming_station/tree/master/clangp2996).

We are excited to hear that this reflection proposal seems to be on-track for C++26. [As per Herb Sutter](https://herbsutter.com/2024/03/22/trip-report-winter-iso-c-standards-meeting-tokyo-japan/).

## Instructions for running the toy_builder example locally (using docker)

We are assuming that you are running Linux or macOS. We recommend that Windows users rely on WSL.

Follow the following two steps:

1. Clone the project

```bash
git clone https://github.com/simdjson/experimental_json_builder.git
```

2. Make sure that you have [docker installed and running](https://docs.docker.com/engine/install/) on your system. Most Linux distributions support docker though some (like RedHat) have the equivalent (Podman). Users of Apple systems may want to [consider OrbStack](https://orbstack.dev). You do not need to familiar with docker, you just need to make sure that you are have it running.

3. Navigate to our repository `experimental_json_builder` and run the following bash script:

```bash
bash run_docker.sh bash
```

This will enter a bash shell with access to the repo directory. Note that this will take some time when running it for the first time, since the specific container image has to be built. 

This step builds and executes a docker container defined by our [Dockerfile](https://github.com/simdjson/experimental_json_builder/blob/main/Dockerfile) which provides the necessary environment.


4. The project has some dependencies and you may want to avoid redownloading them. Thus you should set the cache directory:

```bash
export CPM_SOURCE_CACHE=.cache/CPM
```

5. Configure the build system with cmake:
```bash
cmake -B build
```
This only needs to be done once.

6. Build the code...
```bash
cmake --build build
```


7. Run the tests...
```bash
ctest --test-dir build --output-on-failure
```

8. Run the benchmark.
```bash
./build/benchmarks/src/SerializationBenchmark
```

You can modify the source code with your favorite editor and run again steps 6 (Build the code) and 7 (Run the tests) and 8 (Run the benchmark).

## Instructions for running it natively on macOS

Warning: Installing an experimental compiler as your main compiler on macOS may cause problems.
We recommend using a docker container. If you choose to install an experimental compiler system-wide
under macOS, you are doing so at your own risks.

Here are the step-by-step instructions:

#### 1. Install Homebrew

If you don't already have Homebrew installed, install it by running:

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 2. Install Necessary Packages

Use Homebrew to install the necessary packages:

```
brew install cmake ninja git python3

```

### 3. Clone the Clang Source Code

Clone the Clang source code from the experimental branch:

```
git clone --depth=1 --branch p2996 https://github.com/bloomberg/clang-p2996.git ~/clang-source

```

### 4. Create a Build Directory

Create a build directory and navigate to it:

```
mkdir ~/clang-source/build-llvm
cd ~/clang-source/build-llvm

```

### 5. Configure the Build

Configure the LLVM/Clang build with CMake:

```
ccmake -S ~/clang-source/llvm -B ~/clang-source/build-llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=ON -DLLVM_UNREACHABLE_OPTIMIZE=ON -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" -DCLANG_DEFAULT_CXX_STDLIB=libc++ -DLLVM_ENABLE_PROJECTS=clang -G Ninja

```

### 6. Build LLVM/Clang

Build LLVM/Clang with Ninja:

```
ninja

```

### 7. Install LLVM/Clang

Install LLVM/Clang to a desired prefix (e.g., `/usr/local`):

```
sudo ninja install

```

### 8. Configure the Dynamic Linker

On macOS, you usually don't need to manually configure the dynamic linker paths as you do on Linux. The installation step should have placed the necessary libraries in standard locations. However, if needed, you can set up your environment to point to the correct library paths.

For example, you can add the library path to your `.bashrc` or `.zshrc` file:

```
echo 'export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc

```

### 9. Verify the Installation

Verify that `clang` and `clang++` are correctly installed and working:

```
clang++ --version

```

You should see output indicating that you're using the custom build of Clang/LLVM. (bloomberg experimental)

### 10. Running the project

Now you must run the following comments (that are exactly the same from the docker instructions)

1. Firstly, configure the build system with cmake:
```bash
cmake -B build
```
This only needs to be done once.

2. Build the code...
```bash
cmake --build build
```

3. Run the benchmark.
```bash
./build/benchmarks/src/SerializationBenchmark
```

4. Run the (twitter) benchmark.
```bash
./build/benchmarks/src/SerializationTwitterBenchmark
```

### Running tests

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### Additional comments regarding running natively on macOS
1. There is a linker issue when trying to generate the ExperimentalJsonBuilder target (generated based on the _example.cpp_ file). So for now I have removed it from the makefile.
2. For now we don't have any macOS compatible instructons for profiling. This will be added later on.

## Profiling

You can profile the benchmark like so:

```
cmake --build build &&  perf record ./build/benchmarks/src/SerializationBenchmark && perf report
```

In perf is not available, you can install it like so:

```
sudo apt-get update
sudo apt-get install linux-perf
```

Your password inside the docker container is your username.

## References


Some other references regarding other implementations of reflection in C++:
- https://github.com/matus-chochlik/mirror/blob/develop/example/mirror/print_struct.cpp (depends on llvm-reflection branch + mirror library)
- https://godbolt.org/z/eo3zrro4v (using boost/ext static reflection, latest version is not currently compiling)
