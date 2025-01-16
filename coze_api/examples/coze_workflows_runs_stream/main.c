#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "coze.h"

void handle_workflow_event(const coze_workflow_event_t *event) {
    printf("event: %s\n", event->event);
    if (event->message) {
        printf("  message: %s\n", event->message->content);
    }
    if (event->interrupt) {
        printf("  interrupt: %s\n", event->interrupt->node_title);
    }
    if (event->error) {
        printf("  error: %s\n", event->error->error_message);
    }
    printf("\n");
}

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

    const coze_workflows_runs_stream_request_t req = {
        .api_token = api_token,
        .workflow_id = workflow_id,

        .on_event = handle_workflow_event
    };
    coze_workflows_runs_stream_response_t resp = {0};
    const coze_error_t err = coze_workflows_runs_stream(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error creating workflow run stream: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }

    coze_free_workflows_runs_stream_response(&resp);
    return 0;
}
