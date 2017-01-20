/* Copyright 2017 Google Inc. All Rights Reserved.
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
#include "src/attribute_context.h"
#include "google/protobuf/timestamp.pb.h"

using ::google::protobuf::Map;
using ::google::protobuf::Timestamp;

namespace istio {
namespace mixer_client {
namespace {

// TODO: add code to build context to reduce attributes.
// Only check these attributes to build context.
std::set<std::string> kContextSet = {"serviceName", "peerId", "location",
                                     "apiName", "apiVersion"};

// Convert timestamp from time_point to Timestamp
Timestamp CreateTimestamp(std::chrono::system_clock::time_point tp) {
  Timestamp time_stamp;
  long long nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        tp.time_since_epoch())
                        .count();

  time_stamp.set_seconds(nanos / 1000000000);
  time_stamp.set_nanos(nanos % 1000000000);
  return time_stamp;
}

}  // namespace

bool AttributeContext::UpdateDictionary(const Attributes& attributes) {
  int next_index = name_map_.size();
  bool changed = false;
  for (const auto& it : attributes.attributes) {
    const std::string& name = it.first;
    if (name_map_.find(name) == name_map_.end()) {
      changed = true;
      name_map_[name] = ++next_index;
    }
  }
  return changed;
}

void AttributeContext::FillProto(const Attributes& attributes,
                                 ::istio::mixer::v1::Attributes* pb) {
  if (UpdateDictionary(attributes)) {
    Map<int32_t, std::string>* dict = pb->mutable_dictionary();
    for (const auto& it : name_map_) {
      (*dict)[it.second] = it.first;
    }
  }

  // TODO build context use kContextSet to reduce attributes.

  // Fill attributes.
  for (const auto& it : attributes.attributes) {
    const std::string& name = it.first;
    int index = name_map_[name];
    switch (it.second.type) {
      case Attributes::Value::ValueType::STRING:
        (*pb->mutable_string_attributes())[index] = it.second.str_v;
        break;
      case Attributes::Value::ValueType::BYTES:
        (*pb->mutable_bytes_attributes())[index] = it.second.str_v;
        break;
      case Attributes::Value::ValueType::INT64:
        (*pb->mutable_int64_attributes())[index] = it.second.value.int64_v;
        break;
      case Attributes::Value::ValueType::DOUBLE:
        (*pb->mutable_double_attributes())[index] = it.second.value.double_v;
        break;
      case Attributes::Value::ValueType::BOOL:
        (*pb->mutable_bool_attributes())[index] = it.second.value.bool_v;
        break;
      case Attributes::Value::ValueType::TIME:
        (*pb->mutable_timestamp_attributes())[index] =
            CreateTimestamp(it.second.time_v);
        break;
    }
  }
}

}  // namespace mixer_client
}  // namespace istio
