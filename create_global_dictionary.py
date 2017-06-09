#!/usr/bin/python
#
# Copyright 2017 Istio Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import yaml

TOP = r"""/* Copyright 2017 Istio Authors. All Rights Reserved.
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

#ifndef MIXERCLIENT_GLOBAL_DICTIONARY_H
#define MIXERCLIENT_GLOBAL_DICTIONARY_H

#include <string>
#include <vector>

namespace istio {
namespace mixer_client {

static const std::vector<std::string> kGlobalWords = {
"""

BOTTOM = r"""};

}  // namespace mixer_client
}  // namespace istio

#endif  // MIXERCLIENT_GLOBAL_DICTIONARY_H
"""

all_words = ''
for word in yaml.load(open(sys.argv[1])):
    all_words += "    \"%s\",\n" % word

print TOP + all_words + BOTTOM


