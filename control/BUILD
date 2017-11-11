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

licenses(["notice"])

cc_library(
    name = "common_lib",
    srcs = [
        "src/attribute_names.cc",
        "src/client_context_base.cc",
    ],
    hdrs = [
        "src/attribute_names.h",
        "src/client_context_base.h",
        "src/request_context.h",
    ],
    deps = [
        "//:mixer_client_lib",
        "@mixerapi_git//:mixer_client_config_cc_proto",
    ],
)

cc_library(
    name = "http_control_lib",
    srcs = [
        "src/http/client_context.h",
        "src/http/controller_impl.cc",
        "src/http/controller_impl.h",
        "src/http/attributes_builder.cc",
        "src/http/attributes_builder.h",
        "src/http/request_handler_impl.cc",
        "src/http/request_handler_impl.h",
        "src/http/service_context.h",
        "src/utils/status.cc",
    ],
    hdrs = [
        "include/http/controller.h",
        "include/http/check_data.h",
        "include/http/report_data.h",
        "include/http/request_handler.h",
        "include/utils/status.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":common_lib",
    ],
)

cc_library(
    name = "tcp_control_lib",
    srcs = [
        "src/tcp/client_context.h",
        "src/tcp/controller_impl.cc",
        "src/tcp/controller_impl.h",
        "src/tcp/attributes_builder.cc",
        "src/tcp/attributes_builder.h",
        "src/tcp/request_handler_impl.cc",
        "src/tcp/request_handler_impl.h",
    ],
    hdrs = [
        "include/tcp/controller.h",
        "include/tcp/check_data.h",
        "include/tcp/report_data.h",
        "include/tcp/request_handler.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":common_lib",
    ],
)

cc_test(
    name = "http_attributes_builder_test",
    size = "small",
    srcs = [
        "src/http/mock_check_data.h",
        "src/http/mock_report_data.h",
        "src/http/attributes_builder_test.cc",
    ],
    linkstatic = 1,
    deps = [
        ":http_control_lib",
        "//external:googletest_main",
    ],
)

cc_test(
    name = "http_request_handler_impl_test",
    size = "small",
    srcs = [
        "src/mock_mixer_client.h",
        "src/http/mock_check_data.h",
        "src/http/mock_report_data.h",
        "src/http/request_handler_impl_test.cc",
    ],
    linkstatic = 1,
    deps = [
        ":http_control_lib",
        "//external:googletest_main",
    ],
)

cc_test(
    name = "tcp_attributes_builder_test",
    size = "small",
    srcs = [
        "src/tcp/mock_check_data.h",
        "src/tcp/mock_report_data.h",
        "src/tcp/attributes_builder_test.cc",
    ],
    linkstatic = 1,
    deps = [
        ":tcp_control_lib",
        "//external:googletest_main",
    ],
)

cc_test(
    name = "tcp_request_handler_impl_test",
    size = "small",
    srcs = [
        "src/mock_mixer_client.h",
        "src/tcp/mock_check_data.h",
        "src/tcp/mock_report_data.h",
        "src/tcp/request_handler_impl_test.cc",
    ],
    linkstatic = 1,
    deps = [
        ":tcp_control_lib",
        "//external:googletest_main",
    ],
)
