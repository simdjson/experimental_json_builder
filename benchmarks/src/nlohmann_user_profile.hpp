#ifndef NLOHMANN_USER_PROFILE_HPP
#define NLOHMANN_USER_PROFILE_HPP

#include <nlohmann/json.hpp>
#include "user_profile.hpp"

std::string nlohmann_serialize(const User& user) {
    nlohmann::json user_json = {
        {"id", user.id},
        {"email", user.email},
        {"username", user.username},
        {"profile", {
            {"name", user.profile.name},
            {"company", user.profile.company},
            {"dob", user.profile.dob},
            {"address", user.profile.address},
            {"location", {
                {"lat", user.profile.location.lat},
                {"lng", user.profile.location.lng}
            }},
            {"about", user.profile.about}
        }},
        {"apiKey", user.apiKey},
        {"roles", user.roles},
        {"createdAt", user.createdAt},
        {"updatedAt", user.updatedAt}
    };
    return user_json.dump();
}

std::string nlohmann_serialize(const std::vector<User>& users) {
    nlohmann::json users_json = nlohmann::json::array();
    for (const User& user : users) {
        nlohmann::json user_json = {
            {"id", user.id},
            {"email", user.email},
            {"username", user.username},
            {"profile", {
                {"name", user.profile.name},
                {"company", user.profile.company},
                {"dob", user.profile.dob},
                {"address", user.profile.address},
                {"location", {
                    {"lat", user.profile.location.lat},
                    {"lng", user.profile.location.lng}
                }},
                {"about", user.profile.about}
            }},
            {"apiKey", user.apiKey},
            {"roles", user.roles},
            {"createdAt", user.createdAt},
            {"updatedAt", user.updatedAt}
        };
        users_json.push_back(user_json);
    }
    return users_json.dump();
}

#endif