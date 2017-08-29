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

#include "src/referenced.h"

#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

using ::google::protobuf::TextFormat;

namespace istio {
namespace mixer_client {
namespace {

const char kReferencedText1[] = R"(
words: "bool-key"
words: "bytes-key"
attribute_matches {
  name: 9,
  condition: ABSENCE,
}
attribute_matches {
  name: -1,
  condition: EXACT,
})";

TEST(ReferencedTest, FillTest) {
  ::istio::mixer::v1::ReferencedAttributes referenced_pb;
  ASSERT_TRUE(TextFormat::ParseFromString(kReferencedText1, &referenced_pb));

  Referenced referenced;
  EXPECT_TRUE(referenced.Fill(referenced_pb));

  EXPECT_EQ(referenced.DebugString(), "");

  EXPECT_EQ(referenced.Hash(), "");
}

}  // namespace
}  // namespace mixer_client
}  // namespace istio
