#include "benchmark_reflect_serialization.hpp"
#include "benchmark_helper.hpp"

#include <rfl.hpp>
#include <rfl/json.hpp>

void bench_reflect_cpp(std::vector<User>& data) {
  std::string output = rfl::json::write(data);
  size_t output_volume = output.size();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(
    1, output_volume, "bench_reflect_cpp",
    bench([&data, &measured_volume, &output_volume] () {
      std::string output = rfl::json::write(data);
      measured_volume = output.size();
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}