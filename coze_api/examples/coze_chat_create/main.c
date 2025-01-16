#include <stdio.h>
#include <stdlib.h>
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
    printf("log_id: %s\n", resp.response.logid);
    printf("id: %s\n", resp.data.id);
    printf("created_at: %ld\n", resp.data.created_at);
    printf("conversation_id: %s\n", resp.data.conversation_id);
    printf("bot_id: %s\n", resp.data.bot_id);
    printf("status: %s\n", resp.data.status);

    coze_free_chat_create_response(&resp);
    return 0;
}
