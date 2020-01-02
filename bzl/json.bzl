cc_library(
    name = "json",
    hdrs = glob(
        include = [
            "single_include/nlohmann/**/*.h",
            "single_include/nlohmann/**/*.hpp",
        ],
    ),
    strip_include_prefix = "single_include",
    visibility = ["//visibility:public"],
)
