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

    const coze_bots_retrieve_request_t req = {
        .api_token = api_token,
        .bot_id = bot_id
    };
    coze_bots_retrieve_response_t resp = {0};
    const coze_error_t err = coze_bots_retrieve(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error getting bot info: %d\n", err);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("bot_id: %s\n", resp.data.bot_id);
    printf("name: %s\n", resp.data.name);
    printf("description: %s\n", resp.data.description);
    printf("icon_url: %s\n", resp.data.icon_url);
    printf("create_time: %ld\n", resp.data.create_time);
    printf("update_time: %ld\n", resp.data.update_time);
    printf("version: %s\n", resp.data.version);

    coze_free_bots_retrieve_response(&resp);
    return 0;
}
