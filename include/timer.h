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

#ifndef MIXERCLIENT_TIMER_H
#define MIXERCLIENT_TIMER_H

#include <memory>

namespace istio {
namespace mixer_client {

// Represent a timer created by caller's environment.
class Timer {
 public:
  // Delete the timer, stopping it first if needed.
  virtual ~Timer() {}

  // Stop the timer.
  virtual void Stop() = 0;
};

// Defines a function to create a timer calling the function
// with desired interval. The returned object can be used to stop
// the timer.
using TimerCreateFunc = std::function<std::unique_ptr<Timer>(
    int interval_ms, std::function<void()> timer_func)>;

}  // namespace mixer_client
}  // namespace istio

#endif  // MIXERCLIENT_TIMER_H
