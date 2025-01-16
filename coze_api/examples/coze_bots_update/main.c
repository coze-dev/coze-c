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

    const coze_bots_update_request_t req = {
        .api_token = api_token,
        .bot_id = bot_id,
        .name = "test bot updated",
        .description = "test bot description updated",
    };
    coze_bots_update_response_t resp = {0};
    const coze_error_t err = coze_bots_update(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error updating bot: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);

    coze_free_bots_update_response(&resp);
    return 0;
}
