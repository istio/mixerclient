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

#ifndef MIXERCONTROL_ATTRIBUTE_NAMES_H
#define MIXERCONTROL_ATTRIBUTE_NAMES_H

#include <string>

namespace istio {
namespace mixer_control {

// Define attribute names
struct AttributeName {
  static const std::string kSourceUser;

  static const std::string kRequestHeaders;
  static const std::string kRequestHost;
  static const std::string kRequestMethod;
  static const std::string kRequestPath;
  static const std::string kRequestReferer;
  static const std::string kRequestScheme;
  static const std::string kRequestSize;
  static const std::string kRequestTime;
  static const std::string kRequestUserAgent;

  static const std::string kResponseCode;
  static const std::string kResponseDuration;
  static const std::string kResponseHeaders;
  static const std::string kResponseSize;
  static const std::string kResponseTime;

  // TCP attributes
  // Downstream tcp connection: source ip/port.
  static const std::string kSourceIp;
  static const std::string kSourcePort;
  // Upstream tcp connection: destionation ip/port.

  static const std::string kDestinationIp;
  static const std::string kDestinationPort;
  static const std::string kConnectionReceviedBytes;
  static const std::string kConnectionReceviedTotalBytes;
  static const std::string kConnectionSendBytes;
  static const std::string kConnectionSendTotalBytes;
  static const std::string kConnectionDuration;

  // Context attributes
  static const std::string kContextProtocol;
  static const std::string kContextTime;

  // Check status code.
  static const std::string kCheckStatusCode;
};

}  // namespace mixer_control
}  // namespace istio

#endif  // MIXERCONTROL_ATTRIBUTE_NAMES_H
