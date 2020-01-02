cc_library(
    name = "catch2",
    hdrs = glob(
        include = [
            "single_include/catch2/**/*.h",
            "single_include/catch2/**/*.hpp",
        ],
    ),
    strip_include_prefix = "single_include",
    visibility = ["//visibility:public"],
)
