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

    const coze_audio_voices_list_request_t req = {
        .api_token = api_token,
        .filter_system_voice = false,
        .page_num = 1,
        .page_size = 3,
    };
    coze_audio_voices_list_response_t resp = {0};
    const coze_error_t err = coze_audio_voices_list(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error listing audio voices: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("voices_count: %d\n", resp.data.voices_count);
    printf("has_more: %d\n", resp.data.has_more);
    for (int i = 0; i < resp.data.voices_count; i++) {
        printf(" [voice][%d] preview_audio: %s\n", i, resp.data.voices[i].preview_audio);
        printf(" [voice][%d] language_name: %s\n", i, resp.data.voices[i].language_name);
        printf(" [voice][%d] is_system_voice: %d\n", i, resp.data.voices[i].is_system_voice);
        printf(" [voice][%d] preview_text: %s\n", i, resp.data.voices[i].preview_text);
        printf(" [voice][%d] create_time: %d\n", i, resp.data.voices[i].create_time);
        printf(" [voice][%d] update_time: %d\n", i, resp.data.voices[i].update_time);
        printf(" [voice][%d] name: %s\n", i, resp.data.voices[i].name);
        printf(" [voice][%d] language_code: %s\n", i, resp.data.voices[i].language_code);
        printf(" [voice][%d] voice_id: %s\n", i, resp.data.voices[i].voice_id);
        printf(" [voice][%d] available_training_times: %d\n", i, resp.data.voices[i].available_training_times);
        printf("\n");
    }

    coze_free_audio_voices_list_response(&resp);
    return 0;
}
