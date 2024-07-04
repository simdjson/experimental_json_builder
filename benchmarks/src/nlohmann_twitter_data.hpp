#ifndef NLOHMANN_TWITTER_DATA_HPP
#define NLOHMANN_TWITTER_DATA_HPP

#include <nlohmann/json.hpp>
#include "twitter_data.hpp" // Assuming you have the twitter data structures in this header

void to_json(nlohmann::json& j, const User& u) {
    j = nlohmann::json{{"id", u.id}, {"name", u.name}, {"screen_name", u.screen_name}, {"location", u.location}, {"description", u.description}, {"verified", u.verified}, {"followers_count", u.followers_count}, {"friends_count", u.friends_count}, {"statuses_count", u.statuses_count}};
}

void to_json(nlohmann::json& j, const Hashtag& h) {
    j = nlohmann::json{{"text", h.text}, {"indices_start", h.indices_start}, {"indices_end", h.indices_end}};
}

void to_json(nlohmann::json& j, const Url& u) {
    j = nlohmann::json{{"url", u.url}, {"expanded_url", u.expanded_url}, {"display_url", u.display_url}, {"indices_start", u.indices_start}, {"indices_end", u.indices_end}};
}

void to_json(nlohmann::json& j, const UserMention& um) {
    j = nlohmann::json{{"id", um.id}, {"name", um.name}, {"screen_name", um.screen_name}, {"indices_start", um.indices_start}, {"indices_end", um.indices_end}};
}

void to_json(nlohmann::json& j, const Entities& e) {
    j = nlohmann::json{{"hashtags", e.hashtags}, {"urls", e.urls}, {"user_mentions", e.user_mentions}};
}

void to_json(nlohmann::json& j, const Status& s) {
    j = nlohmann::json{{"created_at", s.created_at}, {"id", s.id}, {"text", s.text}, {"user", s.user}, {"entities", s.entities}, {"retweet_count", s.retweet_count}, {"favorite_count", s.favorite_count}, {"favorited", s.favorited}, {"retweeted", s.retweeted}};
}

void to_json(nlohmann::json& j, const TwitterData& t) {
    j = nlohmann::json{{"statuses", t.statuses}};
}

std::string nlohmann_serialize(const TwitterData& data) {
    nlohmann::json j = data;
    return j.dump();
}

#endif // NLOHMANN_TWITTER_DATA_HPP
