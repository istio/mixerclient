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

#include "referenced.h"

#include "global_dictionary.h"
#include "utils/md5.h"

#include <algorithm>
#include <map>
#include <set>
#include <sstream>

using ::istio::mixer::v1::Attributes;
using ::istio::mixer::v1::Attributes_AttributeValue;
using ::istio::mixer::v1::ReferencedAttributes;

namespace istio {
namespace mixer_client {
namespace {
const char kDelimiter[] = "\0";
const int kDelimiterLength = 1;
const std::string kWordDelimiter = ":";
}  // namespace

// decode dereferences index into str using global and local word lists.
// decode returns false if it is unable to decode.
bool decode(int idx, const std::vector<std::string> &global_words,
            const ReferencedAttributes &reference, std::string *str) {
  if (idx >= 0) {
    if ((unsigned int)idx >= global_words.size()) {
      GOOGLE_LOG(ERROR) << "Global word index is too big: " << idx
                        << " >= " << global_words.size();
      return false;
    }
    *str = global_words[idx];
  } else {
    // per-message index is negative, its format is:
    //    per_message_idx = -(array_idx + 1)
    idx = -idx - 1;
    if (idx >= reference.words_size()) {
      GOOGLE_LOG(ERROR) << "Per message word index is too big: " << idx
                        << " >= " << reference.words_size();
      return false;
    }
    *str = reference.words(idx);
  }

  return true;
}

bool Referenced::Fill(const Attributes &attributes,
                      const ReferencedAttributes &reference) {
  const std::vector<std::string> &global_words = GetGlobalWords();
  const auto &attributes_map = attributes.attributes();

  for (const auto &match : reference.attribute_matches()) {
    AttributeRef ar;
    if (!decode(match.name(), global_words, reference, &ar.name)) {
      return false;
    }

    const auto it = attributes_map.find(ar.name);
    if (it != attributes_map.end()) {
      const Attributes_AttributeValue &value = it->second;
      if (value.value_case() == Attributes_AttributeValue::kStringMapValue) {
        if (!decode(match.map_key(), global_words, reference, &ar.map_key)) {
          return false;
        }
      }
    }

    if (match.condition() == ReferencedAttributes::ABSENCE) {
      absence_keys_.push_back(ar);
    } else if (match.condition() == ReferencedAttributes::EXACT) {
      exact_keys_.push_back(ar);
    } else if (match.condition() == ReferencedAttributes::REGEX) {
      // Don't support REGEX yet, return false to no caching the response.
      GOOGLE_LOG(ERROR) << "Received REGEX in ReferencedAttributes for "
                        << ar.name;
      return false;
    }
  }

  std::sort(absence_keys_.begin(), absence_keys_.end());
  std::sort(exact_keys_.begin(), exact_keys_.end());

  return true;
}

bool Referenced::Signature(const Attributes &attributes,
                           const std::string &extra_key,
                           std::string *signature) const {
  const auto &attributes_map = attributes.attributes();

  for (const AttributeRef &key : absence_keys_) {
    const auto it = attributes_map.find(key.name);
    if (it == attributes_map.end()) {
      continue;
    }

    const Attributes_AttributeValue &value = it->second;
    // if an "absence" key exists for non stringMap, return false for mis-match.
    if (value.value_case() != Attributes_AttributeValue::kStringMapValue) {
      return false;
    }

    const auto &smap = value.string_map_value().entries();
    // check subkey for the string map
    if (smap.find(key.map_key) != smap.end()) {
      return false;
    }
  }

  MD5 hasher;
  for (const AttributeRef &key : exact_keys_) {
    const auto it = attributes_map.find(key.name);
    // if an "exact" attribute not present, return false for mismatch.
    if (it == attributes_map.end()) {
      return false;
    }

    hasher.Update(it->first);
    hasher.Update(kDelimiter, kDelimiterLength);

    const Attributes_AttributeValue &value = it->second;
    switch (value.value_case()) {
      case Attributes_AttributeValue::kStringValue:
        hasher.Update(value.string_value());
        break;
      case Attributes_AttributeValue::kBytesValue:
        hasher.Update(value.bytes_value());
        break;
      case Attributes_AttributeValue::kInt64Value: {
        auto data = value.int64_value();
        hasher.Update(&data, sizeof(data));
      } break;
      case Attributes_AttributeValue::kDoubleValue: {
        auto data = value.double_value();
        hasher.Update(&data, sizeof(data));
      } break;
      case Attributes_AttributeValue::kBoolValue: {
        auto data = value.bool_value();
        hasher.Update(&data, sizeof(data));
      } break;
      case Attributes_AttributeValue::kTimestampValue: {
        auto seconds = value.timestamp_value().seconds();
        auto nanos = value.timestamp_value().nanos();
        hasher.Update(&seconds, sizeof(seconds));
        hasher.Update(kDelimiter, kDelimiterLength);
        hasher.Update(&nanos, sizeof(nanos));
      } break;
      case Attributes_AttributeValue::kDurationValue: {
        auto seconds = value.duration_value().seconds();
        auto nanos = value.duration_value().nanos();
        hasher.Update(&seconds, sizeof(seconds));
        hasher.Update(kDelimiter, kDelimiterLength);
        hasher.Update(&nanos, sizeof(nanos));
      } break;
      case Attributes_AttributeValue::kStringMapValue: {
        const auto &smap = value.string_map_value().entries();
        const auto sub_it = smap.find(key.map_key);
        // exact match of map_key is missing
        if (sub_it == smap.end()) {
          return false;
        }

        hasher.Update(sub_it->first);
        hasher.Update(kDelimiter, kDelimiterLength);
        hasher.Update(sub_it->second);
        hasher.Update(kDelimiter, kDelimiterLength);
      } break;
      case Attributes_AttributeValue::VALUE_NOT_SET:
        break;
    }
    hasher.Update(kDelimiter, kDelimiterLength);
  }
  hasher.Update(extra_key);

  *signature = hasher.Digest();
  return true;
}

void updateHash(MD5 &hasher, const std::vector<AttributeRef> &keys) {
  // keys are already sorted during Fill
  for (const AttributeRef &key : keys) {
    hasher.Update(key.name);
    hasher.Update(kDelimiter, kDelimiterLength);
    if (!key.map_key.empty()) {
      hasher.Update(key.map_key);
      hasher.Update(kDelimiter, kDelimiterLength);
    }
  }
}

std::string Referenced::Hash() const {
  MD5 hasher;

  // keys are sorted during Fill
  updateHash(hasher, absence_keys_);
  hasher.Update(kWordDelimiter);
  updateHash(hasher, exact_keys_);

  return hasher.Digest();
}

std::string Referenced::DebugString() const {
  std::stringstream ss;
  ss << "Absence-keys: ";
  for (const auto &key : absence_keys_) {
    ss << key.name;
    if (!key.map_key.empty()) {
      ss << "[" + key.map_key + "]";
    }
    ss << ", ";
  }
  ss << "Exact-keys: ";
  for (const auto &key : exact_keys_) {
    ss << key.name;
    if (!key.map_key.empty()) {
      ss << "[" + key.map_key + "]";
    }
    ss << ", ";
  }
  return ss.str();
}

}  // namespace mixer_client
}  // namespace istio
