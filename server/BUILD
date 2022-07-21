package(default_visibility = ["//visibility:public"])

cc_library(
    name = "net_wrap",
    srcs = ["net_wrap.cc"],
    hdrs = ["net_wrap.h"],
    linkopts =["-lpthread","-ljsoncpp"],
)

cc_library(
    name = "serverClass",
    srcs = ["serverClass.cc"],
    hdrs = ["serverClass.h"],
    deps = ["net_wrap"],
)

cc_binary(
    name = "server",
    srcs = ["privateRoom.cc",
        "main_menu.cc",
        "sign.cc",
        "server.cc",],
    deps = ["net_wrap","serverClass"],
    linkopts =["-lleveldb"],
)