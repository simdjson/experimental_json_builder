#include "experimental_json_builder.hpp"
#include "json_escaping.hpp"
#include "event_counter.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <format>
#include <atomic>
#include "user_profile.hpp"
#include "custom_serializer.h"
#include "nlohmann_user_profile.hpp"

struct User {
    int64_t id;
    std::string name;
    std::string screen_name;
    std::string location;
    std::string description;
    bool verified;
    int64_t followers_count;
    int64_t friends_count;
    int64_t statuses_count;
};

struct Hashtag {
    std::string text;
    int64_t indices_start;
    int64_t indices_end;
};

struct Url {
    std::string url;
    std::string expanded_url;
    std::string display_url;
    int64_t indices_start;
    int64_t indices_end;
};

struct UserMention {
    int64_t id;
    std::string name;
    std::string screen_name;
    int64_t indices_start;
    int64_t indices_end;
};

struct Entities {
    std::vector<Hashtag> hashtags;
    std::vector<Url> urls;
    std::vector<UserMention> user_mentions;
};

struct Status {
    std::string created_at;
    int64_t id;
    std::string text;
    User user;
    Entities entities;
    int64_t retweet_count;
    int64_t favorite_count;
    bool favorited;
    bool retweeted;
};

struct TwitterData {
    std::vector<Status> statuses;
};

event_collector collector;

template <class function_type>
event_aggregate bench(const function_type &function, size_t min_repeat = 10,
                      size_t min_time_ns = 1000000000,
                      size_t max_repeat = 100) {
  event_aggregate aggregate{};
  size_t N = min_repeat;
  if (N == 0) {
    N = 1;
  }
  for (size_t i = 0; i < N; i++) {
    std::atomic_thread_fence(std::memory_order_acquire);
    collector.start();
    function();
    std::atomic_thread_fence(std::memory_order_release);
    event_count allocate_count = collector.end();
    aggregate << allocate_count;
    if ((i + 1 == N) && (aggregate.total_elapsed_ns() < min_time_ns) &&
        (N < max_repeat)) {
      N *= 10;
    }
  }
  return aggregate;
}

// Source of the 2 functions below:
// https://github.com/simdutf/simdutf/blob/master/benchmarks/base64/benchmark_base64.cpp
void pretty_print(size_t strings, size_t bytes, std::string name, event_aggregate agg) {
  printf("%-60s : ", name.c_str());
  printf(" %5.2f MB/s ", bytes * 1000 / agg.elapsed_ns());
  printf(" %5.2f Ms/s ", strings * 1000 / agg.elapsed_ns());
  if (collector.has_events()) {
    printf(" %5.2f GHz ", agg.cycles() / agg.elapsed_ns());
    printf(" %5.2f c/b ", agg.cycles() / bytes);
    printf(" %5.2f i/b ", agg.instructions() / bytes);
    printf(" %5.2f i/c ", agg.instructions() / agg.cycles());
  }
  printf("\n");
}

template <class T>
void bench_fast_simpler(std::vector<T> &data) {
  experimental_json_builder::StringBuilder b(32*1024*1024); // pre-allocate 32 MB
  experimental_json_builder::fast_to_json_string(b, data);
  size_t output_volume = b.size();
  b.reset();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "bench_fast_simpler",
    bench([&data, &measured_volume, &output_volume, &b] () {
      b.reset();
      experimental_json_builder::fast_to_json_string(b, data);
      measured_volume = b.size();
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}

int main()
{
    
}


