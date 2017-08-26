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
#include "utils/md5.h"

namespace istio {
namespace mixer_client {
namespace {
const char kDelimiter[] = "\0";
const int kDelimiterLength = 1;
}  // namespace

  bool Referenced::Fill(const ::istio::mixer::v1::ReferencedAttributes& reference) {
    const std::vector<std::string>& global_words = GetGlobalWords();

    for (const auto& match : reference.attribute_matches()) {
      std::string name;
      int idx = match.name();
      if (idx >= 0) {
	if (idx >= global_words.size()) {
	  GOOGLE_LOG(ERROR) << "Global word index is too big: " << idx
			    << " vs " << global_words.size();
	  return false;
	}
	name = global_words[idx];
      } else {
	idx = -idx - 1;
	if (idx >= reference.words_size()) {
	  GOOGLE_LOG(ERROR) << "Per message word index is too big: " << idx
			    << " vs " << reference.words_size();
	  return false;
	}
	name = reference.words(idx);
      }

      if (match.condition() == ABSENCE) {
	absence_keys.insert(name);
      } else if (match.condition() == EXACT) {
	exact_keys.insert(name);
      }
    }

    return true;
  }


  bool Referenced::ignature(const Attributes& attributes, std::string* signature) const {
    for (const auto& key : absence_keys) {
      if (attributes.attributes.find(key) != attributes.attributes.end()) {
	return false;
      }
    }

    MD5 hasher;
    for (const auto& key : exact_keys) {
      const auto it = attributes.attributes.find(key);
      if (it == attributes.attributes.end()) {
	return false;
      }

      hasher.Update(it->first);
      hasher.Update(kDelimiter, kDelimiterLength);
      switch (it->second.type) {
      case Attributes::Value::ValueType::STRING:
        hasher.Update(it->second.str_v);
        break;
      case Attributes::Value::ValueType::BYTES:
        hasher.Update(it->second.str_v);
        break;
      case Attributes::Value::ValueType::INT64:
        hasher.Update(&it->second.value.int64_v,
                      sizeof(it->second.value.int64_v));
        break;
      case Attributes::Value::ValueType::DOUBLE:
        hasher.Update(&it->second.value.double_v,
                      sizeof(it->second.value.double_v));
        break;
      case Attributes::Value::ValueType::BOOL:
        hasher.Update(&it->second.value.bool_v,
                      sizeof(it->second.value.bool_v));
        break;
      case Attributes::Value::ValueType::TIME:
        hasher.Update(&it->second.time_v,
                      sizeof(it->second.time_v));
        break;
      case Attributes::Value::ValueType::DURATION:
        hasher.Update(&it->second.duration_nanos_v,
                      sizeof(it->second.duration_nanos_v));
        break;
      case Attributes::Value::ValueType::STRING_MAP:
        for (const auto& sub_it : it->second.string_map_v) {
            hasher.Update(sub_it.first);
            hasher.Update(kDelimiter, kDelimiterLength);
            hasher.Update(sub_it.second);
            hasher.Update(kDelimiter, kDelimiterLength);
        }
        break;
      }
      hasher.Update(kDelimiter, kDelimiterLength);
    }

    *signature = hasher.Digest();
    return true;
  }

  std::string Referenced::Hash() const {
    MD5 hasher;
    for (const auto& key : absence_keys) {
      hasher.Update(key);
      hasher.Update(kDelimiter, kDelimiterLength);
    }
    hasher.Update("====");
    for (const auto& key : exact_keys) {
      hasher.Update(kDelimiter, kDelimiterLength);
    }
  
    return hasher.Digest();
  }
  
}  // namespace mixer_client
}  // namespace istio
