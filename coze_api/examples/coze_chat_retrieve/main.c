#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coze.h"

int main() {
    const char *api_token = getenv("COZE_API_TOKEN");
    if (!api_token) {
        fprintf(stderr, "Error: COZE_API_TOKEN environment variable not set\n");
        return 1;
    }
    const char *bot_id = getenv("COZE_BOT_ID");
    if (!bot_id) {
        fprintf(stderr, "Error: COZE_BOT_ID environment variable not set\n");
        return 1;
    }

    coze_message_t message = {
        .role = COZE_MESSAGE_ROLE_USER,
        .type = COZE_MESSAGE_TYPE_QUESTION,
        .content = "Hello!",
        .content_type = COZE_MESSAGE_CONTENT_TYPE_TEXT
    };
    const coze_chat_create_request_t req = {
        .api_token = api_token,
        .bot_id = bot_id,
        .user_id = "rand_user_id",
        .additional_messages_count = 1,
        .additional_messages = (coze_message_t[]){message}
    };
    coze_chat_create_response_t resp = {0};
    const coze_error_t err = coze_chat_create(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error creating chat: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("[chat.create] log_id: %s\n", resp.response.logid);
    printf("[chat.create] id: %s\n", resp.data.id);
    printf("[chat.create] created_at: %ld\n", resp.data.created_at);
    printf("[chat.create] conversation_id: %s\n", resp.data.conversation_id);
    printf("[chat.create] bot_id: %s\n", resp.data.bot_id);
    printf("[chat.create] status: %s\n", resp.data.status);
    printf("\n");

    const char *conversation_id = strdup(resp.data.conversation_id);
    const char *chat_id = strdup(resp.data.id);
    coze_free_chat_create_response(&resp);

    const coze_chat_retrieve_request_t retrieve_req = {
        .api_token = api_token,
        .conversation_id = conversation_id,
        .chat_id = chat_id
    };
    coze_chat_retrieve_response_t retrieve_resp = {0};
    const coze_error_t retrieve_err = coze_chat_retrieve(&retrieve_req, &retrieve_resp);
    if (retrieve_err != COZE_OK) {
        fprintf(stderr, "Error retrieving chat: %d, code: %d, msg: %s\n", retrieve_err, retrieve_resp.code,
                retrieve_resp.msg);
        return 1;
    }
    printf("[chat.retrieve] log_id: %s\n", retrieve_resp.response.logid);
    printf("[chat.retrieve] id: %s\n", retrieve_resp.data.id);
    printf("[chat.retrieve] created_at: %ld\n", retrieve_resp.data.created_at);
    printf("[chat.retrieve] conversation_id: %s\n", retrieve_resp.data.conversation_id);
    printf("[chat.retrieve] bot_id: %s\n", retrieve_resp.data.bot_id);
    printf("[chat.retrieve] status: %s\n", retrieve_resp.data.status);
    coze_free_chat_retrieve_response(&retrieve_resp);

    return 0;
}
