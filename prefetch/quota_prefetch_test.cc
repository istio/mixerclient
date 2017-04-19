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

#include "quota_prefetch.h"
#include "gtest/gtest.h"

#include <list>
#include <utility>

using namespace std::chrono;
using Tick = ::istio::mixer_client::QuotaPrefetch::Tick;
using DoneFunc = ::istio::mixer_client::QuotaPrefetch::DoneFunc;

namespace istio {
namespace mixer_client {
namespace {

// A rate limit server interface.
class RateServer {
 public:
  RateServer(int rate, milliseconds window) : rate_(rate), window_(window) {}
  virtual ~RateServer() {}

  virtual int Alloc(int amount, milliseconds* expire, Tick t) = 0;

 protected:
  int rate_;
  milliseconds window_;
};

// A rolling window rate limit server.
class RollingWindow : public RateServer {
 public:
  RollingWindow(int rate, milliseconds window, Tick t)
      : RateServer(rate, window), expire_time_(t) {}

  int Alloc(int amount, milliseconds* expire, Tick t) override {
    if (t >= expire_time_) {
      avail_ = rate_;
      expire_time_ = t + window_;
    }
    int granted = std::min(amount, avail_);
    avail_ -= granted;
    *expire = duration_cast<milliseconds>(expire_time_ - t);
    return granted;
  }

 private:
  int avail_;
  Tick expire_time_;
};

// A time based, as time advance, available tokens are added
// according to the rate.
class TimeBased : public RateServer {
 public:
  TimeBased(int rate, milliseconds window, Tick t) : RateServer(rate, window) {
    allowance_ = rate_;
    last_check_ = t;
  }

  int Alloc(int amount, milliseconds* expire, Tick t) override {
    milliseconds d = duration_cast<milliseconds>(t - last_check_);
    // Add the token according to the advanced time.
    allowance_ += d.count() * rate_ / window_.count();
    if (allowance_ > rate_) {
      allowance_ = rate_;
    }
    last_check_ = t;
    int granted = std::min(amount, allowance_);
    allowance_ -= granted;
    // always expires at window time.
    *expire = window_;
    return granted;
  }

 private:
  int allowance_;
  Tick last_check_;
};

// Delay a call with a delay.
class Delay {
 public:
  typedef std::function<void(Tick t)> Func;
  Delay() : delay_(0) {}

  void set_delay(milliseconds delay) { delay_ = delay; }

  void Call(Tick t, Func fn) {
    list_.emplace_back(std::make_pair(t + delay_, fn));
  }

  void OnTimer(Tick t) {
    while (!list_.empty() && t >= list_.front().first) {
      list_.front().second(t);
      list_.pop_front();
    }
  }

 private:
  milliseconds delay_;
  std::list<std::pair<Tick, Func>> list_;
};

struct TestData {
  int rate;
  milliseconds window;
  // 3 tests with traffic as: rate - delta, rate and rate + delta.
  int delta;
};

struct TestResult {
  // margin is abs(actual_rate - expected_rate) / expected_rate.
  // margin1 is for the test with traffic as rate - delta
  float margin1;
  // margin1 is for the test with traffic as rate
  float margin2;
  // margin1 is for the test with traffic as rate + delta
  float margin3;
};

// The response delay.
const milliseconds kResponseDelay(200);
// The test duration in team of number of rate windows.
const int kTestDuration = 10;

// Per minute window rate.
const TestData kPerMinuteWindow(
    {.rate = 1200,                   // 1200 rpm, 20 rps, > 10 minPrefetch
     .window = milliseconds(60000),  // per minute window.
     .delta = 100});

const TestData kPerSecondWindow(
    {.rate = 20,                    // 20 rps, > 10 minPrefetch
     .window = milliseconds(1000),  // per second window.
     .delta = 2});

class QuotaPrefetchTest : public ::testing::Test {
 public:
  QuotaPrefetch::TransportFunc GetTransportFunc() {
    return [this](int amount, DoneFunc fn, Tick t) {
      milliseconds expire;
      int granted = rate_server_->Alloc(amount, &expire, t);
      delay_.Call(t,
                  [fn, granted, expire](Tick t1) { fn(granted, expire, t1); });
    };
  }

  // Run the traffic in the window, return passed number.
  int RunSingleClient(QuotaPrefetch& client, int rate, milliseconds window,
                      Tick t) {
    milliseconds d = window / rate;
    int passed = 0;
    for (int i = 0; i < rate; ++i, t += d) {
      if (client.Check(1, t)) {
        ++passed;
      }
      delay_.OnTimer(t);
    }
    return passed;
  }

  int RunTwoClients(QuotaPrefetch& client1, QuotaPrefetch& client2, int rate1,
                    int rate2, milliseconds window, Tick t) {
    int rate = rate1 + rate2;
    milliseconds d = window / rate;
    int passed = 0;
    int n1 = 0;
    int n2 = 0;
    for (int i = 0; i < rate; ++i, t += d) {
      if (float(n1) / rate1 <= float(n2) / rate2) {
        if (client1.Check(1, t)) {
          ++passed;
        }
        ++n1;
      } else {
        if (client2.Check(1, t)) {
          ++passed;
        }
        ++n2;
      }
      delay_.OnTimer(t);
    }
    return passed;
  }

  void RunSingleTest(QuotaPrefetch& client, const TestData& data, int traffic,
                     int expected, float result, Tick t) {
    int passed =
        RunSingleClient(client, traffic, data.window * kTestDuration, t);
    float margin = float(std::abs(passed - expected)) / expected;
    std::cerr << "===RunTest margin: " << margin << ", expected: " << expected
              << ", actual: " << passed << std::endl;
    EXPECT_LE(margin, result);
  }

  // Run 3 single client tests:
  // one below the rate, one exact the rame and one above the rate.
  void TestSingleClient(bool rolling_window, const TestData& data,
                        const TestResult& result) {
    Tick t;
    QuotaPrefetch::Options options;
    auto client = QuotaPrefetch::Create(GetTransportFunc(), options, t);
    if (rolling_window) {
      rate_server_ = std::unique_ptr<RateServer>(
          new RollingWindow(data.rate, data.window, t));
    } else {
      rate_server_ =
          std::unique_ptr<RateServer>(new TimeBased(data.rate, data.window, t));
    }
    delay_.set_delay(kResponseDelay);

    // Send below the limit traffic: rate - delta
    int traffic = (data.rate - data.delta) * kTestDuration;
    int expected = traffic;
    RunSingleTest(*client, data, traffic, expected, result.margin1, t);

    t += data.window * kTestDuration;
    // The traffic is the same as the limit: rate
    traffic = data.rate * kTestDuration;
    expected = traffic;
    RunSingleTest(*client, data, traffic, expected, result.margin2, t);

    t += data.window * kTestDuration;
    // Send higher than the limit traffic: rate + delta
    traffic = (data.rate + data.delta) * kTestDuration;
    expected = data.rate * kTestDuration;
    RunSingleTest(*client, data, traffic, expected, result.margin3, t);
  }

  void RunTwoClientTest(QuotaPrefetch& client1, QuotaPrefetch& client2,
                        const TestData& data, int traffic, int expected,
                        float result, Tick t) {
    // one client is 2/3 and the other is 1/3
    int passed = RunTwoClients(client1, client2, traffic * 2 / 3, traffic / 3,
                               data.window * kTestDuration, t);
    float margin = float(std::abs(passed - expected)) / expected;
    std::cerr << "===RunTest margin: " << margin << ", expected: " << expected
              << ", actual: " << passed << std::endl;
    EXPECT_LE(margin, result);
  }

  // Run 3 single client tests:
  // one below the rate, one exact the rame and one above the rate.
  void TestTwoClients(bool rolling_window, const TestData& data,
                      const TestResult& result) {
    Tick t;
    QuotaPrefetch::Options options;
    auto client1 = QuotaPrefetch::Create(GetTransportFunc(), options, t);
    auto client2 = QuotaPrefetch::Create(GetTransportFunc(), options, t);
    if (rolling_window) {
      rate_server_ = std::unique_ptr<RateServer>(
          new RollingWindow(data.rate, data.window, t));
    } else {
      rate_server_ =
          std::unique_ptr<RateServer>(new TimeBased(data.rate, data.window, t));
    }
    delay_.set_delay(kResponseDelay);

    // Send below the limit traffic: rate - delta
    int traffic = (data.rate - data.delta) * kTestDuration;
    int expected = traffic;
    RunTwoClientTest(*client1, *client2, data, traffic, expected,
                     result.margin1, t);

    t += data.window * kTestDuration;
    // The traffic is the same as the limit: rate
    traffic = data.rate * kTestDuration;
    expected = traffic;
    RunTwoClientTest(*client1, *client2, data, traffic, expected,
                     result.margin2, t);

    t += data.window * kTestDuration;
    // Send higher than the limit traffic: rate + delta
    traffic = (data.rate + data.delta) * kTestDuration;
    expected = data.rate * kTestDuration;
    RunTwoClientTest(*client1, *client2, data, traffic, expected,
                     result.margin3, t);
  }

  std::unique_ptr<RateServer> rate_server_;
  Delay delay_;
};

TEST_F(QuotaPrefetchTest, TestBigRollingWindow) {
  TestSingleClient(true,  // use rolling window,
                   kPerMinuteWindow,
                   {.margin1 = 0.0, .margin2 = 0.006, .margin3 = 0.0015});
}

TEST_F(QuotaPrefetchTest, TestSmallRollingWindow) {
  TestSingleClient(true,  // use rolling window,
                   kPerSecondWindow,
                   {.margin1 = 0.17, .margin2 = 0.08, .margin3 = 0.24});
}

TEST_F(QuotaPrefetchTest, TestBigTimeBased) {
  TestSingleClient(false,  // use time based.
                   kPerMinuteWindow,
                   {.margin1 = 0.0, .margin2 = 0.0, .margin3 = 0.06});
}

TEST_F(QuotaPrefetchTest, TestSmallTimeBased) {
  TestSingleClient(false,  // use time based
                   kPerSecondWindow,
                   {.margin1 = 0.0, .margin2 = 0.005, .margin3 = 0.045});
}

TEST_F(QuotaPrefetchTest, TestTwoClientBigRollingWindow) {
  TestTwoClients(true,  // use rolling window,
                 kPerMinuteWindow,
                 {.margin1 = 0.0001, .margin2 = 0.01, .margin3 = 0.0018});
}

TEST_F(QuotaPrefetchTest, TestTwoClientSmallRollingWindow) {
  TestTwoClients(true,  // use rolling window,
                 kPerSecondWindow,
                 {.margin1 = 0.29, .margin2 = 0.25, .margin3 = 0.25});
}

TEST_F(QuotaPrefetchTest, TestTwoClientBigTimeBased) {
  TestTwoClients(false,  // use time based
                 kPerMinuteWindow,
                 {.margin1 = 0.0001, .margin2 = 0.0, .margin3 = 0.022});
}

TEST_F(QuotaPrefetchTest, TestTwoClientSmallTimeBased) {
  TestTwoClients(false,  // use time based
                 kPerSecondWindow,
                 {.margin1 = 0.04, .margin2 = 0.095, .margin3 = 0.09});
}

}  // namespace
}  // namespace mixer_client
}  // namespace istio
