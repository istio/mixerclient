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
#include "src/client_impl.h"
#include "mixer/api/v1/service.pb.h"

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

MixerClientImpl::MixerClientImpl(MixerClientOptions &options) {
  check_transport_ = options.check_transport;
  report_transport_ = options.report_transport;
  quota_transport_ = options.quota_transport;
}

MixerClientImpl::~MixerClientImpl() {}

void MixerClientImpl::Check(const CheckRequest &check_request,
                            CheckResponse *check_response,
                            DoneFunc on_check_done) {
  if (check_transport_ == NULL) {
    on_check_done(Status(Code::INVALID_ARGUMENT, "transport is NULL."));
    return;
  }

  check_transport_(
      check_request, check_response, [on_check_done](Status status) {
        if (!status.ok()) {
          GOOGLE_LOG(ERROR)
              << "Failed in Check call: " << status.error_message();
        } else {
          on_check_done(status);
        }
      });
}

void MixerClientImpl::Report(const ReportRequest &report_request,
                             ReportResponse *report_response,
                             DoneFunc on_report_done) {
  if (report_transport_ == NULL) {
    on_report_done(Status(Code::INVALID_ARGUMENT, "transport is NULL."));
    return;
  }

  report_transport_(report_request, report_response, on_report_done);
}

void MixerClientImpl::Quota(const QuotaRequest &quota_request,
                            QuotaResponse *quota_response,
                            DoneFunc on_quota_done) {
  if (quota_transport_ == NULL) {
    on_quota_done(Status(Code::INVALID_ARGUMENT, "transport is NULL."));
    return;
  }

  quota_transport_(quota_request, quota_response, on_quota_done);
}

// Creates a MixerClient object.
std::unique_ptr<MixerClient> CreateMixerClient(MixerClientOptions &options) {
  return std::unique_ptr<MixerClient>(new MixerClientImpl(options));
}

}  // namespace mixer_client
}  // namespace istio
