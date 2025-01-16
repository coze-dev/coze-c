cmake_minimum_required(VERSION 3.29)
project(coze_workspaces_list C)

set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} main.c)

# Add cJSON
set(CJSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../external/cJSON)
target_include_directories(${PROJECT_NAME} PRIVATE ${CJSON_DIR})
# add_library(cjson STATIC ${CJSON_DIR}/cJSON.c) # no_need, already in coze_api

# Link both libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    coze_api
    cjson
)

