#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "coze.h"

// not-ok
int main() {
    const char *api_token = getenv("COZE_API_TOKEN");
    if (!api_token) {
        fprintf(stderr, "Error: COZE_API_TOKEN environment variable not set\n");
        return 1;
    }
    const char *file_id = getenv("COZE_FILE_ID");
    if (!file_id) {
        fprintf(stderr, "Error: COZE_FILE_ID environment variable not set\n");
        return 1;
    }

    const coze_files_retrieve_request_t req = {
        .api_token = api_token,
        .file_id = file_id,
    };
    coze_files_retrieve_response_t resp = {0};
    const coze_error_t err = coze_files_retrieve(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error retrieving file: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("file.id: %s\n", resp.data.id);
    printf("file.file_name: %s\n", resp.data.file_name);
    printf("file.created_at: %d\n", resp.data.created_at);
    printf("file.bytes: %ld\n", resp.data.bytes);

    coze_free_files_retrieve_response(&resp);
    return 0;
}
