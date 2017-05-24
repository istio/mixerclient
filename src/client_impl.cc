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
#include "src/client_impl.h"
#include "utils/protobuf.h"

using ::istio::mixer::v1::CheckRequest;
using ::istio::mixer::v1::CheckResponse;
using ::istio::mixer::v1::ReportRequest;
using ::istio::mixer::v1::ReportResponse;
using ::istio::mixer::v1::QuotaRequest;
using ::istio::mixer::v1::QuotaResponse;
using ::google::protobuf::util::Status;
using ::google::protobuf::util::error::Code;

namespace istio {
namespace mixer_client {
namespace {
bool IsNetworkError(Status status) {
  return status.error_code() == Code::UNAVAILABLE;
}
}  // namespace

MixerClientImpl::MixerClientImpl(const MixerClientOptions &options)
    : options_(options), converter_({}) {
  deduplication_id_ = 0;
  check_cache_ =
    std::unique_ptr<CheckCache>(new CheckCache(options.check_options));
}

MixerClientImpl::~MixerClientImpl() {}

void MixerClientImpl::Check(const Attributes &attributes, DoneFunc on_done) {
  std::string signature;
  Status status = check_cache_->Check(attributes, &signature);
  if (status.error_code() != Code::NOT_FOUND) {
    on_done(status);
    return;
  }

  CheckRequest request;
  converter_.Convert(attributes, request.mutable_attributes());
  auto response = new CheckResponse;
  options_.check_transport(request, response, [this, signature, response, on_done](const Status &status) {
      delete response;
      if (IsNetworkError(status)) {
	if (options_.check_options.network_fail_open) {
	  on_done(Status::OK);
	} else {
	  on_done(status);
	}
      } else {
	check_cache_->CacheResponse(signature, status);
	on_done(status);
      }
    });
}

void MixerClientImpl::Report(const Attributes &attributes, DoneFunc on_done) {
  auto response = new ReportResponse;
  ReportRequest request;
  converter_.Convert(attributes, request.add_attributes());
  options_.report_transport(request, response,
                            [response, on_done](const Status &status) {
                              on_done(status);
                              delete response;
                            });
}

void MixerClientImpl::Quota(const Attributes &attributes, DoneFunc on_done) {
  QuotaRequest request;
  Attributes filtered_attributes;
  for (const auto &it : attributes.attributes) {
    if (it.first == Attributes::kQuotaName &&
        it.second.type == Attributes::Value::STRING) {
      request.set_quota(it.second.str_v);
    } else if (it.first == Attributes::kQuotaAmount &&
               it.second.type == Attributes::Value::INT64) {
      request.set_amount(it.second.value.int64_v);
    } else {
      filtered_attributes.attributes[it.first] = it.second;
    }
  }
  request.set_deduplication_id(std::to_string(deduplication_id_++));
  // Best offort is only for quota cache.
  request.set_best_effort(false);

  auto response = new QuotaResponse;
  converter_.Convert(filtered_attributes, request.mutable_attributes());
  options_.quota_transport(request, response,
                           [response, on_done](const Status &status) {
                             delete response;
                             if (status.error_code() == Code::UNAVAILABLE) {
                               on_done(Status::OK);
                             } else {
                               on_done(status);
                             }
                           });
}

// Creates a MixerClient object.
std::unique_ptr<MixerClient> CreateMixerClient(
    const MixerClientOptions &options) {
  return std::unique_ptr<MixerClient>(new MixerClientImpl(options));
}

}  // namespace mixer_client
}  // namespace istio
