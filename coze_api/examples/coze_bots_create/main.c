#include <stdio.h>
#include <stdlib.h>
#include "coze.h"

int main() {
    const char *api_token = getenv("COZE_API_TOKEN");
    if (!api_token) {
        fprintf(stderr, "Error: COZE_API_TOKEN environment variable not set\n");
        return 1;
    }
    const char *space_id = getenv("COZE_WOEKSPACE_ID");
    if (!space_id) {
        fprintf(stderr, "Error: COZE_WOEKSPACE_ID environment variable not set\n");
        return 1;
    }

    const coze_bots_create_request_t req = {
        .api_token = api_token,
        .space_id = space_id,
        .name = "test bot",
        .description = "test bot description",
    };
    coze_bots_create_response_t resp = {0};
    coze_error_t err = coze_bots_create(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error creating bot: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("bot_id: %s\n", resp.data.bot_id);

    coze_free_bots_create_response(&resp);
    return 0;
}
