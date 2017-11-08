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

using ::google::protobuf::util::Status;
using ::google::protobuf::util::error::Code;
using ::istio::mixer::v1::Attributes;
using ::istio::mixer::v1::config::client::MixerFilterConfig;
using ::istio::mixer_client::CancelFunc;
using ::istio::mixer_client::CheckOptions;
using ::istio::mixer_client::DoneFunc;
using ::istio::mixer_client::MixerClientOptions;
using ::istio::mixer_client::ReportOptions;
using ::istio::mixer_client::QuotaOptions;
using ::istio::mixer_client::TransportCheckFunc;

namespace istio {
namespace mixer_control {
namespace {

CheckOptions GetJustCheckOptions(const MixerFilterConfig& config) {
  if (config.disable_check_cache()) {
    return CheckOptions(0);
  }
  return CheckOptions();
}

CheckOptions GetCheckOptions(const MixerFilterConfig& config) {
  auto options = GetJustCheckOptions(config);
  if (config.network_fail_policy() == MixerFilterConfig::FAIL_CLOSE) {
    options.network_fail_open = false;
  }
  return options;
}

QuotaOptions GetQuotaOptions(const MixerFilterConfig& config) {
  if (config.disable_quota_cache()) {
    return QuotaOptions(0, 1000);
  }
  return QuotaOptions();
}

ReportOptions GetReportOptions(const MixerFilterConfig& config) {
  if (config.disable_report_batch()) {
    return ReportOptions(0, 1000);
  }
  return ReportOptions();
}

}  // namespace

ClientContext::ClientContext(const Controller::Options& data)
    : config_(data.mixer_config) {
  MixerClientOptions options(GetCheckOptions(config_),
                             GetReportOptions(config_),
                             GetQuotaOptions(config_));
  options.check_transport = data.check_transport;
  options.report_transport = data.report_transport;
  options.timer_create_func = data.timer_create_func;
  options.uuid_generate_func = data.uuid_generate_func;

  mixer_client_ = ::istio::mixer_client::CreateMixerClient(options);
}

CancelFunc ClientContext::SendCheck(TransportCheckFunc transport,
                                    DoneFunc on_done, RequestContext* request) {
  if (!mixer_client_) {
    request->check_status =
        Status(Code::INVALID_ARGUMENT, "Missing mixer_server cluster");
    on_done(request->check_status);
    return nullptr;
  }

  // Intercept the callback to save check status in request_context
  auto local_on_done = [request, on_done](const Status& status) {
    // save the check status code
    request->check_status = status;
    on_done(status);
  };

  // TODO: add debug message
  // GOOGLE_LOG(INFO) << "Check attributes: " <<
  // request->attributes.DebugString();
  return mixer_client_->Check(request->attributes, transport, local_on_done);
}

void ClientContext::SendReport(const RequestContext& request) {
  if (!mixer_client_) {
    return;
  }
  // TODO: add debug message
  // GOOGLE_LOG(INFO) << "Report attributes: " <<
  // request.attributes.DebugString();
  mixer_client_->Report(request.attributes);
}

}  // namespace mixer_control
}  // namespace istio
