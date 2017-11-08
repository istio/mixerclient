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
    name = "mixer_control_lib",
    srcs = [
        "src/attribute_names.cc",
        "src/attribute_names.h",
        "src/client_context.cc",
        "src/client_context.h",
        "src/controller_impl.cc",
        "src/controller_impl.h",
        "src/http_attributes_builder.cc",
        "src/http_attributes_builder.h",
        "src/http_request_handler_impl.cc",
        "src/http_request_handler_impl.h",
	"src/request_context.h",
	"src/service_context.h",
        "src/tcp_attributes_builder.cc",
        "src/tcp_attributes_builder.h",
        "src/tcp_request_handler_impl.cc",
        "src/tcp_request_handler_impl.h",
        "src/utils/status.cc",
    ],
    hdrs = [
        "include/controller.h",
        "include/http_check_data.h",
        "include/http_report_data.h",
        "include/http_request_handler.h",
        "include/tcp_check_data.h",
        "include/tcp_report_data.h",
        "include/tcp_request_handler.h",
        "include/utils/status.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//:mixer_client_lib",
        "@mixerapi_git//:mixer_client_config_cc_proto",
    ],
)

cc_test(
    name = "http_attributes_builder_test",
    size = "small",
    srcs = [
        "src/mock_http_check_data.h",
        "src/mock_http_report_data.h",
	"src/http_attributes_builder_test.cc",
    ],
    linkstatic = 1,
    deps = [
        ":mixer_control_lib",
        "//external:googletest_main",
    ],
)

cc_test(
    name = "tcp_attributes_builder_test",
    size = "small",
    srcs = [
        "src/mock_tcp_check_data.h",
        "src/mock_tcp_report_data.h",
	"src/tcp_attributes_builder_test.cc",
    ],
    linkstatic = 1,
    deps = [
        ":mixer_control_lib",
        "//external:googletest_main",
    ],
)

cc_test(
    name = "http_request_handler_impl_test",
    size = "small",
    srcs = [
        "src/mock_mixer_client.h",
        "src/mock_http_check_data.h",
        "src/mock_http_report_data.h",
	"src/http_request_handler_impl_test.cc",
    ],
    linkstatic = 1,
    deps = [
        ":mixer_control_lib",
        "//external:googletest_main",
    ],
)

cc_test(
    name = "tcp_request_handler_impl_test",
    size = "small",
    srcs = [
        "src/mock_mixer_client.h",
        "src/mock_tcp_check_data.h",
        "src/mock_tcp_report_data.h",
	"src/tcp_request_handler_impl_test.cc",
    ],
    linkstatic = 1,
    deps = [
        ":mixer_control_lib",
        "//external:googletest_main",
    ],
)
