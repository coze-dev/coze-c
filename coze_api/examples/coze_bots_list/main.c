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

    const coze_bots_list_request_t req = {
        .api_token = api_token,
        .space_id = space_id,
        .page_num = 1,
        .page_size = 20
    };
    coze_bots_list_response_t resp = {0};
    const coze_error_t err = coze_bots_list(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error getting bots list: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("total_count: %d\n", resp.data.total);
    printf("space_bot_count: %d\n", resp.data.space_bot_count);
    for (int i = 0; i < resp.data.space_bot_count; i++) {
        printf("workspace index: %d\n", i + 1);
        printf(" bot_id: %s\n", resp.data.space_bots[i].bot_id);
        printf(" bot_name: %s\n", resp.data.space_bots[i].bot_name);
        printf(" description: %s\n", resp.data.space_bots[i].description);
        printf(" icon_url: %s\n", resp.data.space_bots[i].icon_url);
        printf(" publish_time: %s\n", resp.data.space_bots[i].publish_time);
        printf("\n");
    }
    coze_free_bots_list_response(&resp);
    return 0;
}
