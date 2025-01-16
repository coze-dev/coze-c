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

    const coze_bots_publish_request_t req = {
        .api_token = api_token,
        .bot_id = bot_id,
        .connector_ids = (const char *[]){"API"},
        .connector_ids_count = 1,
    };
    coze_bots_publish_response_t resp = {0};
    const coze_error_t err = coze_bots_publish(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error publishing bot: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("bot_id: %s\n", resp.data.bot_id);
    printf("version: %s\n", resp.data.version);

    coze_free_bots_publish_response(&resp);
    return 0;
}
