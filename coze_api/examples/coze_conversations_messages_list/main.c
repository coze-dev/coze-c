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

    const coze_conversations_messages_list_request_t req = {
        .api_token = api_token,
        .conversation_id = conversation_id,
        .order = COZE_ORDER_DESC
    };
    coze_conversations_messages_list_response_t resp = {0};
    const coze_error_t err = coze_conversations_messages_list(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error listing conversation messages: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("first_id: %s\n", resp.data.first_id);
    printf("last_id: %s\n", resp.data.last_id);
    printf("has_more: %hhd\n", resp.data.has_more);
    for (int i = 0; i < resp.data.messages_count; i++) {
        printf("message[%d].id: %s\n", i, resp.data.messages[i].id);
        printf("message[%d].conversation_id: %s\n", i, resp.data.messages[i].conversation_id);
        printf("message[%d].bot_id: %s\n", i, resp.data.messages[i].bot_id);
        printf("message[%d].chat_id: %s\n", i, resp.data.messages[i].chat_id);
        printf("message[%d].role: %s\n", i, resp.data.messages[i].role);
        printf("message[%d].content: %s\n", i, resp.data.messages[i].content);
        printf("message[%d].content_type: %s\n", i, resp.data.messages[i].content_type);
        printf("message[%d].type: %s\n", i, resp.data.messages[i].type);
        printf("message[%d].created_at: %ld\n", i, resp.data.messages[i].created_at);
        printf("message[%d].updated_at: %ld\n", i, resp.data.messages[i].updated_at);
    }

    coze_free_conversations_messages_list_response(&resp);
    return 0;
}
