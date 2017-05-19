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

#ifndef MIXER_CLIENT_UTILS_PROTO_POOL_H_
#define MIXER_CLIENT_UTILS_PROTO_POOL_H_

#include <list>
#include <mutex>

namespace istio {
namespace mixer_client {

// The protobuf pool to reuse protobuf. Performance tests showed that reusing
// protobuf is faster than allocating a new protobuf.
template <class Type>
class ProtoPool {
 public:
  // Allocates a protobuf. If there is one in the pool, uses it, otherwise
  // creates a new one.
  std::unique_ptr<Type> Alloc();
  // Frees a protobuf. If pool did not reach maximum size, stores it in the
  // pool, otherwise frees it.
  void Free(std::unique_ptr<Type> item);

 private:
  // Protobuf pool to store used protobufs.
  std::list<std::unique_ptr<Type>> pool_;
  // Mutex to protect the protobuf pool.
  std::mutex mutex_;
};

template <class Type>
std::unique_ptr<Type> ProtoPool<Type>::Alloc() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!pool_.empty()) {
    auto item = std::move(pool_.front());
    pool_.pop_front();
    item->Clear();
    return item;
  } else {
    return std::unique_ptr<Type>(new Type);
  }
}

template <class Type>
void ProtoPool<Type>::Free(std::unique_ptr<Type> item) {
  std::lock_guard<std::mutex> lock(mutex_);
  pool_.push_back(std::move(item));
}

}  // namespace mixer_client
}  // namespace istio

#endif  // MIXER_CLIENT_CLIENT_UTILS_PROTO_POOL_H_
