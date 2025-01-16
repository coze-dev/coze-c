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

    const coze_conversations_retrieve_request_t req = {
        .api_token = api_token,
        .conversation_id = conversation_id
    };
    coze_conversations_retrieve_response_t resp = {0};
    const coze_error_t err = coze_conversations_retrieve(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error retrieving conversation: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("id: %s\n", resp.data.id);
    printf("created_at: %ld\n", resp.data.created_at);
    printf("last_section_id: %s\n", resp.data.last_section_id);

    coze_free_conversations_retrieve_response(&resp);
    return 0;
}
