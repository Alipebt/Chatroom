cmake_minimum_required (VERSION 2.8)

project (client)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory(src CLIENT)

include_directories (include)

add_executable(client ${CLIENT})

TARGET_LINK_LIBRARIES(client pthread)
TARGET_LINK_LIBRARIES(client jsoncpp)