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

#ifndef MIXERCONTROL_TCP_CLIENT_CONTEXT_H
#define MIXERCONTROL_TCP_CLIENT_CONTEXT_H

#include "control/include/tcp/controller.h"
#include "control/src/client_context_base.h"
#include "control/src/request_context.h"
#include "quota/include/config_parser.h"

namespace istio {
namespace mixer_control {
namespace tcp {
namespace {

// The time interval for periodical report.
const int kDefaultReportIntervalMs = 10000;

}  // namespace

// The global context object to hold:
// * the tcp client config
// * the mixer client object to call Check/Report with cache.
class ClientContext : public ClientContextBase {
 public:
  ClientContext(const Controller::Options& data)
      : ClientContextBase(data.config.transport(), data.env),
        config_(data.config),
        env_(data.env) {
    BuildQuotaParser();
    if (config_.report_interval().seconds() < 0 ||
        config_.report_interval().nanos() < 0 ||
        (config_.report_interval().seconds() == 0 &&
         config_.report_interval().nanos() == 0)) {
      report_interval_ms_ = kDefaultReportIntervalMs;
    } else {
      report_interval_ms_ = config_.report_interval().seconds() * 1000 +
                            config_.report_interval().nanos() / 1000000;
    }
  }

  // A constructor for unit-test to pass in a mock mixer_client
  ClientContext(
      std::unique_ptr<::istio::mixer_client::MixerClient> mixer_client,
      const ::istio::mixer::v1::config::client::TcpClientConfig& config)
      : ClientContextBase(std::move(mixer_client)),
        config_(config),
        env_{},
        report_interval_ms_(kDefaultReportIntervalMs) {
    BuildQuotaParser();
  }

  // Add static mixer attributes.
  void AddStaticAttributes(RequestContext* request) const {
    if (config_.has_mixer_attributes()) {
      request->attributes.MergeFrom(config_.mixer_attributes());
    }
  }

  // Add quota requirements from quota configs.
  void AddQuotas(RequestContext* request) const {
    if (quota_parser_) {
      quota_parser_->GetRequirements(request->attributes, &request->quotas);
    }
  }

  bool enable_mixer_check() const { return !config_.disable_check_calls(); }
  bool enable_mixer_report() const { return !config_.disable_report_calls(); }
  int report_interval_ms() const { return report_interval_ms_; }

  const ::istio::mixer_client::Environment& environment() const { return env_; }

 private:
  // If there is quota config, build quota parser.
  void BuildQuotaParser() {
    if (config_.has_connection_quota_spec()) {
      quota_parser_ =
          ::istio::quota::ConfigParser::Create(config_.connection_quota_spec());
    }
  }
  // The mixer client config.
  const ::istio::mixer::v1::config::client::TcpClientConfig& config_;

  // The quota parser.
  std::unique_ptr<::istio::quota::ConfigParser> quota_parser_;

  const ::istio::mixer_client::Environment& env_;

  // Time interval in milliseconds for sending periodical delta reports.
  int report_interval_ms_;
};

}  // namespace tcp
}  // namespace mixer_control
}  // namespace istio

#endif  // MIXERCONTROL_TCP_CLIENT_CONTEXT_H
