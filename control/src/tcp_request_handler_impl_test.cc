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

#include "client_context.h"
#include "controller_impl.h"
#include "gtest/gtest.h"
#include "mock_mixer_client.h"
#include "mock_tcp_check_data.h"
#include "mock_tcp_report_data.h"

using ::google::protobuf::util::Status;
using ::istio::mixer::v1::config::client::MixerFilterConfig;
using ::istio::mixer_client::MixerClient;

using ::testing::_;

namespace istio {
namespace mixer_control {

class TcpRequestHandlerImplTest : public ::testing::Test {
 public:
  void SetUp() {
    mock_client_ = new ::testing::NiceMock<MockMixerClient>;
    client_context_ = std::make_shared<ClientContext>(
        std::unique_ptr<MixerClient>(mock_client_), filter_config_);
    controller_ =
        std::unique_ptr<Controller>(new ControllerImpl(client_context_));
  }

  std::shared_ptr<ClientContext> client_context_;
  MixerFilterConfig filter_config_;
  ::testing::NiceMock<MockMixerClient>* mock_client_;
  std::unique_ptr<Controller> controller_;
};

TEST_F(TcpRequestHandlerImplTest, TestTcpHandlerDisabledCheck) {
  ::testing::NiceMock<MockTcpCheckData> mock_data;
  EXPECT_CALL(mock_data, GetSourceIpPort(_, _)).Times(1);
  EXPECT_CALL(mock_data, GetSourceUser(_)).Times(1);

  // Check should not be called.
  EXPECT_CALL(*mock_client_, Check(_, _, _)).Times(0);

  filter_config_.set_disable_tcp_check_calls(true);
  auto handler = controller_->CreateTcpRequestHandler();
  handler->Check(&mock_data,
                 [](const Status& status) { EXPECT_TRUE(status.ok()); });
}

TEST_F(TcpRequestHandlerImplTest, TestTcpHandlerCheck) {
  ::testing::NiceMock<MockTcpCheckData> mock_data;
  EXPECT_CALL(mock_data, GetSourceIpPort(_, _)).Times(1);
  EXPECT_CALL(mock_data, GetSourceUser(_)).Times(1);

  // Check should be called.
  EXPECT_CALL(*mock_client_, Check(_, _, _)).Times(1);

  auto handler = controller_->CreateTcpRequestHandler();
  handler->Check(&mock_data, nullptr);
}

TEST_F(TcpRequestHandlerImplTest, TestTcpHandlerReport) {
  ::testing::NiceMock<MockTcpReportData> mock_data;
  EXPECT_CALL(mock_data, GetDestinationIpPort(_, _)).Times(1);
  EXPECT_CALL(mock_data, GetReportInfo(_)).Times(1);

  // Report should be called.
  EXPECT_CALL(*mock_client_, Report(_)).Times(1);

  auto handler = controller_->CreateTcpRequestHandler();
  handler->Report(&mock_data);
}

}  // namespace mixer_control
}  // namespace istio
