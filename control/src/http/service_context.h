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

#ifndef MIXERCONTROL_HTTP_SERVICE_CONTEXT_H
#define MIXERCONTROL_HTTP_SERVICE_CONTEXT_H

#include "api_spec/include/http_api_spec_parser.h"
#include "client_context.h"
#include "google/protobuf/stubs/status.h"
#include "mixer/v1/attributes.pb.h"
#include "quota/include/config_parser.h"

namespace istio {
namespace mixer_control {
namespace http {

// The context to hold service config for both HTTP and TCP.
class ServiceContext {
 public:
  ServiceContext(
      std::shared_ptr<ClientContext> client_context,
      const ::istio::mixer::v1::config::client::ServiceConfig& config);

  std::shared_ptr<ClientContext> client_context() const {
    return client_context_;
  }

  // Add static mixer attributes.
  void AddStaticAttributes(RequestContext* request) const;

  // Add api attributes from api_spec.
  void AddApiAttributes(CheckData* check_data, RequestContext* request) const;

  // Add quota requirements from quota configs.
  void AddQuotas(RequestContext* request) const;

  bool enable_mixer_check() const {
    return !service_config_.disable_check_calls();
  }
  bool enable_mixer_report() const {
    return !service_config_.disable_report_calls();
  }

 private:
  // The client context object.
  std::shared_ptr<ClientContext> client_context_;

  // Concatenated api_spec_
  ::istio::mixer::v1::config::client::HTTPAPISpec api_spec_;
  // Api spec parser to generate api attributes and api_key
  std::unique_ptr<::istio::api_spec::HttpApiSpecParser> api_spec_parser_;

  // The quota parsers for each quota config.
  std::vector<std::unique_ptr<::istio::quota::ConfigParser>> quota_parsers_;

  // The service config.
  ::istio::mixer::v1::config::client::ServiceConfig service_config_;
};

}  // namespace http
}  // namespace mixer_control
}  // namespace istio

#endif  // MIXERCONTROL_HTTP_SERVICE_CONTEXT_H
