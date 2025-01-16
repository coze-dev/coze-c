#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "coze.h"
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    const char *client_id = getenv("COZE_AUTH_WEB_OAUTH_CLIENT_ID");
    if (!client_id) {
        printf("Please set COZE_AUTH_WEB_OAUTH_CLIENT_ID in .env file\n");
        return 1;
    }
    const char *client_secret = getenv("COZE_AUTH_WEB_OAUTH_CLIENT_SECRET");
    if (!client_secret) {
        printf("Please set COZE_AUTH_WEB_OAUTH_CLIENT_SECRET in .env file\n");
        return 1;
    }
    const char *redirect_uri = getenv("COZE_AUTH_WEB_OAUTH_REDIRECT_URI");
    if (!redirect_uri) {
        printf("Please set COZE_AUTH_WEB_OAUTH_REDIRECT_URI in .env file\n");
        return 1;
    }
    if (argc != 2) {
        printf("Usage: %s <code>\n", argv[0]);
        return 1;
    }
    const char *refresh_token = argv[1];
    
    coze_web_oauth_refresh_access_token_request_t web_oauth_req = {0};
    web_oauth_req.client_id = client_id;
    web_oauth_req.client_secret = client_secret;
    web_oauth_req.refresh_token = refresh_token;

    coze_web_oauth_refresh_access_token_response_t web_oauth_resp = {0};
    const coze_error_t err = coze_web_oauth_refresh_access_token(&web_oauth_req, &web_oauth_resp);
    if (err != COZE_OK) {
        printf("Error: %s\n", web_oauth_resp.msg);
        coze_free_web_oauth_refresh_access_token_response(&web_oauth_resp);
        return 1;
    }

    printf("[refresh] Web OAuth Access Token: %s\n", web_oauth_resp.data.access_token);
    printf("[refresh] Web OAuth Refresh Token: %s\n", web_oauth_resp.data.refresh_token);
    printf("[refresh] Web OAuth Expires In: %ld\n", web_oauth_resp.data.expires_in);
    printf("[refresh] Web OAuth Token Type: %s\n", web_oauth_resp.data.token_type);
    
    coze_free_web_oauth_refresh_access_token_response(&web_oauth_resp);
    return 0;
}
