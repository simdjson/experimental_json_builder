#ifndef TWITTER_DATA_HPP
#define TWITTER_DATA_HPP

#include <string>
#include <vector>

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

    bool operator==(const User &other) const {
        return id == other.id &&
               name == other.name &&
               screen_name == other.screen_name &&
               location == other.location &&
               description == other.description &&
               verified == other.verified &&
               followers_count == other.followers_count &&
               friends_count == other.friends_count &&
               statuses_count == other.statuses_count;
    }
};

struct Hashtag {
    std::string text;
    int64_t indices_start;
    int64_t indices_end;

    bool operator==(const Hashtag &other) const {
        return text == other.text &&
               indices_start == other.indices_start &&
               indices_end == other.indices_end;
    }
};

struct Url {
    std::string url;
    std::string expanded_url;
    std::string display_url;
    int64_t indices_start;
    int64_t indices_end;

    bool operator==(const Url &other) const {
        return url == other.url &&
               expanded_url == other.expanded_url &&
               display_url == other.display_url &&
               indices_start == other.indices_start &&
               indices_end == other.indices_end;
    }
};

struct UserMention {
    int64_t id;
    std::string name;
    std::string screen_name;
    int64_t indices_start;
    int64_t indices_end;

    bool operator==(const UserMention &other) const {
        return id == other.id &&
               name == other.name &&
               screen_name == other.screen_name &&
               indices_start == other.indices_start &&
               indices_end == other.indices_end;
    }
};

struct Entities {
    std::vector<Hashtag> hashtags;
    std::vector<Url> urls;
    std::vector<UserMention> user_mentions;

    bool operator==(const Entities &other) const {
        return hashtags == other.hashtags &&
               urls == other.urls &&
               user_mentions == other.user_mentions;
    }
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

    bool operator==(const Status &other) const {
        return created_at == other.created_at &&
               id == other.id &&
               text == other.text &&
               user == other.user &&
               entities == other.entities &&
               retweet_count == other.retweet_count &&
               favorite_count == other.favorite_count &&
               favorited == other.favorited &&
               retweeted == other.retweeted;
    }
};

struct TwitterData {
    std::vector<Status> statuses;

    bool operator==(const TwitterData &other) const {
        return statuses == other.statuses;
    }
};

#endif