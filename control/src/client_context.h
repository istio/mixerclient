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

#ifndef MIXERCONTROL_CLIENT_CONTEXT_H
#define MIXERCONTROL_CLIENT_CONTEXT_H

#include "control/include/controller.h"
#include "request_context.h"

namespace istio {
namespace mixer_control {

// The global context object to hold:
// * the mixer client config
// * the mixer client object to call Check/Report with cache.
class ClientContext {
 public:
  ClientContext(const Controller::Options& data);
  // A constructor for unit-test to pass in a mock mixer_client
  ClientContext(
      std::unique_ptr<::istio::mixer_client::MixerClient> mixer_client,
      const ::istio::mixer::v1::config::client::MixerFilterConfig& config)
      : mixer_client_(std::move(mixer_client)), config_(config) {}

  // Use mixer client object to make a Check call.
  ::istio::mixer_client::CancelFunc SendCheck(
      ::istio::mixer_client::TransportCheckFunc transport,
      ::istio::mixer_client::DoneFunc on_done, RequestContext* request);

  // Use mixer client object to make a Report call.
  void SendReport(const RequestContext& request);

  // Retrieve mixer client config.
  const ::istio::mixer::v1::config::client::MixerFilterConfig& config() const {
    return config_;
  }

 private:
  // The mixer client object with check cache and report batch features.
  std::unique_ptr<::istio::mixer_client::MixerClient> mixer_client_;
  // The mixer client config.
  const ::istio::mixer::v1::config::client::MixerFilterConfig& config_;
};

}  // namespace mixer_control
}  // namespace istio

#endif  // MIXERCONTROL_CLIENT_CONTEXT_H
