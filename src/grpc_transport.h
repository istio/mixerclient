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
#ifndef MIXERCLIENT_SRC_TRANSPORT_H
#define MIXERCLIENT_SRC_TRANSPORT_H

#include <grpc++/grpc++.h>
#include <thread>

#include "include/transport.h"
#include "mixer/api/v1/service.grpc.pb.h"

namespace istio {
namespace mixer_client {

// A writer implemenation to use gRPC stream
template <class RequestType, class ResponseType>
class GrpcStream : public WriteInterface<RequestType> {
 public:
  typedef std::unique_ptr<
      ::grpc::ClientReaderWriterInterface<RequestType, ResponseType>>
      StreamPtr;
  typedef std::function<StreamPtr(::grpc::ClientContext&)> StreamNewFunc;

  GrpcStream(ReadInterface<ResponseType>* reader, StreamNewFunc create_func)
      : reader_(reader), write_closed_(false) {
    stream_ = create_func(context_);
    worker_thread_ = std::thread([this]() { WorkerThread(); });
  }

  ~GrpcStream() { worker_thread_.join(); }

  void Write(const RequestType& request) {
    if (!stream_->Write(request)) {
      WritesDone();
    }
  }

  void WritesDone() {
    stream_->WritesDone();
    write_closed_ = true;
  }

  bool is_write_closed() const { return write_closed_; }

 private:
  void WorkerThread() {
    ResponseType response;
    while (stream_->Read(&response)) {
      reader_->OnRead(response);
    }
    ::grpc::Status status = stream_->Finish();
    ::google::protobuf::util::Status pb_status(
        ::google::protobuf::util::error::Code(status.error_code()),
        ::google::protobuf::StringPiece(status.error_message()));
    reader_->OnClose(pb_status);
  }

  ::grpc::ClientContext context_;
  StreamPtr stream_;
  std::thread worker_thread_;
  ReadInterface<ResponseType>* reader_;
  bool write_closed_;
};

// A gRPC implementation of Mixer transport
class GrpcTransport : public TransportInterface {
 public:
  GrpcTransport(const std::string& mixer_server);

  CheckWriterPtr NewStream(CheckReaderRawPtr reader);
  ReportWriterPtr NewStream(ReportReaderRawPtr reader);
  QuotaWriterPtr NewStream(QuotaReaderRawPtr reader);

 private:
  std::shared_ptr<::grpc::Channel> channel_;
  std::unique_ptr<::istio::mixer::v1::Mixer::Stub> stub_;
};

}  // namespace mixer_client
}  // namespace istio

#endif  // MIXERCLIENT_SRC_TRANSPORT_H
