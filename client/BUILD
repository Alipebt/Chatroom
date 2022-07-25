# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "net_wrap",
    srcs = ["net_wrap.cc"],
    hdrs = ["net_wrap.h"],
    linkopts=["-lpthread","-ljsoncpp"],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "clientClass",
    srcs = ["clientClass.cc"],
    hdrs = ["clientClass.h"],
    deps = ["net_wrap"],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary
cc_binary(
    name = "client",
    srcs = ["client.cc",
        "friends_menu.cc",
        "privateChat.cc"],
    deps = ["net_wrap","clientClass"],
)
