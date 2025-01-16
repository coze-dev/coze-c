#include <stdio.h>
#include <stdlib.h>
#include "coze.h"

int main() {
    const char *api_token = getenv("COZE_API_TOKEN");
    if (!api_token) {
        fprintf(stderr, "Error: COZE_API_TOKEN environment variable not set\n");
        return 1;
    }

    const coze_workspaces_list_request_t req = {
        .api_token = api_token,
        .page_num = 1,
        .page_size = 20
    };
    coze_workspaces_list_response_t resp = {0};
    const coze_error_t err = coze_workspaces_list(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error getting workspaces list: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("log_id: %s\n", resp.response.logid);
    printf("total_count: %d\n", resp.data.total_count);
    printf("workspace_count: %d\n", resp.data.workspace_count);
    for (int i = 0; i < resp.data.workspace_count; i++) {
        printf("workspace index: %d\n", i + 1);
        printf(" id: %s\n", resp.data.workspaces[i].id);
        printf(" name: %s\n", resp.data.workspaces[i].name);
        printf(" icon_url: %s\n", resp.data.workspaces[i].icon_url);
        printf(" role_type: %s\n", resp.data.workspaces[i].role_type);
        printf(" workspace_type: %s\n", resp.data.workspaces[i].workspace_type);
        printf("\n");
    }
    coze_free_workspaces_list_response(&resp);
    return 0;
}
