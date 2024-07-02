#include "../../src/experimental_json_builder.hpp"
#include "../../src/from_json.hpp"
#include "../../src/json_escaping.hpp"
#include "event_counter.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <format>
#include <atomic>
#include <simdjson.h>
#include <fstream>
#include <filesystem>

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

event_collector collector;

template <class function_type>
event_aggregate bench(const function_type &function, size_t min_repeat = 10,
                      size_t min_time_ns = 1000000000,
                      size_t max_repeat = 100000) {
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
void bench_fast_simpler(T &data) {
  experimental_json_builder::StringBuilder b(32*1024*1024); // pre-allocate 32 MB
  experimental_json_builder::fast_to_json_string(b, data);
  size_t output_volume = b.size();
  b.reset();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(
    sizeof(data), output_volume, "bench_fast_simpler",
    bench([&data, &measured_volume, &output_volume, &b] () {
      b.reset();
      experimental_json_builder::fast_to_json_string(b, data);
      measured_volume = b.size();
      if(measured_volume != output_volume) { printf("mismatch\n"); }
    })
  );
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string read_file(std::string filename) {
  printf("Reading file %s\n", filename.c_str());
  constexpr size_t read_size = 4096;
  auto stream = std::ifstream(filename.c_str());
  stream.exceptions(std::ios_base::badbit);
  std::string out;
  std::string buf(read_size, '\0');
  while (stream.read(&buf[0], read_size)) {
    out.append(buf, 0, size_t(stream.gcount()));
  }
  out.append(buf, 0, size_t(stream.gcount()));
  return out;
}

void test_correctness()
{
  std::string json_str = read_file(JSON_FILE);
  simdjson::dom::parser simd_parser;
  simdjson::dom::element simd_doc;
  auto error = simd_parser.parse(json_str).get(simd_doc);
  if (error) {
    std::cout << "simdjson parsing failed: " << error << std::endl;
    return;
  }

  // Parsing json_str into a struct using simdjson
  TwitterData simd_struct;
  for (auto status : simd_doc["statuses"]) {
      Status s;
      s.created_at = status["created_at"].get_c_str().value();
      s.id = status["id"].get_int64().value();
      s.text = status["text"].get_c_str().value();

      auto user = status["user"];
      s.user.id = user["id"].get_int64().value();
      s.user.name = user["name"].get_c_str().value();
      s.user.screen_name = user["screen_name"].get_c_str().value();
      s.user.location = user["location"].get_c_str().value();
      s.user.description = user["description"].get_c_str().value();
      s.user.verified = user["verified"].get_bool().value();
      s.user.followers_count = user["followers_count"].get_int64().value();
      s.user.friends_count = user["friends_count"].get_int64().value();
      s.user.statuses_count = user["statuses_count"].get_int64().value();

      for (auto hashtag : status["entities"]["hashtags"]) {
          Hashtag h;
          h.text = hashtag["text"].get_c_str().value();
          auto indices = hashtag["indices"];
          h.indices_start = indices.at(0).get_int64().value();
          h.indices_end = indices.at(1).get_int64().value();
          s.entities.hashtags.push_back(h);
      }

      for (auto url : status["entities"]["urls"]) {
          Url u;
          u.url = url["url"].get_c_str().value();
          u.expanded_url = url["expanded_url"].get_c_str().value();
          u.display_url = url["display_url"].get_c_str().value();
          auto indices = url["indices"];
          u.indices_start = indices.at(0).get_int64().value();
          u.indices_end = indices.at(1).get_int64().value();
          s.entities.urls.push_back(u);
      }

      for (auto user_mention : status["entities"]["user_mentions"]) {
          UserMention um;
          um.id = user_mention["id"].get_int64().value();
          um.name = user_mention["name"].get_c_str().value();
          um.screen_name = user_mention["screen_name"].get_c_str().value();
          auto indices = user_mention["indices"];
          um.indices_start = indices.at(0).get_int64().value();
          um.indices_end = indices.at(1).get_int64().value();
          s.entities.user_mentions.push_back(um);
      }

      s.retweet_count = status["retweet_count"].get_int64().value();
      s.favorite_count = status["favorite_count"].get_int64().value();
      s.favorited = status["favorited"].get_bool().value();
      s.retweeted = status["retweeted"].get_bool().value();

      simd_struct.statuses.push_back(s);
  }

  // Now let's do a round-trip

  // Serializing the simd_struct back to a string
  experimental_json_builder::StringBuilder b(32*1024*1024); // pre-allocate 32 MB
  experimental_json_builder::fast_to_json_string(b, simd_struct);
  std::string simd_struct_to_json = std::string(b.view());

  // Parsing it back into a new struct
  TwitterData my_struct;
  json_parser::JsonParser parser(simd_struct_to_json);
  auto json_value = parser.parse();
  experimental_json_builder::from_json(json_value, my_struct);

  // Now let's validate the result
  if (my_struct != simd_struct) {
    std::cout << "the structs do not match" << std::endl;
    experimental_json_builder::StringBuilder b1(32*1024*1024); // pre-allocate 32 MB
    experimental_json_builder::fast_to_json_string(b1, my_struct);
    experimental_json_builder::StringBuilder b2(32*1024*1024); // pre-allocate 32 MB
    experimental_json_builder::fast_to_json_string(b2, simd_struct);

    auto A = b1.view();
    auto B = b2.view();

    std::cout << A.size() << " " << B.size() << std::endl;
    if(A == B) std::cout << "equal " << std::endl;
    else std::cout << "different" << std::endl;
    return;
  }
  std::cout << "The structs match" << std::endl;
}

int main()
{
  // Testing correctness of round-trip (serialization + deserialization)
  test_correctness();

  // Benchmarking the serialization
  std::string json_str = read_file(JSON_FILE);
  json_parser::JsonParser parser(json_str);
  auto json_value = parser.parse();
  TwitterData my_struct;
  experimental_json_builder::from_json(json_value, my_struct);
  bench_fast_simpler(my_struct);

  return 0;
}