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

#ifndef MIXERCONTROL_CONTROLLER_H
#define MIXERCONTROL_CONTROLLER_H

#include "http_request_handler.h"
#include "include/client.h"
#include "mixer/v1/config/client/mixer_filter_config.pb.h"
#include "tcp_request_handler.h"

namespace istio {
namespace mixer_control {

// An interface to support Mixer control.
// It takes MixerFitlerConfig and performs tasks to enforce
// mixer control over HTTP and TCP requests.
class Controller {
 public:
  virtual ~Controller() {}

  // A data struct to pass in per-route config.
  struct PerRouteConfig {
    // The per route destination.server name.
    // It will be used to lookup per route config map.
    std::string destination_service;

    // if not NULL, legacy per-route config for 0.2 and before.
    const ::istio::mixer::v1::config::client::MixerControlConfig* legacy_config;
  };

  // Creates a HTTP request handler.
  // The handler supports making Check and Report calls to Mixer.
  // "per_route_config" is for supporting older version of Pilot which
  // set per-route config in route opaque data.
  virtual std::unique_ptr<HttpRequestHandler> CreateHttpRequestHandler(
      const PerRouteConfig& per_route_config) = 0;

  // Creates a TCP request handler.
  // The handler supports making Check and Report calls to Mixer.
  virtual std::unique_ptr<TcpRequestHandler> CreateTcpRequestHandler() = 0;

  // The initial data required by the Controller. It needs:
  // * mixer_config: the mixer client config.
  // * some functions provided by the platform (Envoy)
  struct Options {
    Options(const ::istio::mixer::v1::config::client::MixerFilterConfig& config)
        : mixer_config(config) {}

    // Mixer filter config
    const ::istio::mixer::v1::config::client::MixerFilterConfig& mixer_config;

    // Some plaform functions for mixer client library.
    ::istio::mixer_client::TransportCheckFunc check_transport;
    ::istio::mixer_client::TransportReportFunc report_transport;
    ::istio::mixer_client::TimerCreateFunc timer_create_func;
    ::istio::mixer_client::UUIDGenerateFunc uuid_generate_func;
  };

  // The factory function to create a new instance of the controller.
  static std::unique_ptr<Controller> Create(const Options& options);
};

}  // namespace mixer_control
}  // namespace istio

#endif  // MIXERCONTROL_CONTROLLER_H
