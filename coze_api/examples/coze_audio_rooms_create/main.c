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
    const char *bot_id = getenv("COZE_BOT_ID");
    if (!bot_id) {
        fprintf(stderr, "Error: COZE_BOT_ID environment variable not set\n");
        return 1;
    }

    const coze_audio_rooms_create_request_t req = {
        .api_token = api_token,
        .bot_id = bot_id,
    };
    coze_audio_rooms_create_response_t resp = {0};
    const coze_error_t err = coze_audio_rooms_create( &req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error creating audio room: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("room_id: %s\n", resp.data.room_id);
    printf("app_id: %s\n", resp.data.app_id);
    printf("token: %s\n", resp.data.token);
    printf("uid: %s\n", resp.data.uid);

    coze_free_audio_rooms_create_response(&resp);
    return 0;
}
