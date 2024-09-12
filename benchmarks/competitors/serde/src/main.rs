extern crate serde;
extern crate serde_json;

use std::fs;
use serde::{Serialize, Deserialize};
use std::time::Instant;

#[derive(Serialize, Deserialize, Debug)]
struct Metadata {
    result_type: String,
    iso_language_code: String,
}

#[derive(Serialize, Deserialize, Debug)]
struct User {
    id: i64,
    id_str: String,
    name: String,
    screen_name: String,
    location: String,
    description: String,
    url: Option<String>,
    protected: bool,
    followers_count: i64,
    friends_count: i64,
    listed_count: i64,
    created_at: String,
    favourites_count: i64,
    utc_offset: Option<i64>,
    time_zone: Option<String>,
    geo_enabled: bool,
    verified: bool,
    statuses_count: i64,
    lang: String,
    profile_background_color: String,
    profile_background_image_url: String,
    profile_background_image_url_https: String,
    profile_background_tile: bool,
    profile_image_url: String,
    profile_image_url_https: String,
    profile_banner_url: Option<String>,
    profile_link_color: String,
    profile_sidebar_border_color: String,
    profile_sidebar_fill_color: String,
    profile_text_color: String,
    profile_use_background_image: bool,
    default_profile: bool,
    default_profile_image: bool,
    following: bool,
    follow_request_sent: bool,
    notifications: bool,
}

#[derive(Serialize, Deserialize, Debug)]
struct Hashtag {}

#[derive(Serialize, Deserialize, Debug)]
struct Url {}

#[derive(Serialize, Deserialize, Debug)]
struct UserMention {
    screen_name: String,
    name: String,
    id: i64,
    id_str: String,
    indices: Vec<i64>,
}

#[derive(Serialize, Deserialize, Debug)]
struct Entities {
    hashtags: Vec<Hashtag>,
    urls: Vec<Url>,
    user_mentions: Vec<UserMention>,
}

#[derive(Serialize, Deserialize, Debug)]
struct Status {
    metadata: Metadata,
    created_at: String,
    id: i64,
    id_str: String,
    text: String,
    source: String,
    truncated: bool,
    in_reply_to_status_id: Option<i64>,
    in_reply_to_status_id_str: Option<String>,
    in_reply_to_user_id: Option<i64>,
    in_reply_to_user_id_str: Option<String>,
    in_reply_to_screen_name: Option<String>,
    user: User,
    geo: Option<String>,
    coordinates: Option<String>,
    place: Option<String>,
    contributors: Option<String>,
    retweet_count: i64,
    favorite_count: i64,
    favorited: bool,
    retweeted: bool,
    entities: Entities,
    lang: String,
}

#[derive(Serialize, Deserialize, Debug)]
struct TwitterData {
    statuses: Vec<Status>,
}

fn main() {
    // Read the JSON data from the file once
    let data = fs::read_to_string("/Users/random_person/Desktop/experimental_json_builder/benchmarks/src/data/twitter.json")
        .expect("Unable to read file");

    // Deserialize the JSON data into a TwitterData object once
    let twitter_data: TwitterData = serde_json::from_str(&data)
        .expect("Unable to deserialize data");

    let num_iterations = 500;
    let mut total_serialize_duration = 0.0;
    let mut output_volume = 0;

    for _ in 0..num_iterations {
        let start = Instant::now();
        let serialized = serde_json::to_string(&twitter_data)
            .expect("Unable to serialize data");
        let serialize_duration = start.elapsed().as_secs_f64();
        total_serialize_duration += serialize_duration;

        output_volume += serialized.len();
    }

    let avg_serialize_time = total_serialize_duration / num_iterations as f64;
    let throughput_mb_s = (output_volume as f64 / 1_000_000.0) / total_serialize_duration;

    println!("# volume: {} bytes", data.len());
    println!("# output volume: {} bytes", output_volume);
    println!("# output volume per string: {:.1} bytes", output_volume as f64 / num_iterations as f64);
    println!("# number of inputs: {}", num_iterations);
    println!("Serialization:");
    println!("  Avg time per operation: {:.6} s", avg_serialize_time);
    println!("  Throughput: {:.2} MB/s", throughput_mb_s);
}
