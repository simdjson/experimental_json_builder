#ifndef USER_PROFILE_HPP
#define USER_PROFILE_HPP

#include <string>
#include <vector>

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
  Profile profile;
  std::string apiKey;
  std::vector<std::string> roles;
  std::string createdAt;
  std::string updatedAt;
};

#endif