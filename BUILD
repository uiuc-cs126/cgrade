cc_library(
    name = "cgrade",
    hdrs = [
        "lib/cgrade.h",
        "lib/catch_reporter_gradescope.h",
        "lib/catch_reporter_gradescope.hpp",
    ],
    srcs = [ ],
    deps = [
        "@catch2//:catch2",
        "@nlohmann//:json",
    ],
    strip_include_prefix = "lib",
    include_prefix = "cgrade",
    visibility = ["//visibility:public"],
)

cc_test(
    name = "cgrade_test",
    srcs = [
        "test/cgrade-test.cc",
    ],
    deps = [
        ":cgrade",
    ],
    args = [
        "--reporter=gradescope",
    ],
)
