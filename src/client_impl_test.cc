/* Copyright 2017 Istio Authors. All Rights Reserved.
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

#include "include/client.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::istio::mixer::v1::CheckRequest;
using ::istio::mixer::v1::CheckResponse;
using ::google::protobuf::util::Status;
using ::google::protobuf::util::error::Code;
using ::testing::Invoke;
using ::testing::_;

namespace istio {
namespace mixer_client {
namespace {

const std::string kQuotaName = "RequestCount";

// A mocking class to mock CheckTransport interface.
class MockCheckTransport {
 public:
  MOCK_METHOD3(Check, void(const CheckRequest&, CheckResponse*, DoneFunc));
  TransportCheckFunc GetFunc() {
    return
        [this](const CheckRequest& request, CheckResponse* response,
               DoneFunc on_done) { this->Check(request, response, on_done); };
  }
};

class MixerClientImplTest : public ::testing::Test {
 public:
  MixerClientImplTest() {
    MixerClientOptions options(
        CheckOptions(1 /*entries */), ReportOptions(1, 1000),
        QuotaOptions(1 /* entries */, 600000 /* expiration_ms */));
    options.check_options.network_fail_open = false;
    options.check_options.cache_keys.push_back(
        "key");  // to enable check cache.
    options.check_transport = mock_check_transport_.GetFunc();
    client_ = CreateMixerClient(options);

    request_.attributes[Attributes::kQuotaName] =
        Attributes::StringValue(kQuotaName);
  }

  Attributes request_;
  std::unique_ptr<MixerClient> client_;
  MockCheckTransport mock_check_transport_;
};

TEST_F(MixerClientImplTest, TestSuccessCheck) {
  EXPECT_CALL(mock_check_transport_, Check(_, _, _))
      .WillOnce(Invoke([](const CheckRequest& request, CheckResponse* response,
                          DoneFunc on_done) {
        response->mutable_precondition()->set_valid_use_count(1000);
        on_done(Status::OK);
      }));

  // Remove quota, not to test quota
  request_.attributes.erase(Attributes::kQuotaName);
  Status done_status = Status::UNKNOWN;
  client_->Check(request_,
                 [&done_status](Status status) { done_status = status; });
  EXPECT_TRUE(done_status.ok());

  // Second call should ba cached.
  Status done_status1 = Status::UNKNOWN;
  client_->Check(request_,
                 [&done_status1](Status status) { done_status1 = status; });
  EXPECT_TRUE(done_status1.ok());
}

}  // namespace
}  // namespace mixer_client
}  // namespace istio
