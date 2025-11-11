#include "simdjson/json_builder/universal_formatter.h"
#include <gtest/gtest.h>

TEST(UniversalFormatterTest, EmptyStruct) {
  struct Empty {};

  Empty obj;
  simdjson::json_builder::universal_formatter universal_formatter;
  simdjson::json_builder::string_builder string_builder;
  universal_formatter.format(obj, string_builder);

  EXPECT_EQ(string_builder.view(), "{}");
}