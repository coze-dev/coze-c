#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "coze.h"

int main() {
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
    coze_web_oauth_get_oauth_url_request_t web_oauth_req = {0};
    web_oauth_req.client_id = client_id;
    web_oauth_req.redirect_uri = redirect_uri;
    const char *web_oauth_url = coze_web_oauth_get_oauth_url(&web_oauth_req);
    printf("Web OAuth URL: %s\n", web_oauth_url);
    free((void *)web_oauth_url);
    return 0;
}
