#include <stdio.h>
#include <stdlib.h>
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

    const coze_conversations_messages_create_request_t req = {
        .api_token = api_token,
        .conversation_id = conversation_id,
        .role = COZE_MESSAGE_ROLE_USER,
        .content = "Hello, how are you?",
        .content_type = COZE_MESSAGE_CONTENT_TYPE_TEXT
    };
    coze_conversations_messages_create_response_t resp = {0};
    const coze_error_t err = coze_conversations_messages_create(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error creating conversation message: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("id: %s\n", resp.data.id);
    printf("conversation_id: %s\n", resp.data.conversation_id);
    printf("bot_id: %s\n", resp.data.bot_id);
    printf("chat_id: %s\n", resp.data.chat_id);
    printf("role: %s\n", resp.data.role);
    printf("content: %s\n", resp.data.content);
    printf("content_type: %s\n", resp.data.content_type);
    printf("type: %s\n", resp.data.type);
    printf("created_at: %ld\n", resp.data.created_at);
    printf("updated_at: %ld\n", resp.data.updated_at);

    coze_free_conversations_messages_create_response(&resp);
    return 0;
}
