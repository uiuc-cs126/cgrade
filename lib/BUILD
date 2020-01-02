# Header only library dependencies
cc_library(
    name = "cgrade",
    hdrs = [
        "cgrade.h",
    ],
    srcs = [
        "catch_reporter_gradescope.h",
        "catch_reporter_gradescope.hpp",
    ],
    deps = [
        "@catch2//:catch2",
        "@nlohmann//:json",
    ],
    visibility = ["//visibility:public"],
)
