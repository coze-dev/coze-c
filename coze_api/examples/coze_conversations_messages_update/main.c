#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "coze.h"

int main() {
    const char *api_token = getenv("COZE_API_TOKEN");
    if (!api_token) {
        fprintf(stderr, "Error: COZE_API_TOKEN environment variable not set\n");
        return 1;
    }
    const char *conversation_id = getenv("COZE_CONVERSATION_ID");
    if (!conversation_id) {
        fprintf(stderr, "Error: COZE_CONVERSATION_ID environment variable not set\n");
        return 1;
    }
    const char *message_id = getenv("COZE_MESSAGE_ID");
    if (!message_id) {
        fprintf(stderr, "Error: COZE_MESSAGE_ID environment variable not set\n");
        return 1;
    }

    const coze_conversations_messages_update_request_t req = {
        .api_token = api_token,
        .conversation_id = conversation_id,
        .message_id = message_id,
        .content = "Hello, world!",
        .content_type = COZE_MESSAGE_CONTENT_TYPE_TEXT
    };
    coze_conversations_messages_update_response_t resp = {0};
    const coze_error_t err = coze_conversations_messages_update(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error updating conversation message: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("message.id: %s\n", resp.data.id);
    printf("message.conversation_id: %s\n", resp.data.conversation_id);
    printf("message.bot_id: %s\n", resp.data.bot_id);
    printf("message.chat_id: %s\n", resp.data.chat_id);
    printf("message.role: %s\n", resp.data.role);
    printf("message.content: %s\n", resp.data.content);
    printf("message.content_type: %s\n", resp.data.content_type);
    printf("message.type: %s\n", resp.data.type);
    printf("message.created_at: %ld\n", resp.data.created_at);
    printf("message.updated_at: %ld\n", resp.data.updated_at);

    coze_free_conversations_messages_update_response(&resp);
    return 0;
}
