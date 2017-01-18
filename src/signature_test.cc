/* Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "src/signature.h"
#include "utils/md5.h"

#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

using std::string;
using ::google::protobuf::TextFormat;

namespace istio {
namespace mixer_client {
namespace {

class SignatureUtilTest : public ::testing::Test {
 protected:
  SignatureUtilTest() { attributes_map_.attributes.clear(); }

  void AddString(const string& key, const string& value) {
    Attributes::Value string_value;
    string_value.type = Attributes::Value::STRING;
    string_value.str_v = value;
    attributes_map_.attributes[key] = string_value;
  }

  void AddBytes(const string& key, const string& value) {
    Attributes::Value bytes_value;
    bytes_value.type = Attributes::Value::BYTES;
    bytes_value.str_v = value;
    attributes_map_.attributes[key] = bytes_value;
  }

  void AddTime(
      const string& key,
      const std::chrono::time_point<std::chrono::system_clock>& value) {
    Attributes::Value time_value;
    time_value.time_v = value;
    attributes_map_.attributes[key] = time_value;
  }

  void AddDoublePair(const string& key, const double& value) {
    Attributes::Value double_value;
    double_value.type = Attributes::Value::DOUBLE;
    double_value.value.double_v = value;
    attributes_map_.attributes[key] = double_value;
  }

  void AddInt64Pair(const string& key, const int64_t& value) {
    Attributes::Value int64_value;
    int64_value.type = Attributes::Value::INT64;
    int64_value.value.int64_v = value;
    attributes_map_.attributes[key] = int64_value;
  }

  void AddBoolPair(const string& key, const bool& value) {
    Attributes::Value bool_value;
    bool_value.type = Attributes::Value::BOOL;
    bool_value.value.bool_v = value;
    attributes_map_.attributes[key] = bool_value;
  }

  Attributes attributes_map_;
};

TEST_F(SignatureUtilTest, Attributes) {
  AddString("string-key", "this is a string value");
  EXPECT_EQ("26f8f724383c46e7f5803380ab9c17ba",
            MD5::DebugString(GenerateSignature(attributes_map_)));

  AddBytes("bytes-key", "this is a bytes value");
  EXPECT_EQ("1f409524b79b9b5760032dab7ecaf960",
            MD5::DebugString(GenerateSignature(attributes_map_)));

  AddDoublePair("double-key", 999);
  EXPECT_EQ("e9c0e5f10e2110ca0636d421da4da29e",
            MD5::DebugString(GenerateSignature(attributes_map_)));

  AddInt64Pair("int-key", 35);
  EXPECT_EQ("02da0af21811309e2290c37a703082d5",
            MD5::DebugString(GenerateSignature(attributes_map_)));

  AddBoolPair("bool-key", true);
  EXPECT_EQ("b1ffa4e01c3882938d6fd75a575f259a",
            MD5::DebugString(GenerateSignature(attributes_map_)));
}

}  // namespace
}  // namespace mixer_client
}  // namespace istio
