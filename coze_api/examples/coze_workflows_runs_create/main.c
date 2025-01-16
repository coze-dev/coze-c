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
    const char *workflow_id = getenv("COZE_WORKFLOW_ID");
    if (!workflow_id) {
        fprintf(stderr, "Error: COZE_WORKFLOW_ID environment variable not set\n");
        return 1;
    }

    const coze_workflows_runs_create_request_t req = {
        .api_token = api_token,
        .workflow_id = workflow_id,
        .is_async = false,
    };
    coze_workflows_runs_create_response_t resp = {0};
    const coze_error_t err = coze_workflows_runs_create(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error creating workflow run: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("data: %s\n", resp.data.data);
    printf("debug_url: %s\n", resp.data.debug_url);
    printf("execute_id: %s\n", resp.data.execute_id);

    coze_free_workflows_runs_create_response(&resp);
    return 0;
}
