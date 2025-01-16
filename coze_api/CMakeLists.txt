cmake_minimum_required(VERSION 3.29)
project(coze_api C)

set(CMAKE_C_STANDARD 99)

# 查找 CURL 包
find_package(CURL REQUIRED)

# add coze_api
add_library(${PROJECT_NAME} STATIC
        include/coze.h
        src/coze.c
        examples/coze_conversations_create/main.c
        examples/coze_conversations_retrieve/main.c
        examples/coze_conversations_messages_create/main.c
        examples/coze_conversations_messages_list/main.c
        examples/coze_conversations_messages_retrieve/main.c
        examples/coze_conversations_messages_delete/main.c
        examples/coze_audio_voices_list/main.c
        examples/coze_auth_web_oauth_get_oauth_url/main.c
        examples/coze_auth_web_oauth_get_access_token/main.c
        examples/coze_audio_rooms_create/main.c
        examples/coze_bots_create/main.c
        examples/coze_bots_list/main.c
        examples/coze_bots_publish/main.c
        examples/coze_bots_retrieve/main.c
        examples/coze_bots_update/main.c
        examples/coze_chat_cancel/main.c
        examples/coze_chat_create/main.c
        examples/coze_chat_messages_list/main.c
        examples/coze_chat_retrieve/main.c
        examples/coze_chat_stream/main.c
        examples/coze_chat_submit_tool_outputs_create/main.c
        examples/coze_conversations_messages_update/main.c
        examples/coze_files_retrieve/main.c
        examples/coze_files_upload/main.c
        examples/coze_workflows_runs_create/main.c
        examples/coze_workflows_runs_stream/main.c
        examples/coze_workspaces_list/main.c
)

# Add cJSON
set(CJSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/cJSON)
target_include_directories(${PROJECT_NAME} PRIVATE ${CJSON_DIR})
add_library(cjson
        STATIC
        ${CJSON_DIR}/cJSON.c
)

target_include_directories(${PROJECT_NAME}
        PUBLIC include
)

# 链接 CURL 和 cJSON
target_link_libraries(${PROJECT_NAME}
        PRIVATE CURL::libcurl
)

add_subdirectory(examples)