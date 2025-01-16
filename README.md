# Coze C SDK

## Install

```bash
git clone https://github.com/coze-dev/coze-c.git
```

## Examples

run examples:

```bash
./run.sh coze_workspaces_list
```

| Example                                                                                          | Description                  |
|--------------------------------------------------------------------------------------------------|------------------------------|
| [coze_audio_rooms_create](./coze_api/examples/coze_audio_rooms_create)                           | Create an audio room         |
| [coze_audio_voices_list](./coze_api/examples/coze_audio_voices_list)                             | List available audio voices  |
| [coze_bots_create](./coze_api/examples/coze_bots_create)                                         | Create a new bot             |
| [coze_bots_list](./coze_api/examples/coze_bots_list)                                             | List all bots                |
| [coze_bots_publish](./coze_api/examples/coze_bots_publish)                                       | Publish a bot                |
| [coze_bots_retrieve](./coze_api/examples/coze_bots_retrieve)                                     | Get bot details              |
| [coze_bots_update](./coze_api/examples/coze_bots_update)                                         | Update bot settings          |
| [coze_chat_cancel](./coze_api/examples/coze_chat_cancel)                                         | Cancel an ongoing chat       |
| [coze_chat_create](./coze_api/examples/coze_chat_create)                                         | Start a new chat             |
| [coze_chat_messages_list](./coze_api/examples/coze_chat_messages_list)                           | List chat messages           |
| [coze_chat_retrieve](./coze_api/examples/coze_chat_retrieve)                                     | Get chat details             |
| [coze_chat_stream](./coze_api/examples/coze_chat_stream)                                         | Stream chat messages         |
| [coze_chat_submit_tool_outputs_create](./coze_api/examples/coze_chat_submit_tool_outputs_create) | Submit tool outputs for chat |
| [coze_conversations_create](./coze_api/examples/coze_conversations_create)                       | Create a conversation        |
| [coze_conversations_messages_create](./coze_api/examples/coze_conversations_messages_create)     | Create conversation message  |
| [coze_conversations_messages_delete](./coze_api/examples/coze_conversations_messages_delete)     | Delete conversation message  |
| [coze_conversations_messages_list](./coze_api/examples/coze_conversations_messages_list)         | List conversation messages   |
| [coze_conversations_messages_retrieve](./coze_api/examples/coze_conversations_messages_retrieve) | Get conversation message     |
| [coze_conversations_messages_update](./coze_api/examples/coze_conversations_messages_update)     | Update conversation message  |
| [coze_conversations_retrieve](./coze_api/examples/coze_conversations_retrieve)                   | Get conversation details     |
| [coze_files_retrieve](./coze_api/examples/coze_files_retrieve)                                   | Get file details             |
| [coze_files_upload](./coze_api/examples/coze_files_upload)                                       | Upload a file                |
| [coze_workflows_runs_create](./coze_api/examples/coze_workflows_runs_create)                     | Create workflow run          |
| [coze_workflows_runs_stream](./coze_api/examples/coze_workflows_runs_stream)                     | Stream workflow run events   |
| [coze_workspaces_list](./coze_api/examples/coze_workspaces_list)                                 | List workspaces              |

## Usage

```c
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

    const coze_bots_retrieve_request_t req = {.bot_id = bot_id};
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
```
