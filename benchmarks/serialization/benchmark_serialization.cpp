#include "../../src/json_utils.hpp"
#include "event_counter.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <vector>

/* This struct is based on one example from the
 * https://app.json-generator.com/9jQKNGW0cMIA */

struct Location {
  double lat;
  double lng;
};

struct User {
  std::string id;
  std::string email;
  std::string username;
  struct Profile {
    std::string name;
    std::string company;
    std::string dob;
    std::string address;
    Location location;
    std::string about;
  } profile;
  std::string apiKey;
  std::vector<std::string> roles;
  std::string createdAt;
  std::string updatedAt;
};

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

// Source of the 2 functions below:
// https://github.com/simdutf/simdutf/blob/master/benchmarks/base64/benchmark_base64.cpp
void pretty_print(size_t, size_t bytes, std::string name, event_aggregate agg) {
  printf("%-40s : ", name.c_str());
  printf(" %5.2f GB/s ", bytes / agg.elapsed_ns());
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
      [](size_t a, const std::vector<char> &b) { return a + b.size(); });
  size_t max_size = std::max_element(data.begin(), data.end(),
                                     [](const T &a,
                                        const T &b) {
                                       return a.size() < b.size();
                                     })
                        ->size();
  printf("# volume: %zu bytes\n", volume);
  printf("# max length: %zu bytes\n", max_size);
  printf("# number of inputs: %zu\n", data.size());

  printf("# serialization\n");
  pretty_print(
    data.size(), volume, "experimental_json_builder::to_json_string",
    bench([&data] () {
      for (const &T : data) {
        // IO is probably not the best idea here since it might add significant overead (Get Daniel's feedback)
        std::cout << experimental_json_builder::to_json_string() << std::endl;
      }
    });
  );
}



int main() {
  constexpr int test_sz = 100'000;
  vector<User> test_data(test_sz);
  for(int i = 0; i < test_sz; ++i) test_data[i] = generate_random_user();
  bench<User>(test_data);
  return 0;
}