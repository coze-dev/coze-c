cmake_minimum_required(VERSION 3.29)
project(examples C)

set(CMAKE_C_STANDARD 99)

# auth
add_subdirectory(coze_auth_web_oauth_get_oauth_url)
add_subdirectory(coze_auth_web_oauth_get_access_token)
add_subdirectory(coze_auth_web_oauth_refresh_access_token)
# # workspaces
# add_subdirectory(coze_workspaces_list)
# # bots
# add_subdirectory(coze_bots_create)
# add_subdirectory(coze_bots_update)
# add_subdirectory(coze_bots_publish)
# add_subdirectory(coze_bots_list)
# add_subdirectory(coze_bots_retrieve)
# # conversations
# add_subdirectory(coze_conversations_create)
# add_subdirectory(coze_conversations_retrieve)
# # conversations.messages
# add_subdirectory(coze_conversations_messages_create)
# add_subdirectory(coze_conversations_messages_list)
# add_subdirectory(coze_conversations_messages_retrieve)
# add_subdirectory(coze_conversations_messages_update)
# add_subdirectory(coze_conversations_messages_delete)
# # chat
# add_subdirectory(coze_chat_create)
# add_subdirectory(coze_chat_stream)
# add_subdirectory(coze_chat_retrieve)
# add_subdirectory(coze_chat_messages_list)
# add_subdirectory(coze_chat_submit_tool_outputs_create)
# add_subdirectory(coze_chat_cancel)
# # workflows.runs
# add_subdirectory(coze_workflows_runs_create)
# add_subdirectory(coze_workflows_runs_stream)
# # files
# add_subdirectory(coze_files_upload)
# add_subdirectory(coze_files_retrieve)
# # audio
# add_subdirectory(coze_audio_voices_list)
# add_subdirectory(coze_audio_rooms_create)
# # other
# add_subdirectory(json_example)