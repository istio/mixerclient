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
#include "src/grpc_transport.h"

namespace istio {
namespace mixer_client {

typedef GrpcStream<::istio::mixer::v1::CheckRequest,
                   ::istio::mixer::v1::CheckResponse>
    CheckGrpcStream;
typedef GrpcStream<::istio::mixer::v1::ReportRequest,
                   ::istio::mixer::v1::ReportResponse>
    ReportGrpcStream;
typedef GrpcStream<::istio::mixer::v1::QuotaRequest,
                   ::istio::mixer::v1::QuotaResponse>
    QuotaGrpcStream;

GrpcTransport::GrpcTransport(const std::string& mixer_server) {
  channel_ = CreateChannel(mixer_server, ::grpc::InsecureChannelCredentials());
  stub_ = ::istio::mixer::v1::Mixer::NewStub(channel_);
}

CheckWriterPtr GrpcTransport::NewStream(CheckReaderRawPtr reader) {
  return CheckWriterPtr(new CheckGrpcStream(
      reader,
      [this](::grpc::ClientContext& context) -> CheckGrpcStream::StreamPtr {
        return stub_->Check(&context);
      }));
}

ReportWriterPtr GrpcTransport::NewStream(ReportReaderRawPtr reader) {
  return ReportWriterPtr(new ReportGrpcStream(
      reader,
      [this](::grpc::ClientContext& context) -> ReportGrpcStream::StreamPtr {
        return stub_->Report(&context);
      }));
}

QuotaWriterPtr GrpcTransport::NewStream(QuotaReaderRawPtr reader) {
  return QuotaWriterPtr(new QuotaGrpcStream(
      reader,
      [this](::grpc::ClientContext& context) -> QuotaGrpcStream::StreamPtr {
        return stub_->Quota(&context);
      }));
}

}  // namespace mixer_client
}  // namespace istio
