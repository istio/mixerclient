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

#include "attribute_names.h"

namespace istio {
namespace mixer_control {

// Define attribute names
const std::string AttributeName::kSourceUser = "source.user";

const std::string AttributeName::kRequestHeaders = "request.headers";
const std::string AttributeName::kRequestHost = "request.host";
const std::string AttributeName::kRequestMethod = "request.method";
const std::string AttributeName::kRequestPath = "request.path";
const std::string AttributeName::kRequestReferer = "request.referer";
const std::string AttributeName::kRequestScheme = "request.scheme";
const std::string AttributeName::kRequestSize = "request.size";
const std::string AttributeName::kRequestTime = "request.time";
const std::string AttributeName::kRequestUserAgent = "request.useragent";

const std::string AttributeName::kResponseCode = "response.code";
const std::string AttributeName::kResponseDuration = "response.duration";
const std::string AttributeName::kResponseHeaders = "response.headers";
const std::string AttributeName::kResponseSize = "response.size";
const std::string AttributeName::kResponseTime = "response.time";

// TCP attributes
// Downstream tcp connection: source ip/port.
const std::string AttributeName::kSourceIp = "source.ip";
const std::string AttributeName::kSourcePort = "source.port";
// Upstream tcp connection: destionation ip/port.
const std::string AttributeName::kDestinationIp = "destination.ip";
const std::string AttributeName::kDestinationPort = "destination.port";
const std::string AttributeName::kConnectionReceviedBytes =
    "connection.received.bytes";
const std::string AttributeName::kConnectionReceviedTotalBytes =
    "connection.received.bytes_total";
const std::string AttributeName::kConnectionSendBytes = "connection.sent.bytes";
const std::string AttributeName::kConnectionSendTotalBytes =
    "connection.sent.bytes_total";
const std::string AttributeName::kConnectionDuration = "connection.duration";

// Context attributes
const std::string AttributeName::kContextProtocol = "context.protocol";
const std::string AttributeName::kContextTime = "context.time";

// Check status code.
const std::string AttributeName::kCheckStatusCode = "check.status";

}  // namespace mixer_control
}  // namespace istio
