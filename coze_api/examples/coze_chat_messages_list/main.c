#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

    coze_message_t message = {
        .role = COZE_MESSAGE_ROLE_USER,
        .type = COZE_MESSAGE_TYPE_QUESTION, 
        .content = "Hello!",
        .content_type = COZE_MESSAGE_CONTENT_TYPE_TEXT
    };
    const coze_chat_create_request_t req = {
        .api_token = api_token,
        .bot_id = bot_id,
        .user_id = "rand_user_id",
        .additional_messages_count = 1,
        .additional_messages = (coze_message_t[]){message}
    };
    coze_chat_create_response_t resp = {0};
    const coze_error_t err = coze_chat_create(&req, &resp);
    if (err != COZE_OK) {
        fprintf(stderr, "Error creating chat: %d, code: %d, msg: %s\n", err, resp.code, resp.msg);
        return 1;
    }
    printf("[chat.create] log_id: %s\n", resp.response.logid);
    printf("[chat.create] id: %s\n", resp.data.id);
    printf("[chat.create] created_at: %ld\n", resp.data.created_at);
    printf("[chat.create] conversation_id: %s\n", resp.data.conversation_id);
    printf("[chat.create] bot_id: %s\n", resp.data.bot_id);
    printf("[chat.create] status: %s\n", resp.data.status);
    printf("\n");

    const char *conversation_id = strdup(resp.data.conversation_id);
    const char *chat_id = strdup(resp.data.id);
    coze_free_chat_create_response(&resp);

    // Keep checking status until completed
    const coze_chat_retrieve_request_t retrieve_req = {
        .api_token = api_token,
        .conversation_id = conversation_id,
        .chat_id = chat_id
    };
    coze_chat_retrieve_response_t retrieve_resp={0};
    
    while (1) {
        const coze_error_t retrieve_err = coze_chat_retrieve(&retrieve_req, &retrieve_resp);
        if (retrieve_err != COZE_OK) {
            fprintf(stderr, "Error retrieving chat: %d, code: %d, msg: %s\n", retrieve_err, retrieve_resp.code, retrieve_resp.msg);
            return 1;
        }
        printf("[chat.retrieve] status: %s\n", retrieve_resp.data.status);
        
        if (strcmp(retrieve_resp.data.status, COZE_CHAT_STATUS_COMPLETED) == 0) {
            break;
        }
        
        coze_free_chat_retrieve_response(&retrieve_resp);
        // Wait a bit before next check
        usleep(1000000); // 1s
    }

    // Get messages once completed
    const coze_chat_messages_list_request_t messages_req = {
        .api_token = api_token,
        .conversation_id = conversation_id,
        .chat_id = chat_id
    };
    coze_chat_messages_list_response_t messages_resp={0};
    const coze_error_t messages_err = coze_chat_messages_list(&messages_req, &messages_resp);
    if (messages_err != COZE_OK) {
        fprintf(stderr, "Error listing messages: %d, code: %d, msg: %s\n", messages_err, messages_resp.code, messages_resp.msg);
        return 1;
    }

    printf("\nMessages:\n");
    for (int i = 0; i < messages_resp.data.messages_count; i++) {
        printf("  [chat.messages][%d] [%s] [%s] %s\n", 
            i,
            messages_resp.data.messages[i].role,
            messages_resp.data.messages[i].type,
            messages_resp.data.messages[i].content);
    }

    // Cleanup
    coze_free_chat_retrieve_response(&retrieve_resp);
    coze_free_chat_messages_list_response(&messages_resp);
    free((void*)conversation_id);
    free((void*)chat_id);

    return 0;
}
