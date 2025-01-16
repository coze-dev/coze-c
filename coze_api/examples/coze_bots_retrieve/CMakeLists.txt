cmake_minimum_required(VERSION 3.29)
project(coze_bots_retrieve C)

set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} main.c)

message("[coze_bots_retrieve] CMAKE_CURRENT_SOURCE_DIR is: ${CMAKE_CURRENT_SOURCE_DIR}")

# Add cJSON
set(CJSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../external/cJSON)
message("[coze_bots_retrieve] CJSON_DIR is: ${CJSON_DIR}")
message("[coze_bots_retrieve] PROJECT_NAME is: ${PROJECT_NAME}")
target_include_directories(${PROJECT_NAME} PRIVATE ${CJSON_DIR})
#add_library(cjson STATIC ${CJSON_DIR}/cJSON.c) # no_need, already in coze_api

# Link both libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    coze_api
    cjson
)

