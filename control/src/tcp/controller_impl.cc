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

#include "controller_impl.h"
#include "request_handler_impl.h"

using ::istio::mixer::v1::config::client::TcpClientConfig;
using ::istio::mixer_client::Statistics;

namespace istio {
namespace mixer_control {
namespace tcp {
namespace {

// The time interval for periodical report.
const int kDefaultReportIntervalMs = 10000;

}  // namespace

ControllerImpl::ControllerImpl(const Options& data) : env_(data.env) {
  client_context_.reset(new ClientContext(data));

  if (data.config.report_interval().seconds() < 0 ||
      data.config.report_interval().nanos() < 0 ||
      (data.config.report_interval().seconds() == 0 &&
       data.config.report_interval().nanos() == 0)) {
    report_interval_ms_ = std::chrono::milliseconds(kDefaultReportIntervalMs);
  } else {
    report_interval_ms_ = std::chrono::milliseconds(
        data.config.report_interval().seconds() * 1000 +
        data.config.report_interval().nanos() / 1000000);
  }
}

std::unique_ptr<RequestHandler> ControllerImpl::CreateRequestHandler() {
  return std::unique_ptr<RequestHandler>(
      new RequestHandlerImpl(client_context_));
}

std::unique_ptr<Controller> Controller::Create(const Options& data) {
  return std::unique_ptr<Controller>(new ControllerImpl(data));
}

void ControllerImpl::GetStatistics(Statistics* stat) const {
  client_context_->GetStatistics(stat);
}

}  // namespace tcp
}  // namespace mixer_control
}  // namespace istio
