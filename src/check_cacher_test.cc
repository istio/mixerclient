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

#include "src/check_cacher.h"

#include "gmock/gmock.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"
#include "utils/status_test_util.h"

#include <unistd.h>

using std::string;
using ::istio::mixer::v1::CheckRequest;
using ::istio::mixer::v1::CheckResponse;
using ::google::protobuf::TextFormat;
using ::google::protobuf::util::MessageDifferencer;
using ::google::protobuf::util::Status;
using ::google::protobuf::util::error::Code;

namespace istio {
namespace mixer_client {
namespace {

const int kFlushIntervalMs = 100;
const int kExpirationMs = 200;

const char kSuccessResponse1[] = R"(
request_index: 1
result: {
 code: 0
 message: "success response"
}
)";
}

class CheckCacherTest : public ::testing::Test {
 public:
  void SetUp() {
    ASSERT_TRUE(
        TextFormat::ParseFromString(kSuccessResponse1, &pass_response1_));
    CheckOptions options(1 /*entries*/, kFlushIntervalMs, kExpirationMs);

    cacher_ = CreateCheckCacher(options);
    ASSERT_TRUE((bool)(cacher_));

    Attributes::Value string_value;
    string_value.type = Attributes::Value::STRING;
    string_value.str_v = "this-is-a-string-value";
    attributes1_.attributes["string-key"] = string_value;
  }

  Attributes attributes1_;
  CheckResponse pass_response1_;

  std::unique_ptr<CheckCacher> cacher_;
  std::vector<Attributes> flushed_;
};

TEST_F(CheckCacherTest, TestDisableCache) {
  CheckOptions options(0 /*entries*/, 1000, 2000);
  cacher_ = CreateCheckCacher(options);

  ASSERT_TRUE((bool)(cacher_));
  CheckResponse response;
  EXPECT_ERROR_CODE(Code::NOT_FOUND, cacher_->Check(attributes1_, &response));
}

TEST_F(CheckCacherTest, TestCachePassResponses) {
  CheckResponse response;
  EXPECT_ERROR_CODE(Code::NOT_FOUND, cacher_->Check(attributes1_, &response));

  EXPECT_OK(cacher_->CacheResponse(attributes1_, pass_response1_));
  EXPECT_OK(cacher_->Check(attributes1_, &response));
}

TEST_F(CheckCacherTest, TestRefresh) {
  CheckResponse response;
  EXPECT_ERROR_CODE(Code::NOT_FOUND, cacher_->Check(attributes1_, &response));
  EXPECT_OK(cacher_->CacheResponse(attributes1_, pass_response1_));
  EXPECT_OK(cacher_->Check(attributes1_, &response));
  // sleep 0.12 second.
  usleep(120000);

  // First one should be NOT_FOUND for refresh
  EXPECT_ERROR_CODE(Code::NOT_FOUND, cacher_->Check(attributes1_, &response));
  // Second one use cached response.
  EXPECT_OK(cacher_->CacheResponse(attributes1_, pass_response1_));
  EXPECT_OK(cacher_->Check(attributes1_, &response));
  EXPECT_TRUE(MessageDifferencer::Equals(response, pass_response1_));

  EXPECT_OK(cacher_->FlushAll());
  EXPECT_ERROR_CODE(Code::NOT_FOUND, cacher_->Check(attributes1_, &response));
}

TEST_F(CheckCacherTest, TestCacheExpired) {
  CheckResponse response;
  EXPECT_ERROR_CODE(Code::NOT_FOUND, cacher_->Check(attributes1_, &response));

  EXPECT_OK(cacher_->CacheResponse(attributes1_, pass_response1_));
  EXPECT_OK(cacher_->Check(attributes1_, &response));
  EXPECT_TRUE(MessageDifferencer::Equals(response, pass_response1_));

  // sleep 0.22 second to cause cache expired.
  usleep(220000);
  EXPECT_OK(cacher_->Flush());

  // First one should be NOT_FOUND for refresh
  EXPECT_ERROR_CODE(Code::NOT_FOUND, cacher_->Check(attributes1_, &response));
}

}  // namespace mixer_client
}  // namespace istio
