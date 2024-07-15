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
#include "benchmark_helper.hpp"

#if SIMDJSON_BENCH_CPP_REFLECT
#include "benchmark_reflect_serialization.hpp"
#endif
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


void bench_custom(std::vector<User> &data) {
  size_t volume = std::accumulate(data.begin(), data.end(), size_t(0), [](size_t a, const User &b) { return a + sizeof(b); });
  std::vector<char> buff(8192);

  size_t output_volume = std::accumulate(data.begin(), data.end(), size_t(0), [&buff](size_t a, const User &b) {
    return a + custom::serialize(b, buff.data());
  });

  size_t max_string_length = custom::serialize(data[0], buff.data());
  size_t min_string_length = max_string_length;

  for (size_t i = 1; i < data.size(); i++) {
    size_t this_size = custom::serialize(data[i], buff.data());
    if (this_size > max_string_length) { max_string_length = this_size; }
    if (this_size < min_string_length) { min_string_length = this_size; }
  }

  std::vector<char> output(2 * max_string_length); // plenty of memory.
  output_volume = 0;

  for (const User& x : data) {
    output_volume += custom::serialize(x, output.data());
  }

  size_t max_size = sizeof(std::max_element(data.begin(), data.end(), [](const User &a, const User &b) {
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
      if (measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
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


void bench_nlohmann(std::vector<User> &data) {
  std::string output = nlohmann_serialize(data);
  size_t output_volume = output.size();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(
    data.size(), output_volume, "bench_nlohmann",
    bench([&data, &measured_volume, &output_volume] () {
      std::string output = nlohmann_serialize(data);
      measured_volume = output.size();
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}



int main() {
  constexpr int test_sz = 50'000;
  std::vector<User> test_data(test_sz);
  for(int i = 0; i < test_sz; ++i) test_data[i] = generate_random_user();
  
  std::vector<std::vector<User>> in_array = {  test_data  };
  bench_nlohmann(test_data);
  bench_custom(test_data);
  bench_fast_simpler(test_data);
#if SIMDJSON_BENCH_CPP_REFLECT
  bench_reflect_cpp(test_data);
#endif
  return EXIT_SUCCESS;
}