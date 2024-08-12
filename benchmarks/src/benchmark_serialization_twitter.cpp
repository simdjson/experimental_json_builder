#include "benchmark_helper.hpp"
#include "from_json.hpp"
#include "nlohmann_twitter_data.hpp"
#include "simdjson/json_builder/json_builder.h"
#include "simdjson/json_builder/string_builder.h"
#include "twitter_data.hpp"
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <simdjson.h>
#include <string>
#if SIMDJSON_BENCH_CPP_REFLECT
#include "benchmark_reflect_serialization_twitter.hpp"
#endif

template <class T> void bench_fast_simpler(T &data) {
  simdjson::json_builder::string_builder b;
  simdjson::json_builder::fast_to_json_string(b, data);
  size_t output_volume = b.size();
  b.reset();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(sizeof(data), output_volume, "bench_fast_simpler",
               bench([&data, &measured_volume, &output_volume, &b]() {
                 b.reset();
                 simdjson::json_builder::fast_to_json_string(b, data);
                 measured_volume = b.size();
                 if (measured_volume != output_volume) {
                   printf("mismatch\n");
                 }
               }));
}


template <class T> void bench_fast_with_alloc(T &data) {
  std::string json = simdjson::json_builder::to_json_string(data);
  size_t output_volume = json.size();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(sizeof(data), output_volume, "bench_fast_with_alloc",
               bench([&data, &measured_volume, &output_volume]() {
                 std::string json = simdjson::json_builder::to_json_string(data);

                 measured_volume = json.size();
                 if (measured_volume != output_volume) {
                   printf("mismatch\n");
                 }
               }));
}


template <class T> void bench_fast_with_assign(T &data) {
  std::string json;
  simdjson::json_builder::to_json(data, json);
  size_t output_volume = json.size();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(sizeof(data), output_volume, "bench_fast_with_assign",
               bench([&data, &measured_volume, &output_volume, &json]() {
                 simdjson::json_builder::to_json(data, json);
                 measured_volume = json.size();
                 if (measured_volume != output_volume) {
                   printf("mismatch\n");
                 }
               }));
}

void bench_nlohmann(TwitterData &data) {
  std::string output = nlohmann_serialize(data);
  size_t output_volume = output.size();
  printf("# output volume: %zu bytes\n", output_volume);

  volatile size_t measured_volume = 0;
  pretty_print(1, output_volume, "bench_nlohmann",
               bench([&data, &measured_volume, &output_volume]() {
                 std::string output = nlohmann_serialize(data);
                 measured_volume = output.size();
                 if (measured_volume != output_volume) {
                   printf("mismatch\n");
                 }
               }));
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

std::string read_file(std::string filename) {
  printf("# Reading file %s\n", filename.c_str());
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

void test_correctness(std::string_view json_str) {
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
  simdjson::json_builder::string_builder b;
  simdjson::json_builder::fast_to_json_string(b, simd_struct);
  std::string simd_struct_to_json = std::string(b.view());

  // Parsing it back into a new struct
  TwitterData my_struct;
  simpleparser::json_parser::JsonParser parser(simd_struct_to_json);
  auto json_value = parser.parse();
  simpleparser::json_builder::from_json(json_value, my_struct);

  // Now let's validate the result to see if the round-trip struct is the same
  // as the original
  if (my_struct != simd_struct) {
    std::cout << "WARNING: the structs do not match !!!" << std::endl;
  } else {
    std::cout << "# Verification succesful, the structs match." << std::endl;
  }
}

int main() {
  // Testing correctness of round-trip (serialization + deserialization)
  const std::string json_str = read_file(JSON_FILE);

  test_correctness(json_str);

  // Benchmarking the serialization
  simpleparser::json_parser::JsonParser parser(json_str);
  auto json_value = parser.parse();
  TwitterData my_struct;
  bench_nlohmann(my_struct);
  simpleparser::json_builder::from_json(json_value, my_struct);
  bench_fast_simpler(my_struct);
  bench_fast_with_alloc(my_struct);
  bench_fast_with_assign(my_struct);
#if SIMDJSON_BENCH_CPP_REFLECT
  bench_reflect_cpp(my_struct);
#endif
  return EXIT_SUCCESS;
}