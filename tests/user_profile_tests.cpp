
#include "simdjson/json_builder/json_builder.h"
#include "simdjson/json_builder/string_builder.h"
#include <nlohmann/json.hpp>
#include <simdjson.h>

#include <concepts>
#include <iostream>
#include <random>
#include <vector>

template <auto... Xs, typename F> constexpr void for_values(F &&f) {
  (f.template operator()<Xs>(), ...);
}

template <auto B, auto E, typename F> constexpr void for_range(F &&f) {
  using t = std::common_type_t<decltype(B), decltype(E)>;

  [&f]<auto... Xs>(std::integer_sequence<t, Xs...>) {
    for_values<(B + Xs)...>(f);
  }(std::make_integer_sequence<t, E - B>{});
}

template <typename T> consteval auto member_info(int n) {
  return nonstatic_data_members_of(^^T)[n];
}

std::mt19937 rng(12345);
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
  Profile profile;
  std::string apiKey;
  std::vector<std::string> roles;
  std::string createdAt;
  std::string updatedAt;
};

User parseUser(simdjson::ondemand::value val) {
  User user;
  simdjson::ondemand::object obj = val.get_object();
  obj["id"].get_string(user.id);
  obj["email"].get_string(user.email);
  obj["username"].get_string(user.username);
  simdjson::ondemand::object profile = obj["profile"].get_object();
  profile["name"].get_string(user.profile.name);
  profile["company"].get_string(user.profile.company);
  profile["dob"].get_string(user.profile.dob);
  profile["address"].get_string(user.profile.address);
  simdjson::ondemand::object location = profile["location"].get_object();
  user.profile.location.lat = location["lat"].get_double();
  user.profile.location.lng = location["lng"].get_double();
  profile["about"].get_string(user.profile.about);
  obj["apiKey"].get_string(user.apiKey);
  simdjson::ondemand::array roles = obj["roles"].get_array();
  for (simdjson::ondemand::value role : roles) {
    std::string_view v = role.get_string();
    user.roles.push_back(std::string(v));
  }
  obj["createdAt"].get_string(user.createdAt);
  obj["updatedAt"].get_string(user.updatedAt);
  return user;
}

std::vector<User> parseArrayOfUsers(simdjson::ondemand::value val) {
  std::vector<User> users;
  simdjson::ondemand::array arr = val.get_array();
  for (simdjson::ondemand::value user : arr) {
    users.push_back(parseUser(user));
  }
  return users;
}

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
  std::uniform_int_distribution<int> dis(0, 15);
  const char *hex_chars = "0123456789abcdef";
  for (char &c : guid) {
    if (c == 'x')
      c = hex_chars[dis(rng)];
    else if (c == 'y')
      c = hex_chars[(dis(rng) & 0x3) | 0x8];
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

std::string simdjson_serialize(const User &user) {
  simdjson::json_builder::string_builder b;
  simdjson::json_builder::fast_to_json_string(b, user);
  return std::string(b.view());
}

std::string nlohmann_serialize(const User &user) {
  nlohmann::json user_json = {{"id", user.id},
                              {"email", user.email},
                              {"username", user.username},
                              {"profile",
                               {{"name", user.profile.name},
                                {"company", user.profile.company},
                                {"dob", user.profile.dob},
                                {"address", user.profile.address},
                                {"location",
                                 {{"lat", user.profile.location.lat},
                                  {"lng", user.profile.location.lng}}},
                                {"about", user.profile.about}}},
                              {"apiKey", user.apiKey},
                              {"roles", user.roles},
                              {"createdAt", user.createdAt},
                              {"updatedAt", user.updatedAt}};
  return user_json.dump();
}

template <typename T>
bool compare(const T &user1, const T &user2, const std::string &json) {
  bool result = true;
  for_range<0, nonstatic_data_members_of(^^T).size()>([&]<auto i>() {
    constexpr auto mem = member_info<T>(i);
    if constexpr (std::equality_comparable<typename[:type_of(mem):]>) {
      if (user1.[:mem:] != user2.[:mem:]) {
        std::cerr << std::format("{} mismatch: {} != {} in {}\n", identifier_of(mem),
                               user1.[:mem:], user2.[:mem:], json);
        result = false;
      }
    } else {
      if (!compare(user1.[:mem:], user2.[:mem:], json)) {
        result = false;
      }
    }
  });

  return result;
}

bool round_trip_nlohmann(const User &user) {
  simdjson::ondemand::parser parser;
  std::string json = nlohmann_serialize(user);
  simdjson::ondemand::document doc = parser.iterate(json);
  User user2 = parseUser(doc);
  bool equal = compare(user, user2, json);
  if (!equal) {
    std::cerr << "Round trip failed for User: " << json << std::endl;
    return false;
  }
  return true;
}

bool round_trip_simdjson(const User &user) {
  simdjson::ondemand::parser parser;
  std::string json = simdjson_serialize(user);
  simdjson::ondemand::document doc = parser.iterate(json);
  User user2 = parseUser(doc);
  bool equal = compare(user, user2, json);
  if (!equal) {
    std::cerr << "Round trip failed for User: " << json << std::endl;
    return false;
  }
  return true;
}

int main() {
  for (size_t i = 0; i < 100000; i++) {
    if (i % 1000 == 0) {
      printf(".");
      fflush(NULL);
    }
    User user = generate_random_user();

    if (!round_trip_nlohmann(user)) {
      return EXIT_FAILURE;
    }
    if (!round_trip_simdjson(user)) {
      return EXIT_FAILURE;
    }
  }
  printf("\n");
  return EXIT_SUCCESS;
}
