cmake_minimum_required (VERSION 2.8)

SET(CMAKE_BUILD_TYPE "Debug")  # 定义编译类型
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") # 定义Debug编译参数

project (server)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory(src SERVER)

include_directories (include)

add_executable(server ${SERVER})

TARGET_LINK_LIBRARIES(server leveldb)
TARGET_LINK_LIBRARIES(server pthread)
TARGET_LINK_LIBRARIES(server jsoncpp)