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

#ifndef MIXERCONTROL_HTTP_CONTROLLER_IMPL_H
#define MIXERCONTROL_HTTP_CONTROLLER_IMPL_H

#include <memory>
#include <unordered_map>

#include "client_context.h"
#include "control/include/http/controller.h"
#include "service_context.h"

namespace istio {
namespace mixer_control {
namespace http {

// The class to implement Controller interface.
class ControllerImpl : public Controller {
 public:
  ControllerImpl(const Controller::Options& data);
  // A constructor for unit-test to pass in a mock client_context
  ControllerImpl(std::shared_ptr<ClientContext> client_context)
      : client_context_(client_context) {}

  // Creates a HTTP request handler
  std::unique_ptr<RequestHandler> CreateRequestHandler(
      const PerRouteConfig& per_route_config) override;

 private:
  // Create service config context for HTTP.
  std::shared_ptr<ServiceContext> GetServiceContext(
      const PerRouteConfig& per_route_config);

  // The client context object to hold client config and client cache.
  std::shared_ptr<ClientContext> client_context_;

  // The map to cache service context. key is destination.service
  std::unordered_map<std::string, std::shared_ptr<ServiceContext>>
      service_context_map_;
};

}  // namespace http
}  // namespace mixer_control
}  // namespace istio

#endif  // MIXERCONTROL_HTTP_CONTROLLER_IMPL_H
