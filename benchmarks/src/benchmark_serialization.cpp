#include "fast_json_serializer.hpp"
#include "fast_json_serializer2.hpp"
#include "fast_json_serializer_simpler.hpp"
#include "../../src/json_utils.hpp"
#include "../../src/simpler_reflection.hpp"
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


/* This struct is based on one example from the
 * https://app.json-generator.com/9jQKNGW0cMIA */

struct Location {
  double lat;
  double lng;
};

struct Profile {
  std::string name;
  std::string company;
  std::string dob;
  std::string address;
  Location location;
  std::string about;
};

struct User {
  std::string id;
  std::string email;
  std::string username;
  /*
  struct Profile {
    std::string name;
    std::string company;
    std::string dob;
    std::string address;
    Location location;
    std::string about;
  } profile;*/
  Profile profile;
  std::string apiKey;
  std::vector<std::string> roles;
  std::string createdAt;
  std::string updatedAt;
};

#include "custom_serializer.h"

template <> struct std::formatter<Location> : experimental_json_builder::universal_formatter { };
template <> struct std::formatter<Profile> : experimental_json_builder::universal_formatter { };
template <> struct std::formatter<User> : experimental_json_builder::universal_formatter { };

std::string generate_email(const std::string &name,
                           const std::string &company) {
  std::string email = name + "@" + company + ".com";
  std::transform(email.begin(), email.end(), email.begin(), ::tolower);
  return email;
}

std::string generate_username(const std::string &name, const std::string &dob) {
  std::string username = name.substr(0, name.find(' ')) + dob.substr(2, 2);
  std::transform(username.begin(), username.end(), username.begin(), ::tolower);
  return username;
}

std::string generate_date() {
  std::mt19937_64 rng(
      std::chrono::steady_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<int> year_dist(1988, 1995);
  std::uniform_int_distribution<int> month_dist(1, 12);
  std::uniform_int_distribution<int> day_dist(
      1, 28); // Assume February has 28 days for simplicity
  int year = year_dist(rng);
  int month = month_dist(rng);
  int day = day_dist(rng);
  return std::to_string(year) + "-" + (month < 10 ? "0" : "") +
         std::to_string(month) + "-" + (day < 10 ? "0" : "") +
         std::to_string(day);
}

std::string generate_address() {
  std::mt19937_64 rng(
      std::chrono::steady_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<int> num_dist(1, 100);
  std::uniform_int_distribution<int> street_dist(1000, 9999);
  std::uniform_int_distribution<int> city_dist(1, 10);
  std::uniform_int_distribution<int> state_dist(1, 50);
  return std::to_string(num_dist(rng)) + " Street" + ", City" +
         std::to_string(city_dist(rng)) + ", State" +
         std::to_string(state_dist(rng));
}

std::string generate_about() {
  return "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
}

std::string generate_guid() {
  std::string guid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(0, 15);
  const char *hex_chars = "0123456789abcdef";
  for (char &c : guid) {
    if (c == 'x')
      c = hex_chars[dis(gen)];
    else if (c == 'y')
      c = hex_chars[(dis(gen) & 0x3) | 0x8];
  }
  return guid;
}

User generate_random_user() {
  User user;
  user.id = generate_guid();
  user.profile.name = "John Doe"; // Placeholder for random name generation
  user.profile.company =
      "Example Company"; // Placeholder for random company generation
  user.profile.dob = generate_date();
  user.profile.address = generate_address();
  user.profile.location.lat = 0.0; // Placeholder for random latitude generation
  user.profile.location.lng =
      0.0; // Placeholder for random longitude generation
  user.profile.about = generate_about();
  user.apiKey = generate_guid();
  user.roles = {"owner", "admin", "member", "guest"};
  std::shuffle(
      user.roles.begin(), user.roles.end(),
      std::default_random_engine(
          std::chrono::system_clock::now().time_since_epoch().count()));
  user.createdAt = generate_date();
  user.updatedAt = user.createdAt; // Assume updated at creation time

  user.email = generate_email(user.profile.name, user.profile.company);
  user.username = generate_username(user.profile.name, user.profile.dob);

  return user;
}

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


template<typename T>
void bench(std::vector<T> &data) {
  size_t volume = std::accumulate(
      data.begin(), data.end(), size_t(0),
      [](size_t a, const T &b) { return a + sizeof(b); });
  size_t output_volume = std::accumulate(
      data.begin(), data.end(), size_t(0),
      [](size_t a, const T &b) { return a + experimental_json_builder::to_json_string(b).size(); });
  size_t max_string_length = experimental_json_builder::to_json_string(data[0]).size();
  size_t min_string_length = max_string_length;
  for(size_t i = 1; i < data.size(); i++) {
    size_t this_size = experimental_json_builder::to_json_string(data[i]).size();
    if(this_size > max_string_length) { max_string_length = this_size; }
    if(this_size < min_string_length) { min_string_length = this_size; }
  }

  size_t max_size = sizeof(std::max_element(data.begin(), data.end(),
                                     [](const T &a,
                                        const T &b) {
                                       return sizeof(a) > sizeof(b);
                                     }));
  printf("# volume: %zu bytes\n", volume);
  printf("# output volume: %zu bytes\n", output_volume);
  printf("# output volume per string: %0.1f bytes\n", double(output_volume) / data.size());
  printf("# min output volume per string: %zu bytes\n", max_string_length);
  printf("# max output volume per string: %zu bytes\n", min_string_length);

  printf("# max length: %zu bytes\n", max_size);
  printf("# number of inputs: %zu\n", data.size());

  printf("# serialization\n");
  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "experimental_json_builder::to_json_string",
    bench([&data, &measured_volume, &output_volume] () {
      measured_volume = 0;
      for (const T& x : data) {
        // The compiler is not smart enough to optimize the string creation.
        std::string out = experimental_json_builder::to_json_string(x);
        measured_volume += out.size();
      }
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}

template<typename T>
void bench_no_alloc(std::vector<T> &data) {
  size_t volume = std::accumulate(
      data.begin(), data.end(), size_t(0),
      [](size_t a, const T &b) { return a + sizeof(b); });
  
  
  size_t output_volume = std::accumulate(
      data.begin(), data.end(), size_t(0),
      [](size_t a, const T &b) { return a + experimental_json_builder::to_json_string(b).size(); });
  size_t max_string_length = experimental_json_builder::to_json_string(data[0]).size();
  size_t min_string_length = max_string_length;
  for(size_t i = 1; i < data.size(); i++) {
    size_t this_size = experimental_json_builder::to_json_string(data[i]).size();
    if(this_size > max_string_length) { max_string_length = this_size; }
    if(this_size < min_string_length) { min_string_length = this_size; }
  }

  auto sb = experimental_json_builder::StringBuilder(); // definitely more than we need
  for (const User& x : data) {
    experimental_json_builder::to_json_string(x, sb);
  }
  output_volume = sb.size();

  size_t max_size = sizeof(std::max_element(data.begin(), data.end(),
                                     [](const T &a,
                                        const T &b) {
                                       return sizeof(a) > sizeof(b);
                                     }));
  printf("# volume: %zu bytes\n", volume);
  printf("# output volume: %zu bytes\n", output_volume);
  printf("# output volume per string: %0.1f bytes\n", double(output_volume) / data.size());
  printf("# min output volume per string: %zu bytes\n", max_string_length);
  printf("# max output volume per string: %zu bytes\n", min_string_length);

  printf("# max length: %zu bytes\n", max_size);
  printf("# number of inputs: %zu\n", data.size());

  printf("# serialization\n");        
  auto sb2 = experimental_json_builder::StringBuilder();

  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "experimental_json_builder::to_json_string with stringBuilder",
    bench([&data, &measured_volume, &output_volume, &sb2] () {
      for (const T& x : data) {
        // The compiler is not smart enough to optimize the string creation.
        experimental_json_builder::to_json_string(x, sb2);
      }
      measured_volume = sb2.size();
    })
  );
  if(measured_volume != output_volume * 100) { printf("mismatch\n"); }
}

template<typename T>
void bench_simpler_reflection(std::vector<T> &data) {
  
  size_t volume = std::accumulate(
      data.begin(), data.end(), size_t(0),
      [](size_t a, const T &b) { return a + sizeof(b); });
  size_t output_volume = std::accumulate(
      data.begin(), data.end(), size_t(0),
      [](size_t a, const T &b) { return a + std::format("{}", b).size(); });
  size_t max_string_length = std::format("{}", data[0]).size();
  size_t min_string_length = max_string_length;
  for(size_t i = 1; i < data.size(); i++) {
    size_t this_size = std::format("{}", data[i]).size();
    if(this_size > max_string_length) { max_string_length = this_size; }
    if(this_size < min_string_length) { min_string_length = this_size; }
  }

  size_t max_size = sizeof(std::max_element(data.begin(), data.end(),
                                     [](const T &a,
                                        const T &b) {
                                       return sizeof(a) > sizeof(b);
                                     }));
  printf("# volume: %zu bytes\n", volume);
  printf("# output volume: %zu bytes\n", output_volume);
  printf("# output volume per string: %0.1f bytes\n", double(output_volume) / data.size());
  printf("# min output volume per string: %zu bytes\n", max_string_length);
  printf("# max output volume per string: %zu bytes\n", min_string_length);

  printf("# max length: %zu bytes\n", max_size);
  printf("# number of inputs: %zu\n", data.size());

  printf("# serialization\n");
  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "experimental_json_builder::universal_formatter",
    bench([&data, &measured_volume, &output_volume] () {
      measured_volume = 0;
      for (const T& x : data) {
        // The compiler is not smart enough to optimize the string creation.
        std::string out = std::format("{}", x);
        measured_volume += out.size();
      }
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}


void bench_custom(std::vector<User> &data) {
  
  size_t volume = std::accumulate(
      data.begin(), data.end(), size_t(0),
      [](size_t a, const User &b) { return a + sizeof(b); });
  // this is going to be approximate.
  size_t output_volume = std::accumulate(
      data.begin(), data.end(), size_t(0),
      [](size_t a, const User &b) { return a + std::format("{}", b).size(); });
  size_t max_string_length = std::format("{}", data[0]).size();
  size_t min_string_length = max_string_length;
  for(size_t i = 1; i < data.size(); i++) {
    size_t this_size = std::format("{}", data[i]).size();
    if(this_size > max_string_length) { max_string_length = this_size; }
    if(this_size < min_string_length) { min_string_length = this_size; }
  }
  std::vector<char> output(2*max_string_length); // plenty of memory.
  output_volume = 0;
  for (const User& x : data) {
    output_volume += custom::serialize(x, output.data());
  }
  size_t max_size = sizeof(std::max_element(data.begin(), data.end(),
                                     [](const User &a,
                                        const User &b) {
                                       return sizeof(a) > sizeof(b);
                                     }));
  printf("# volume: %zu bytes\n", volume);
  printf("# output volume: %zu bytes\n", output_volume);
  printf("# output volume per string: %0.1f bytes\n", double(output_volume) / data.size());
  printf("# min output volume per string: %zu bytes\n", max_string_length);
  printf("# max output volume per string: %zu bytes\n", min_string_length);

  printf("# max length: %zu bytes\n", max_size);
  printf("# number of inputs: %zu\n", data.size());

  printf("# serialization\n");
  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "experimental_json_builder::custom",
    bench([&data, &measured_volume, &output_volume, &output] () {
      measured_volume = 0;
      for (const User& x : data) {
         measured_volume += custom::serialize(x, output.data());
      }
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}

template <class T>
void bench_fast(std::vector<T> &data) {
  fast_json_serializer::StringBuilder b;
  fast_json_serializer::fast_to_json_string(b, data);
  size_t output_volume = b.size();
  b.reset();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "bench_fast",
    bench([&data, &measured_volume, &output_volume, &b] () {
      b.reset();
      fast_json_serializer::fast_to_json_string(b, data);
      measured_volume = b.size();
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}


template <class T>
void bench_fast_simpler(std::vector<T> &data) {
  fast_json_serializer_simpler::StringBuilder b;
  fast_json_serializer_simpler::fast_to_json_string(b, data);
  size_t output_volume = b.size();
  b.reset();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "bench_fast_simpler",
    bench([&data, &measured_volume, &output_volume, &b] () {
      b.reset();
      fast_json_serializer_simpler::fast_to_json_string(b, data);
      measured_volume = b.size();
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}

template <class T>
void bench_fast_v2(std::vector<T> &data) {
  fast_json_serializer2::StringBuilder b;
  fast_json_serializer2::fast_to_json_string(b, data);
  size_t output_volume = b.size();
  b.reset();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "bench_fast_v2",
    bench([&data, &measured_volume, &output_volume, &b] () {
      b.reset();
      fast_json_serializer2::fast_to_json_string(b, data);
      measured_volume = b.size();
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}


template <class T>
void bench_fast_one_by_one(std::vector<T> &data) {
  fast_json_serializer::StringBuilder b;
  for(T& t: data) {
    fast_json_serializer::fast_to_json_string(b, t);
  }
  size_t output_volume = b.size();
  b.reset();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "bench_fast_one_by_one",
    bench([&data, &measured_volume, &output_volume, &b] () {
      b.reset();
      for(T& t: data) {
        fast_json_serializer::fast_to_json_string(b, t);
      }
      measured_volume = b.size();
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}


template <class T>
void bench_fast_one_by_one_v2(std::vector<T> &data) {
  fast_json_serializer2::StringBuilder b;
  for(T& t: data) {
    fast_json_serializer2::fast_to_json_string(b, t);
  }
  size_t output_volume = b.size();
  b.reset();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "bench_fast_one_by_one_v2",
    bench([&data, &measured_volume, &output_volume, &b] () {
      b.reset();
      for(T& t: data) {
        fast_json_serializer2::fast_to_json_string(b, t);
      }
      measured_volume = b.size();
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}

int main() {
  constexpr int test_sz = 50'000;
  std::vector<User> test_data(test_sz);
  for(int i = 0; i < test_sz; ++i) test_data[i] = generate_random_user();
  
  std::vector<std::vector<User>> in_array = {  test_data  };
  // bench<std::vector<User>>(in_array); // currently not supported
  // bench_simpler_reflection<std::vector<User>>(in_array);
  // bench<User>(test_data);
  // bench_no_alloc<User>(test_data);
  // bench_simpler_reflection<User>(test_data);
  bench_custom(test_data);
  bench_fast_simpler(test_data);
  bench_fast(test_data);
  bench_fast_v2(test_data);
  bench_fast_one_by_one(test_data);
  bench_fast_one_by_one_v2(test_data);

  return EXIT_SUCCESS;
}