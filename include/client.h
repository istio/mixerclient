/* Copyright 2016 Google Inc. All Rights Reserved.
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

#ifndef MIXERCLIENT_CLIENT_H
#define MIXERCLIENT_CLIENT_H

#include <functional>
#include <memory>
#include <string>

#include "mixer/api/v1/service.pb.h"
#include "google/protobuf/stubs/status.h"
#include "options.h"

namespace google {
namespace mixer_client {

// Defines a function prototype used when an asynchronous transport call
// is completed.
using DoneFunc =
    std::function<void(const ::google::protobuf::util::Status&)>;

// Defines a function prototype to make an asynchronous Check call to
// the mixer server.
using TransportCheckFunc = std::function<void(
    const ::istio::mixer::v1::CheckRequest& request,
    ::istio::mixer::v1::CheckResponse* response, DoneFunc on_done)>;

// Defines a function prototype to make an asynchronous Report call to
// the mixer server.
using TransportReportFunc = std::function<void(
    const ::istio::mixer::v1::ReportRequest& request,
    ::istio::mixer::v1::ReportResponse* response, DoneFunc on_done)>;

// Defines the options to create an instance of MixerClient interface.
struct MixerClientOptions {
  // Default constructor with default values.
  MixerClientOptions() {}

  // Constructor with specified option values.
  MixerClientOptions(const CheckOptions& check_options,
                     const ReportOptions& report_options)
      : check_options(check_options), report_options(report_options) {}

  // Check aggregation options.
  CheckOptions check_options;

  // Report aggregation options.
  ReportOptions report_options;

  // Transport functions are used to send request to mixer server.
  // It can be implemented many ways based on the environments.
  // If not provided, the GRPC transport will be used.
  TransportCheckFunc check_transport;
  TransportReportFunc report_transport;
};

class MixerClient {
 public:
  // Destructor
  virtual ~MixerClient() {}

  // A check call with provided per_request transport function.
  // Only some special platforms may need to use this function.
  // It allows caller to pass in a per_request transport function.
  virtual void Check(const ::istio::mixer::v1::CheckRequest& check_request,
                     ::istio::mixer::v1::CheckResponse* check_response,
                     DoneFunc on_check_done) = 0;

  // A report call with provided per_request transport function.
  // Only some special platforms may need to use this function.
  // It allows callers to pass in a per_request transport function.
  virtual void Report(const ::istio::mixer::v1::ReportResponse& report_request,
                      ::istio::mixer::v1::ReportResponse* report_response,
                      DoneFunc on_report_done) = 0;
};

// Creates a MixerClient object.
std::unique_ptr<MixerClient> CreateMixerClient(MixerClientOptions& options);

}  // namespace mixer_client
}  // namespace google

#endif  // MIXERCLIENT_CLIENT_H
