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
    const char *chat_id = getenv("COZE_CHAT_ID");
    if (!chat_id) {
        fprintf(stderr, "Error: COZE_CHAT_ID environment variable not set\n");
        return 1;
    }

    const coze_chat_submit_tool_outputs_create_request_t req = {
        .api_token = api_token,
        .conversation_id = conversation_id,
        .chat_id = chat_id,
    };
    coze_chat_submit_tool_outputs_create_response_t resp = {0};
    const coze_error_t err = coze_chat_submit_tool_outputs_create(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error submitting tool outputs: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("chat.id: %s\n", resp.data.id);
    printf("chat.conversation_id: %s\n", resp.data.conversation_id);
    printf("chat.bot_id: %s\n", resp.data.bot_id);

    coze_free_chat_submit_tool_outputs_create_response(&resp);
    return 0;
}
