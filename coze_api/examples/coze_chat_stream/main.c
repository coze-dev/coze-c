#include <stdio.h>
#include <stdlib.h>
#include "coze.h"

void handle_chat_event(const coze_chat_event_t *chat_event) {
    printf("event: %s\n", chat_event->event);
    if (chat_event->message) {
        printf("  message: %s %s\n", chat_event->message->content_type, chat_event->message->content);
    }
    if (chat_event->chat) {
        printf("  chat: %s\n", chat_event->chat->status);
    }
    printf("\n");
}

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

    coze_message_t message = {
        .role = COZE_MESSAGE_ROLE_USER,
        .type = COZE_MESSAGE_TYPE_QUESTION,
        .content = "Hello!",
        .content_type = COZE_MESSAGE_CONTENT_TYPE_TEXT,
    };
    const coze_chat_stream_request_t req = {
        .api_token = api_token,
        .bot_id = bot_id,
        .user_id = "rand_user_id",
        .additional_messages_count = 1,
        .additional_messages = (coze_message_t[]){message},
        .auto_save_history = true,

        .on_event = handle_chat_event
    };
    coze_chat_stream_response_t resp = {0};
    const coze_error_t err = coze_chat_stream(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error creating chat stream: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }

    coze_free_chat_stream_response(&resp);
    return 0;
}
