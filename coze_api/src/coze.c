#include "coze.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>
#include "cJSON.h"

static coze_error_t parse_response_code(cJSON *json, char **msg, int *code);

void coze_free_response(coze_response_t *resp);

void coze_free_bot(coze_bot_t *bot);

void coze_free_conversation(coze_conversation_t *conversation);

void coze_free_message(coze_message_t *message);

void coze_free_chat(coze_chat_t *chat);

void coze_free_file(coze_file_t *file);

void coze_free_voice(coze_voice_t *voice);

void coze_free_oauth_token(coze_oauth_token_t *token);

void coze_free_response_header(coze_response_t *coze_response) {
    if (!coze_response) {
        return;
    }
    free((void *) coze_response->logid);
    coze_response->logid = NULL;
}

void pure_log(const char *title, const char *data) {
    char *escaped = malloc(strlen(data) * 2 + 1);
    char *p = escaped;
    for (const char *s = data; *s; s++) {
        if (*s == '\n') {
            *p++ = '\\';
            *p++ = 'n';
        } else if (*s == '\r') {
            *p++ = '\\';
            *p++ = 'r';
        } else if (*s == '\t') {
            *p++ = '\\';
            *p++ = 't';
        } else if (*s == '"') {
            *p++ = '\\';
            *p++ = '"';
        } else {
            *p++ = *s;
        }
    }
    *p = '\0';
    printf("%s: %s\n", title, escaped);
    free(escaped);
}

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *) userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// 回调函数，用于处理响应头数据
static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
    size_t total_size = size * nitems;
    coze_response_t *coze_response = (coze_response_t *) userdata;

    // 将响应头行转换为字符串便于处理
    char header_line[1024] = {0};
    strncpy(header_line, buffer, total_size < sizeof(header_line) ? total_size : sizeof(header_line) - 1);

    // 查找 x-tt-logid header
    if (strncasecmp(header_line, "x-tt-logid:", 11) == 0) {
        char *logid_start = header_line + 11;
        // 跳过空格
        while (*logid_start == ' ') {
            logid_start++;
        }
        // 移除末尾的换行符和回车符
        char *logid_end = logid_start;
        while (*logid_end && *logid_end != '\r' && *logid_end != '\n') {
            logid_end++;
        }
        *logid_end = '\0';

        // 保存 logid 到 header 结构体
        if (coze_response) {
            coze_response->logid = strdup(logid_start);
        }
    }

    return total_size;
}

static char *build_url(const char *api_base, const char *path) {
    const char *base_url;
    if (api_base && strlen(api_base) > 0) {
        if (api_base[strlen(api_base) - 1] == '/') {
            base_url = strdup(api_base);
        } else {
            base_url = strdup(api_base);
        }
    } else {
        base_url = "https://api.coze.cn";
    }

    char *url = strdup(base_url);
    if (url[strlen(url) - 1] == '/') {
        url[strlen(url) - 1] = '\0';
    }
    url = strcat(url, path);
    return url;
}

// 通用的 HTTP 请求函数
static coze_error_t make_http_request(
    const char *api_base, const char *api_token,
    const char *path, const char *method, const char *json_body,
    struct MemoryStruct *chunk, coze_response_t *coze_response) {
    CURL *curl = curl_easy_init();
    if (!curl) return COZE_ERROR_NETWORK;

    struct curl_slist *headers = NULL;
    if (api_token) {
        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_token);
        headers = curl_slist_append(headers, auth_header);
    }
    if (json_body) {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }
    char *url = build_url(api_base, path);

    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    // 设置 CURL 选项
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)chunk);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, coze_response);

    // 如果是 POST 请求
    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (json_body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);
        }
    }

    printf("[coze_api] start: %s %s\n", method, url);
    if (json_body && strcmp(method, "POST") == 0) {
        printf("[coze_api] body: %s\n", json_body);
    }

    // 执行请求
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        free(chunk->memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return COZE_ERROR_NETWORK;
    }

    printf("[coze_api] response: %s, %s\n", coze_response->logid, chunk->memory);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return COZE_OK;
}

// data: id: xx\ndata: xx\n
typedef void (*sse_event_callback_t)(const char *data, void *biz_ctx);

// SSE 数据处理回调
struct SSEContext {
    char *buffer; // 用于存储未完整的 SSE 消息
    size_t buffer_size;
    size_t buffer_used;
    sse_event_callback_t sse_event_callback;
    void *biz_ctx;
};

static void process_sse_message(const struct SSEContext *ctx, const char *message, size_t length) {
    if (!message || length == 0) return;

    // 创建临时缓冲区存储消息
    char *temp = malloc(length + 1);
    memcpy(temp, message, length);
    temp[length] = '\0';

    if (ctx->sse_event_callback) {
        ctx->sse_event_callback(temp, ctx->biz_ctx);
    }

    free(temp);
}

static size_t sse_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct SSEContext *ctx = userp;

    // 确保缓冲区足够大
    if (ctx->buffer_used + realsize > ctx->buffer_size) {
        size_t new_size = ctx->buffer_size + realsize + 4096; // 增加额外空间
        char *new_buffer = realloc(ctx->buffer, new_size);
        if (!new_buffer) return 0;

        ctx->buffer = new_buffer;
        ctx->buffer_size = new_size;
    }

    // 添加新数据到缓冲区
    memcpy(ctx->buffer + ctx->buffer_used, contents, realsize);
    ctx->buffer_used += realsize;
    ctx->buffer[ctx->buffer_used] = '\0';

    // 处理完整的 SSE 消息
    char *start = ctx->buffer;
    char *end;
    while ((end = strstr(start, "\n\n")) != NULL) {
        *end = '\0'; // 临时终止字符串
        process_sse_message(ctx, start, end - start);
        start = end + 2; // 跳过两个换行符

        size_t remaining = ctx->buffer_used - (start - ctx->buffer);
        if (remaining > 0) {
            memmove(ctx->buffer, start, remaining);
        }
        ctx->buffer_used = remaining;
        ctx->buffer[ctx->buffer_used] = '\0';
        start = ctx->buffer;
    }

    return realsize;
}

// 通用的 HTTP SSE 请求函数
static coze_error_t make_http_sse_request(
    const char *api_base, const char *api_token,
    const char *path, const char *method,
    const char *json_body,
    const sse_event_callback_t sse_event_callback,
    void *biz_ctx,
    coze_response_t *coze_response) {
    CURL *curl = curl_easy_init();
    if (!curl) return COZE_ERROR_NETWORK;

    struct SSEContext ctx = {0};
    ctx.buffer = malloc(4096); // 初始分配 4KB
    ctx.buffer_size = 4096;
    ctx.buffer_used = 0;
    ctx.sse_event_callback = sse_event_callback;
    ctx.biz_ctx = biz_ctx;

    // 设置请求头
    struct curl_slist *headers = NULL;
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_token);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Accept: text/event-stream");
    headers = curl_slist_append(headers, "Cache-Control: no-cache");

    if (json_body) {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }

    char *url = build_url(api_base, url);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sse_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&ctx);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, coze_response);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L); // 无超时限制
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    // 如果是 POST 请求
    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (json_body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);
        }
    }

    if (json_body) {
        printf("[coze_api] start SSE: %s %s, body: %s\n", method, url, json_body);
    } else {
        printf("[coze_api] start SSE: %s %s\n", method, url);
    }

    // 执行请求
    CURLcode res = curl_easy_perform(curl);

    // 处理剩余的不完整消息
    if (ctx.buffer_used > 0) {
        process_sse_message(&ctx, ctx.buffer, ctx.buffer_used);
    }

    free(ctx.buffer);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return COZE_ERROR_NETWORK;
    }

    printf("[coze_api] SSE completed: %s\n", coze_response->logid);
    return COZE_OK;
}

// 通用的 JSON 响应解析函数
static coze_error_t parse_response_code(cJSON *json, char **msg, int *code) {
    cJSON *code_obj = cJSON_GetObjectItem(json, "code");
    cJSON *msg_obj = cJSON_GetObjectItem(json, "msg");
    cJSON *error_code_obj = cJSON_GetObjectItem(json, "error_code");
    cJSON *error_message_obj = cJSON_GetObjectItem(json, "error_message");

    if (msg_obj && msg) {
        *msg = strdup(msg_obj->valuestring);
    }
    if (error_message_obj && msg) {
        *msg = strdup(error_message_obj->valuestring);
    }

    if (code_obj) {
        if (code) {
            *code = code_obj->valueint;
        }
        if (code_obj->valueint != 0) {
            return COZE_ERROR_API;
        }
    } else if (error_code_obj) {
        *code = -1; // auth code not int
        return COZE_ERROR_API;
    }

    return COZE_OK;
}


char *coze_web_oauth_get_oauth_url(const coze_web_oauth_get_oauth_url_request_t *req) {
    if (!req || !req->client_id) {
        return NULL;
    }

    char url[512];
    snprintf(url, sizeof(url), "https://www.coze.cn/api/permission/oauth2/authorize");
    if (req->workspace_id) {
        snprintf(url, sizeof(url), "https://www.coze.cn/api/permission/oauth2/workspace_id/%s/authorize",
                 req->workspace_id);
    }
    snprintf(url, sizeof(url), "%s?response_type=code", url);
    if (req->client_id) {
        snprintf(url, sizeof(url), "%s&client_id=%s", url, req->client_id);
    }
    if (req->redirect_uri) {
        snprintf(url, sizeof(url), "%s&redirect_uri=%s", url, req->redirect_uri);
    }
    if (req->state) {
        snprintf(url, sizeof(url), "%s?state=%s", url, req->state);
    } else {
        snprintf(url, sizeof(url), "%s&state=%s", url, "");
    }

    return strdup(url);
}

coze_error_t coze_web_oauth_get_access_token(const coze_web_oauth_get_access_token_request_t *req,
                                             coze_web_oauth_get_access_token_response_t *resp) {
    if (!req || !resp) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    // 构建 URL 和查询参数
    const char *path = "/api/permission/oauth2/token";

    cJSON *body = cJSON_CreateObject();
    if (req->client_id) {
        cJSON_AddStringToObject(body, "client_id", req->client_id);
    }
    cJSON_AddStringToObject(body, "grant_type", "authorization_code");
    if (req->code) {
        cJSON_AddStringToObject(body, "code", req->code);
    }
    if (req->redirect_uri) {
        cJSON_AddStringToObject(body, "redirect_uri", req->redirect_uri);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->client_secret,
                                         path, "POST", json_body, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        resp->msg = tmp_msg;
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_workspaces_data_t workspaces_data = {0};

    cJSON *access_token = cJSON_GetObjectItem(json, "access_token");
    cJSON *refresh_token = cJSON_GetObjectItem(json, "refresh_token");
    cJSON *expires_in = cJSON_GetObjectItem(json, "expires_in");
    cJSON *token_type = cJSON_GetObjectItem(json, "token_type");

    resp->data.access_token = access_token ? strdup(access_token->valuestring) : NULL;
    resp->data.refresh_token = refresh_token ? strdup(refresh_token->valuestring) : NULL;
    resp->data.expires_in = expires_in ? expires_in->valueint : 0;
    resp->data.token_type = token_type ? strdup(token_type->valuestring) : NULL;
    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_web_oauth_get_access_token_response(coze_web_oauth_get_access_token_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_oauth_token(&resp->data);
}

coze_error_t coze_web_oauth_refresh_access_token(const coze_web_oauth_refresh_access_token_request_t *req,
                                                 coze_web_oauth_refresh_access_token_response_t *resp) {
    if (!req || !resp) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    // 构建 URL 和查询参数
    const char *path = "/api/permission/oauth2/token";

    cJSON *body = cJSON_CreateObject();
    if (req->client_id) {
        cJSON_AddStringToObject(body, "client_id", req->client_id);
    }
    cJSON_AddStringToObject(body, "grant_type", "refresh_token");
    if (req->refresh_token) {
        cJSON_AddStringToObject(body, "refresh_token", req->refresh_token);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->client_secret,
                                         path, "POST", json_body, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        resp->msg = tmp_msg;
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_workspaces_data_t workspaces_data = {0};

    cJSON *access_token = cJSON_GetObjectItem(json, "access_token");
    cJSON *refresh_token = cJSON_GetObjectItem(json, "refresh_token");
    cJSON *expires_in = cJSON_GetObjectItem(json, "expires_in");
    cJSON *token_type = cJSON_GetObjectItem(json, "token_type");

    resp->data.access_token = access_token ? strdup(access_token->valuestring) : NULL;
    resp->data.refresh_token = refresh_token ? strdup(refresh_token->valuestring) : NULL;
    resp->data.expires_in = expires_in ? expires_in->valueint : 0;
    resp->data.token_type = token_type ? strdup(token_type->valuestring) : NULL;
    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_web_oauth_refresh_access_token_response(coze_web_oauth_refresh_access_token_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_oauth_token(&resp->data);
}


coze_error_t coze_workspaces_list(const coze_workspaces_list_request_t *req,
                                  coze_workspaces_list_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path),
             "/v1/workspaces?page_num=%d&page_size=%d",
             req->page_num, req->page_size);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token,
                                         path, "GET", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    cJSON *data = cJSON_GetObjectItem(json, "data");
    coze_workspaces_data_t workspaces_data = {0};
    if (data) {
        cJSON *total = cJSON_GetObjectItem(data, "total_count");
        if (total) {
            workspaces_data.total_count = total->valueint;
        }

        cJSON *workspaces = cJSON_GetObjectItem(data, "workspaces");
        if (workspaces) {
            int workspace_count = cJSON_GetArraySize(workspaces);
            workspaces_data.workspace_count = workspace_count;
            workspaces_data.workspaces = calloc(workspace_count, sizeof(coze_workspace_t));

            for (int i = 0; i < workspace_count; i++) {
                cJSON *workspace = cJSON_GetArrayItem(workspaces, i);
                if (workspace) {
                    cJSON *id = cJSON_GetObjectItem(workspace, "id");
                    cJSON *name = cJSON_GetObjectItem(workspace, "name");
                    cJSON *icon_url = cJSON_GetObjectItem(workspace, "icon_url");
                    cJSON *role_type = cJSON_GetObjectItem(workspace, "role_type");
                    cJSON *workspace_type = cJSON_GetObjectItem(workspace, "workspace_type");

                    workspaces_data.workspaces[i].id = id ? strdup(id->valuestring) : NULL;
                    workspaces_data.workspaces[i].name = name ? strdup(name->valuestring) : NULL;
                    workspaces_data.workspaces[i].icon_url = icon_url ? strdup(icon_url->valuestring) : NULL;
                    workspaces_data.workspaces[i].role_type = role_type ? strdup(role_type->valuestring) : NULL;
                    workspaces_data.workspaces[i].workspace_type = workspace_type
                                                                       ? strdup(workspace_type->valuestring)
                                                                       : NULL;
                }
            }
        }
    }
    resp->data = workspaces_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_workspaces_list_response(coze_workspaces_list_response_t *resp) {
    if (!resp) {
        return;
    }

    for (int i = 0; i < resp->data.workspace_count; i++) {
        free((void *) resp->data.workspaces[i].id);
        free((void *) resp->data.workspaces[i].name);
        free((void *) resp->data.workspaces[i].icon_url);
        free((void *) resp->data.workspaces[i].role_type);
        free((void *) resp->data.workspaces[i].workspace_type);
    }

    free(resp->data.workspaces);
    free((void *) resp->msg);
    coze_free_response(&resp->response);
}


coze_error_t coze_bots_create(const coze_bots_create_request_t *req,
                              coze_bots_create_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    // 构建完整的 URL
    const char *path = "/v1/bot/create";


    cJSON *body = cJSON_CreateObject();
    if (req->space_id) {
        cJSON_AddStringToObject(body, "space_id", req->space_id);
    }
    if (req->name) {
        cJSON_AddStringToObject(body, "name", req->name);
    }
    if (req->description) {
        cJSON_AddStringToObject(body, "description", req->description);
    }
    if (req->icon_file_id) {
        cJSON_AddStringToObject(body, "icon_file_id", req->icon_file_id);
    }
    cJSON *prompt_info = cJSON_CreateObject();
    if (req->prompt_info) {
        if (req->prompt_info->prompt) {
            cJSON_AddStringToObject(prompt_info, "prompt", req->prompt_info->prompt);
        }
        cJSON_AddItemToObject(body, "prompt_info", prompt_info);
    }
    cJSON *onboarding_info = cJSON_CreateObject();
    if (req->onboarding_info) {
        if (req->onboarding_info->prologue) {
            cJSON_AddStringToObject(onboarding_info, "prologue", req->onboarding_info->prologue);
        }
        if (req->onboarding_info->suggested_questions) {
            cJSON *suggested_questions = cJSON_CreateArray();
            for (int i = 0; i < req->onboarding_info->suggested_questions_count; i++) {
                cJSON *question = cJSON_CreateString(req->onboarding_info->suggested_questions[i]);
                cJSON_AddItemToArray(suggested_questions, question);
            }
            cJSON_AddItemToObject(onboarding_info, "suggested_questions", suggested_questions);
        }
        cJSON_AddItemToObject(body, "onboarding_info", onboarding_info);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);
    cJSON_Delete(prompt_info);
    cJSON_Delete(onboarding_info);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, NULL);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 提取数据
    cJSON *data = cJSON_GetObjectItem(json, "data");
    coze_bot_t bot_info = {0};
    if (data) {
        cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");


        bot_info.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
    }
    resp->data = bot_info;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_bots_create_response(coze_bots_create_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_bot(&resp->data);
}


coze_error_t coze_bots_update(const coze_bots_update_request_t *req,
                              coze_bots_update_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    // 构建完整的 URL
    const char *path = "/v1/bot/update";


    cJSON *body = cJSON_CreateObject();
    if (req->bot_id) {
        cJSON_AddStringToObject(body, "bot_id", req->bot_id);
    }
    if (req->name) {
        cJSON_AddStringToObject(body, "name", req->name);
    }
    if (req->description) {
        cJSON_AddStringToObject(body, "description", req->description);
    }
    if (req->icon_file_id) {
        cJSON_AddStringToObject(body, "icon_file_id", req->icon_file_id);
    }
    if (req->prompt_info) {
        cJSON *prompt_info = cJSON_CreateObject();
        if (req->prompt_info->prompt) {
            cJSON_AddStringToObject(prompt_info, "prompt", req->prompt_info->prompt);
        }
        cJSON_AddItemToObject(body, "prompt_info", prompt_info);
    }
    if (req->onboarding_info) {
        cJSON *onboarding_info = cJSON_CreateObject();
        if (req->onboarding_info->prologue) {
            cJSON_AddStringToObject(onboarding_info, "prologue", req->onboarding_info->prologue);
        }
        if (req->onboarding_info->suggested_questions) {
            cJSON *suggested_questions = cJSON_CreateArray();
            for (int i = 0; i < req->onboarding_info->suggested_questions_count; i++) {
                cJSON *question = cJSON_CreateString(req->onboarding_info->suggested_questions[i]);
                cJSON_AddItemToArray(suggested_questions, question);
            }
            cJSON_AddItemToObject(onboarding_info, "suggested_questions", suggested_questions);
        }
        cJSON_AddItemToObject(body, "onboarding_info", onboarding_info);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, NULL);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }


    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_bots_update_response(coze_bots_update_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    coze_free_response(&resp->response);
}

coze_error_t coze_bots_publish(const coze_bots_publish_request_t *req,
                               coze_bots_publish_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    // 构建完整的 URL
    const char *path = "/v1/bot/publish";

    cJSON *body = cJSON_CreateObject();
    if (req->bot_id) {
        cJSON_AddStringToObject(body, "bot_id", req->bot_id);
    }
    cJSON *connector_ids = cJSON_CreateArray();
    if (req->connector_ids && req->connector_ids_count > 0) {
        for (int i = 0; i < req->connector_ids_count; i++) {
            cJSON *connector_id = cJSON_CreateString(req->connector_ids[i]);
            cJSON_AddItemToArray(connector_ids, connector_id);
        }
        cJSON_AddItemToObject(body, "connector_ids", connector_ids);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, NULL);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }


    // 提取数据
    cJSON *data = cJSON_GetObjectItem(json, "data");
    coze_bot_t bot_info = {0};
    if (data) {
        cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        cJSON *version = cJSON_GetObjectItem(data, "version");


        bot_info.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        bot_info.version = version ? strdup(version->valuestring) : NULL;
    }
    resp->data = bot_info;


    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_bots_publish_response(coze_bots_publish_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_bot(&resp->data);
}


coze_error_t coze_bots_list(const coze_bots_list_request_t *req,
                            coze_bots_list_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path),
             "/v1/space/published_bots_list?space_id=%s&page_index=%d&page_size=%d",
             req->space_id, req->page_num, req->page_size);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "GET", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    cJSON *data = cJSON_GetObjectItem(json, "data");
    coze_bots_list_data_t coze_bots_list_data = {0};
    if (data) {
        cJSON *total = cJSON_GetObjectItem(data, "total");
        if (total) {
            coze_bots_list_data.total = total->valueint;
        }

        cJSON *space_bots = cJSON_GetObjectItem(data, "space_bots");
        if (space_bots) {
            int space_bot_count = cJSON_GetArraySize(space_bots);
            coze_bots_list_data.space_bot_count = space_bot_count;
            coze_bots_list_data.space_bots = calloc(space_bot_count, sizeof(coze_simple_bot_t));

            for (int i = 0; i < space_bot_count; i++) {
                cJSON *space_bot = cJSON_GetArrayItem(space_bots, i);
                if (space_bot) {
                    cJSON *bot_id = cJSON_GetObjectItem(space_bot, "bot_id");
                    cJSON *bot_name = cJSON_GetObjectItem(space_bot, "bot_name");
                    cJSON *description = cJSON_GetObjectItem(space_bot, "description");
                    cJSON *icon_url = cJSON_GetObjectItem(space_bot, "icon_url");
                    cJSON *publish_time = cJSON_GetObjectItem(space_bot, "publish_time");

                    coze_bots_list_data.space_bots[i].bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
                    coze_bots_list_data.space_bots[i].bot_name = bot_name ? strdup(bot_name->valuestring) : NULL;
                    coze_bots_list_data.space_bots[i].description = description
                                                                        ? strdup(description->valuestring)
                                                                        : NULL;
                    coze_bots_list_data.space_bots[i].icon_url = icon_url ? strdup(icon_url->valuestring) : NULL;
                    coze_bots_list_data.space_bots[i].publish_time = publish_time
                                                                         ? strdup(publish_time->valuestring)
                                                                         : NULL;
                }
            }
        }
    }
    resp->data = coze_bots_list_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_bots_list_response(coze_bots_list_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    if (resp->data.space_bots) {
        for (int i = 0; i < resp->data.space_bot_count; i++) {
            free((void *) resp->data.space_bots[i].bot_id);
            free((void *) resp->data.space_bots[i].bot_name);
            free((void *) resp->data.space_bots[i].description);
            free((void *) resp->data.space_bots[i].icon_url);
            free((void *) resp->data.space_bots[i].publish_time);
        }
    }
    coze_free_response(&resp->response);
}

coze_error_t coze_bots_retrieve(const coze_bots_retrieve_request_t *req,
                                coze_bots_retrieve_response_t *resp) {
    if (!req || !resp || !req->api_token || !req->bot_id) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    // 构建完整的 URL
    char path[512];
    snprintf(path, sizeof(path), "/v1/bot/get_online_info?bot_id=%s", req->bot_id);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "GET", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, NULL);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 提取数据
    cJSON *data = cJSON_GetObjectItem(json, "data");
    coze_bot_t bot_info = {0};
    if (data) {
        cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        cJSON *name = cJSON_GetObjectItem(data, "name");
        cJSON *description = cJSON_GetObjectItem(data, "description");
        cJSON *icon_url = cJSON_GetObjectItem(data, "icon_url");
        cJSON *create_time = cJSON_GetObjectItem(data, "create_time");
        cJSON *update_time = cJSON_GetObjectItem(data, "update_time");
        cJSON *version = cJSON_GetObjectItem(data, "version");

        bot_info.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        bot_info.name = name ? strdup(name->valuestring) : NULL;
        bot_info.description = description ? strdup(description->valuestring) : NULL;
        bot_info.icon_url = icon_url ? strdup(icon_url->valuestring) : NULL;
        bot_info.create_time = create_time ? create_time->valueint : 0;
        bot_info.update_time = update_time ? update_time->valueint : 0;
        bot_info.version = version ? strdup(version->valuestring) : NULL;
    }
    resp->data = bot_info;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_bots_retrieve_response(coze_bots_retrieve_response_t *resp) {
    if (!resp) {
        return;
    }
    if (resp->msg) {
        free((void *) resp->msg);
    }
    coze_free_bot(&resp->data);
    coze_free_response(&resp->response);
}


coze_error_t coze_conversations_create(const coze_conversations_create_request_t *req,
                                       coze_conversations_create_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    const char *path = "/v1/conversation/create";

    // 构建请求体
    cJSON *body = cJSON_CreateObject();
    if (req->bot_id) {
        cJSON_AddStringToObject(body, "bot_id", req->bot_id);
    }
    if (req->messages) {
        cJSON *messages = cJSON_CreateArray();
        for (int i = 0; i < req->message_count; i++) {
            cJSON *message = cJSON_CreateObject();
            if (req->messages[i].role) {
                cJSON_AddStringToObject(message, "role", req->messages[i].role);
            }
            if (req->messages[i].type) {
                cJSON_AddStringToObject(message, "type", req->messages[i].type);
            }
            if (req->messages[i].content) {
                cJSON_AddStringToObject(message, "content", req->messages[i].content);
            }
            if (req->messages[i].content_type) {
                cJSON_AddStringToObject(message, "content_type", req->messages[i].content_type);
            }
            cJSON_AddItemToArray(messages, message);
        }
        cJSON_AddItemToObject(body, "messages", messages);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);


    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;
    free(json_body);

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    cJSON *data = cJSON_GetObjectItem(json, "data");
    coze_conversation_t conversation_data = {0};
    if (data) {
        cJSON *id = cJSON_GetObjectItem(data, "id");
        cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        cJSON *last_section_id = cJSON_GetObjectItem(data, "last_section_id");

        if (id) {
            conversation_data.id = id ? strdup(id->valuestring) : NULL;
        }
        if (created_at) {
            conversation_data.created_at = created_at ? created_at->valueint : 0;
        }
        if (last_section_id) {
            conversation_data.last_section_id = last_section_id ? strdup(last_section_id->valuestring) : NULL;
        }
    }
    resp->data = conversation_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_conversations_create_response(coze_conversations_create_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_conversation(&resp->data);
}

coze_error_t coze_conversations_retrieve(const coze_conversations_retrieve_request_t *req,
                                         coze_conversations_retrieve_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;


    // 构建 URL
    char path[512];
    snprintf(path, sizeof(path),
             "/v1/conversation/retrieve?conversation_id=%s",
             req->conversation_id);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "GET", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    cJSON *data = cJSON_GetObjectItem(json, "data");
    coze_conversation_t conversation_data = {0};
    if (data) {
        cJSON *id = cJSON_GetObjectItem(data, "id");
        cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        cJSON *last_section_id = cJSON_GetObjectItem(data, "last_section_id");

        if (id) {
            conversation_data.id = id ? strdup(id->valuestring) : NULL;
        }
        if (created_at) {
            conversation_data.created_at = created_at ? created_at->valueint : 0;
        }
        if (last_section_id) {
            conversation_data.last_section_id = last_section_id ? strdup(last_section_id->valuestring) : NULL;
        }
    }
    resp->data = conversation_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_conversations_retrieve_response(coze_conversations_retrieve_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_conversation(&resp->data);
}


coze_error_t coze_conversations_messages_create(const coze_conversations_messages_create_request_t *req,
                                                coze_conversations_messages_create_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    cJSON *body = cJSON_CreateObject();
    if (req->role) {
        cJSON_AddStringToObject(body, "role", req->role);
    }
    if (req->content) {
        cJSON_AddStringToObject(body, "content", req->content);
    }
    if (req->content_type) {
        cJSON_AddStringToObject(body, "content_type", req->content_type);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    // 构建 URL
    char path[512];
    snprintf(path, sizeof(path),
             "https://api.coze.cn/v1/conversation/message/create?conversation_id=%s",
             req->conversation_id);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;
    free(json_body);

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    cJSON *data = cJSON_GetObjectItem(json, "data");
    coze_message_t message_data = {0};
    if (data) {
        cJSON *id = cJSON_GetObjectItem(data, "id");
        cJSON *conversation_id = cJSON_GetObjectItem(data, "conversation_id");
        cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        cJSON *chat_id = cJSON_GetObjectItem(data, "chat_id");
        cJSON *role = cJSON_GetObjectItem(data, "role");
        cJSON *content = cJSON_GetObjectItem(data, "content");
        cJSON *content_type = cJSON_GetObjectItem(data, "content_type");
        cJSON *type = cJSON_GetObjectItem(data, "type");
        cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        cJSON *updated_at = cJSON_GetObjectItem(data, "updated_at");

        if (id) {
            message_data.id = id ? strdup(id->valuestring) : NULL;
        }
        if (conversation_id) {
            message_data.conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
        }
        if (bot_id) {
            message_data.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        }
        if (chat_id) {
            message_data.chat_id = chat_id ? strdup(chat_id->valuestring) : NULL;
        }
        if (role) {
            message_data.role = role ? strdup(role->valuestring) : NULL;
        }
        if (content) {
            message_data.content = content ? strdup(content->valuestring) : NULL;
        }
        if (content_type) {
            message_data.content_type = content_type ? strdup(content_type->valuestring) : NULL;
        }
        if (type) {
            message_data.type = type ? strdup(type->valuestring) : NULL;
        }
        if (created_at) {
            message_data.created_at = created_at ? created_at->valueint : 0;
        }
        if (updated_at) {
            message_data.updated_at = updated_at ? updated_at->valueint : 0;
        }
    }
    resp->data = message_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_conversations_messages_create_response(coze_conversations_messages_create_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_message(&resp->data);
}


coze_error_t coze_conversations_messages_list(const coze_conversations_messages_list_request_t *req,
                                              coze_conversations_messages_list_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path),
             "/v1/conversation/message/list?conversation_id=%s",
             req->conversation_id);

    cJSON *body = cJSON_CreateObject();
    if (req->order) {
        cJSON_AddStringToObject(body, "order", req->order);
    }
    if (req->chat_id) {
        cJSON_AddStringToObject(body, "chat_id", req->chat_id);
    }
    if (req->before_id) {
        cJSON_AddStringToObject(body, "before_id", req->before_id);
    }
    if (req->after_id) {
        cJSON_AddStringToObject(body, "after_id", req->after_id);
    }
    if (req->limit) {
        cJSON_AddNumberToObject(body, "limit", req->limit);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;
    free(json_body);
    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_conversations_messages_list_data_t messages_data = {0};
    cJSON *has_more = cJSON_GetObjectItem(json, "has_more");
    if (has_more) {
        messages_data.has_more = cJSON_IsTrue(has_more);
    }
    cJSON *first_id = cJSON_GetObjectItem(json, "first_id");
    if (first_id) {
        messages_data.first_id = first_id ? strdup(first_id->valuestring) : NULL;
    }
    cJSON *last_id = cJSON_GetObjectItem(json, "last_id");
    if (last_id) {
        messages_data.last_id = last_id ? strdup(last_id->valuestring) : NULL;
    }
    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        int messages_count = cJSON_GetArraySize(data);
        messages_data.messages_count = messages_count;
        messages_data.messages = calloc(messages_count, sizeof(coze_message_t));

        for (int i = 0; i < messages_count; i++) {
            cJSON *message = cJSON_GetArrayItem(data, i);
            if (message) {
                cJSON *id = cJSON_GetObjectItem(message, "id");
                cJSON *conversation_id = cJSON_GetObjectItem(message, "conversation_id");
                cJSON *bot_id = cJSON_GetObjectItem(message, "bot_id");
                cJSON *chat_id = cJSON_GetObjectItem(message, "chat_id");
                cJSON *role = cJSON_GetObjectItem(message, "role");
                cJSON *content = cJSON_GetObjectItem(message, "content");
                cJSON *content_type = cJSON_GetObjectItem(message, "content_type");
                cJSON *type = cJSON_GetObjectItem(message, "type");
                cJSON *created_at = cJSON_GetObjectItem(message, "created_at");
                cJSON *updated_at = cJSON_GetObjectItem(message, "updated_at");

                messages_data.messages[i].id = id ? strdup(id->valuestring) : NULL;
                messages_data.messages[i].conversation_id = conversation_id
                                                                ? strdup(conversation_id->valuestring)
                                                                : NULL;
                messages_data.messages[i].bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
                messages_data.messages[i].chat_id = chat_id ? strdup(chat_id->valuestring) : NULL;
                messages_data.messages[i].role = role ? strdup(role->valuestring) : NULL;
                messages_data.messages[i].content = content ? strdup(content->valuestring) : NULL;
                messages_data.messages[i].content_type = content_type ? strdup(content_type->valuestring) : NULL;
                messages_data.messages[i].type = type ? strdup(type->valuestring) : NULL;
                messages_data.messages[i].created_at = created_at ? created_at->valueint : 0;
                messages_data.messages[i].updated_at = updated_at ? updated_at->valueint : 0;
            }
        }
    }
    resp->data = messages_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_conversations_messages_list_response(coze_conversations_messages_list_response_t *resp) {
    if (!resp) {
        return;
    }
    free((void *) resp->msg);
    coze_free_response(&resp->response);
    free((void *) resp->data.first_id);
    free((void *) resp->data.last_id);
    free((void *) resp->data.has_more);
    for (int i = 0; i < resp->data.messages_count; i++) {
        coze_free_message(&resp->data.messages[i]);
    }
    free(resp->data.messages);
}


coze_error_t coze_conversations_messages_retrieve(const coze_conversations_messages_retrieve_request_t *req,
                                                  coze_conversations_messages_retrieve_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path),
             "/v1/conversation/message/retrieve?conversation_id=%s&message_id=%s",
             req->conversation_id, req->message_id);


    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "GET", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_message_t message_data = {0};
    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        cJSON *id = cJSON_GetObjectItem(data, "id");
        cJSON *conversation_id = cJSON_GetObjectItem(data, "conversation_id");
        cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        cJSON *chat_id = cJSON_GetObjectItem(data, "chat_id");
        cJSON *role = cJSON_GetObjectItem(data, "role");
        cJSON *content = cJSON_GetObjectItem(data, "content");
        cJSON *content_type = cJSON_GetObjectItem(data, "content_type");
        cJSON *type = cJSON_GetObjectItem(data, "type");
        cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        cJSON *updated_at = cJSON_GetObjectItem(data, "updated_at");

        message_data.id = id ? strdup(id->valuestring) : NULL;
        message_data.conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
        message_data.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        message_data.chat_id = chat_id ? strdup(chat_id->valuestring) : NULL;
        message_data.role = role ? strdup(role->valuestring) : NULL;
        message_data.content = content ? strdup(content->valuestring) : NULL;
        message_data.content_type = content_type ? strdup(content_type->valuestring) : NULL;
        message_data.type = type ? strdup(type->valuestring) : NULL;
        message_data.created_at = created_at ? created_at->valueint : 0;
        message_data.updated_at = updated_at ? updated_at->valueint : 0;
    }
    resp->data = message_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_conversations_messages_retrieve_response(coze_conversations_messages_retrieve_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_message(&resp->data);
}

coze_error_t coze_conversations_messages_update(const coze_conversations_messages_update_request_t *req,
                                                coze_conversations_messages_update_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path),
             "/v1/conversation/message/modify?conversation_id=%s&message_id=%s",
             req->conversation_id, req->message_id);

    cJSON *body = cJSON_CreateObject();
    if (req->content) {
        cJSON_AddStringToObject(body, "content", req->content);
    }
    if (req->content_type) {
        cJSON_AddStringToObject(body, "content_type", req->content_type);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;
    free(json_body);
    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_message_t message_data = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "message");
    if (data) {
        const cJSON *id = cJSON_GetObjectItem(data, "id");
        const cJSON *conversation_id = cJSON_GetObjectItem(data, "conversation_id");
        const cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        const cJSON *chat_id = cJSON_GetObjectItem(data, "chat_id");
        const cJSON *role = cJSON_GetObjectItem(data, "role");
        const cJSON *content = cJSON_GetObjectItem(data, "content");
        const cJSON *content_type = cJSON_GetObjectItem(data, "content_type");
        const cJSON *type = cJSON_GetObjectItem(data, "type");
        const cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        const cJSON *updated_at = cJSON_GetObjectItem(data, "updated_at");

        message_data.id = id ? strdup(id->valuestring) : NULL;
        message_data.conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
        message_data.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        message_data.chat_id = chat_id ? strdup(chat_id->valuestring) : NULL;
        message_data.role = role ? strdup(role->valuestring) : NULL;
        message_data.content = content ? strdup(content->valuestring) : NULL;
        message_data.content_type = content_type ? strdup(content_type->valuestring) : NULL;
        message_data.type = type ? strdup(type->valuestring) : NULL;
        message_data.created_at = created_at ? created_at->valueint : 0;
        message_data.updated_at = updated_at ? updated_at->valueint : 0;
    }
    resp->data = message_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_conversations_messages_update_response(coze_conversations_messages_update_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_message(&resp->data);
}


coze_error_t coze_conversations_messages_delete(const coze_conversations_messages_delete_request_t *req,
                                                coze_conversations_messages_delete_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path),
             "/v1/conversation/message/delete?conversation_id=%s&message_id=%s",
             req->conversation_id, req->message_id);


    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_message_t message_data = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const cJSON *id = cJSON_GetObjectItem(data, "id");
        const cJSON *conversation_id = cJSON_GetObjectItem(data, "conversation_id");
        const cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        const cJSON *chat_id = cJSON_GetObjectItem(data, "chat_id");
        const cJSON *role = cJSON_GetObjectItem(data, "role");
        const cJSON *content = cJSON_GetObjectItem(data, "content");
        const cJSON *content_type = cJSON_GetObjectItem(data, "content_type");
        const cJSON *type = cJSON_GetObjectItem(data, "type");
        const cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        const cJSON *updated_at = cJSON_GetObjectItem(data, "updated_at");

        message_data.id = id ? strdup(id->valuestring) : NULL;
        message_data.conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
        message_data.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        message_data.chat_id = chat_id ? strdup(chat_id->valuestring) : NULL;
        message_data.role = role ? strdup(role->valuestring) : NULL;
        message_data.content = content ? strdup(content->valuestring) : NULL;
        message_data.content_type = content_type ? strdup(content_type->valuestring) : NULL;
        message_data.type = type ? strdup(type->valuestring) : NULL;
        message_data.created_at = created_at ? created_at->valueint : 0;
        message_data.updated_at = updated_at ? updated_at->valueint : 0;
    }
    resp->data = message_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_conversations_messages_delete_response(coze_conversations_messages_delete_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_message(&resp->data);
}

coze_error_t coze_chat_create(const coze_chat_create_request_t *req,
                              coze_chat_create_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path), "/v3/chat%s%s",
             req->conversation_id ? "?conversation_id=" : "",
             req->conversation_id ? req->conversation_id : "");


    cJSON *body = cJSON_CreateObject();
    if (req->bot_id) {
        cJSON_AddStringToObject(body, "bot_id", req->bot_id);
    }
    if (req->user_id) {
        cJSON_AddStringToObject(body, "user_id", req->user_id);
    }
    if (req->additional_messages && req->additional_messages_count > 0) {
        cJSON *additional_messages_array = cJSON_CreateArray();
        for (size_t i = 0; i < req->additional_messages_count; i++) {
            cJSON *message_obj = cJSON_CreateObject();
            if (req->additional_messages[i].role) {
                cJSON_AddStringToObject(message_obj, "role", req->additional_messages[i].role);
            }
            if (req->additional_messages[i].type) {
                cJSON_AddStringToObject(message_obj, "type", req->additional_messages[i].type);
            }
            if (req->additional_messages[i].content) {
                cJSON_AddStringToObject(message_obj, "content", req->additional_messages[i].content);
            }
            if (req->additional_messages[i].content_type) {
                cJSON_AddStringToObject(message_obj, "content_type", req->additional_messages[i].content_type);
            }
            cJSON_AddItemToArray(additional_messages_array, message_obj);
        }
        cJSON_AddItemToObject(body, "additional_messages", additional_messages_array);
    }
    cJSON_AddFalseToObject(body, "stream");
    cJSON_AddTrueToObject(body, "auto_save_history");
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;
    free(json_body);
    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_chat_t chat_data = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const cJSON *id = cJSON_GetObjectItem(data, "id");
        const cJSON *conversation_id = cJSON_GetObjectItem(data, "conversation_id");
        const cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        const cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        const cJSON *completed_at = cJSON_GetObjectItem(data, "completed_at");
        const cJSON *status = cJSON_GetObjectItem(data, "status");

        chat_data.id = id ? strdup(id->valuestring) : NULL;
        chat_data.conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
        chat_data.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        chat_data.created_at = created_at ? created_at->valueint : 0;
        chat_data.completed_at = completed_at ? completed_at->valueint : 0;
        chat_data.status = status ? strdup(status->valuestring) : NULL;
    }
    resp->data = chat_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_chat_create_response(coze_chat_create_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_chat(&resp->data);
}


struct ChatSSECallbackContext {
    void (*callback)(const coze_chat_event_t *chat_event);
};

void chat_stream_handler(const char *data, void *biz_ctx) {
    pure_log("data", data);
    const struct ChatSSECallbackContext *ctx = (struct ChatSSECallbackContext *) biz_ctx;

    char *event = NULL;
    char *sse_data = NULL;
    char *line = (char *) data;
    char *next_line;

    // Parse event: or data: prefix
    char *saveptr;
    char *line_copy = strdup(line);
    char *curr_line = strtok_r(line_copy, "\n", &saveptr);
    while (curr_line) {
        if (strncmp(curr_line, "event:", 6) == 0) {
            event = strdup(curr_line + 6);
        } else if (strncmp(curr_line, "data:", 5) == 0) {
            sse_data = strdup(curr_line + 5);
        }

        curr_line = strtok_r(NULL, "\n", &saveptr);
    }
    free(line_copy);

    if (!event || !sse_data || !ctx || !ctx->callback) {
        if (event) free(event);
        if (sse_data) free(sse_data);
        return;
    }

    // 在堆上创建事件
    coze_chat_event_t *event_data = calloc(1, sizeof(coze_chat_event_t));
    event_data->event = strdup(event);

    if (strcmp(event, COZE_EVENT_TYPE_DONE) == 0) {
    } else if (strcmp(event, COZE_EVENT_TYPE_ERROR) == 0) {
        printf("[coze_api] SSE error: %s\n", sse_data);
    } else if (strcmp(event, COZE_EVENT_TYPE_CONVERSATION_MESSAGE_DELTA) == 0 ||
               strcmp(event, COZE_EVENT_TYPE_CONVERSATION_MESSAGE_COMPLETED) == 0 ||
               strcmp(event, COZE_EVENT_TYPE_CONVERSATION_AUDIO_DELTA) == 0) {
        cJSON *json_data = cJSON_Parse(sse_data);
        if (json_data) {
            coze_message_t *message = calloc(1, sizeof(coze_message_t));
            const cJSON *id = cJSON_GetObjectItem(json_data, "id");
            const cJSON *conversation_id = cJSON_GetObjectItem(json_data, "conversation_id");
            const cJSON *bot_id = cJSON_GetObjectItem(json_data, "bot_id");
            const cJSON *chat_id = cJSON_GetObjectItem(json_data, "chat_id");
            const cJSON *role = cJSON_GetObjectItem(json_data, "role");
            const cJSON *content = cJSON_GetObjectItem(json_data, "content");
            const cJSON *content_type = cJSON_GetObjectItem(json_data, "content_type");
            const cJSON *type = cJSON_GetObjectItem(json_data, "type");
            const cJSON *created_at = cJSON_GetObjectItem(json_data, "created_at");
            const cJSON *updated_at = cJSON_GetObjectItem(json_data, "updated_at");

            message->id = id ? strdup(id->valuestring) : NULL;
            message->conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
            message->bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
            message->chat_id = chat_id ? strdup(chat_id->valuestring) : NULL;
            message->role = role ? strdup(role->valuestring) : NULL;
            message->content = content ? strdup(content->valuestring) : NULL;
            message->content_type = content_type ? strdup(content_type->valuestring) : NULL;
            message->type = type ? strdup(type->valuestring) : NULL;
            message->created_at = created_at ? created_at->valueint : 0;
            message->updated_at = updated_at ? updated_at->valueint : 0;

            cJSON_Delete(json_data);
            event_data->message = message;
        }
    } else if (strcmp(event, COZE_EVENT_TYPE_CONVERSATION_CHAT_CREATED) == 0 ||
               strcmp(event, COZE_EVENT_TYPE_CONVERSATION_CHAT_IN_PROGRESS) == 0 ||
               strcmp(event, COZE_EVENT_TYPE_CONVERSATION_CHAT_COMPLETED) == 0 ||
               strcmp(event, COZE_EVENT_TYPE_CONVERSATION_CHAT_FAILED) == 0 ||
               strcmp(event, COZE_EVENT_TYPE_CONVERSATION_CHAT_REQUIRES_ACTION) == 0) {
        cJSON *json_data = cJSON_Parse(sse_data);
        if (json_data) {
            coze_chat_t *chat = calloc(1, sizeof(coze_chat_t));
            const cJSON *id = cJSON_GetObjectItem(json_data, "id");
            const cJSON *conversation_id = cJSON_GetObjectItem(json_data, "conversation_id");
            const cJSON *bot_id = cJSON_GetObjectItem(json_data, "bot_id");
            const cJSON *created_at = cJSON_GetObjectItem(json_data, "created_at");
            const cJSON *completed_at = cJSON_GetObjectItem(json_data, "completed_at");
            const cJSON *status = cJSON_GetObjectItem(json_data, "status");

            chat->id = id ? strdup(id->valuestring) : NULL;
            chat->conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
            chat->bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
            chat->created_at = created_at ? created_at->valueint : 0;
            chat->completed_at = completed_at ? completed_at->valueint : 0;
            chat->status = status ? strdup(status->valuestring) : NULL;

            cJSON_Delete(json_data);
            event_data->chat = chat;
        }
    }

    if (ctx->callback) {
        ctx->callback(event_data);
    }

    // 释放临时数据
    free(event);
    free(sse_data);
}

coze_error_t coze_chat_stream(const coze_chat_stream_request_t *req,
                              coze_chat_stream_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    char path[512];
    snprintf(path, sizeof(path), "/v3/chat%s%s",
             req->conversation_id ? "?conversation_id=" : "",
             req->conversation_id ? req->conversation_id : "");

    cJSON *body = cJSON_CreateObject();
    if (req->bot_id) {
        cJSON_AddStringToObject(body, "bot_id", req->bot_id);
    }
    if (req->user_id) {
        cJSON_AddStringToObject(body, "user_id", req->user_id);
    }
    if (req->additional_messages && req->additional_messages_count > 0) {
        cJSON *additional_messages_array = cJSON_CreateArray();
        for (size_t i = 0; i < req->additional_messages_count; i++) {
            cJSON *message_obj = cJSON_CreateObject();
            if (req->additional_messages[i].role) {
                cJSON_AddStringToObject(message_obj, "role", req->additional_messages[i].role);
            }
            if (req->additional_messages[i].type) {
                cJSON_AddStringToObject(message_obj, "type", req->additional_messages[i].type);
            }
            if (req->additional_messages[i].content) {
                cJSON_AddStringToObject(message_obj, "content", req->additional_messages[i].content);
            }
            if (req->additional_messages[i].content_type) {
                cJSON_AddStringToObject(message_obj, "content_type", req->additional_messages[i].content_type);
            }
            cJSON_AddItemToArray(additional_messages_array, message_obj);
        }
        cJSON_AddItemToObject(body, "additional_messages", additional_messages_array);
    }
    cJSON_AddTrueToObject(body, "stream");
    if (req->auto_save_history) {
        cJSON_AddBoolToObject(body, "auto_save_history", req->auto_save_history);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};

    struct ChatSSECallbackContext biz_ctx = {
        .callback = req->on_event,
    };
    const coze_error_t err = make_http_sse_request(req->api_base, req->api_token, path, "POST", json_body,
                                                   chat_stream_handler,
                                                   &biz_ctx,
                                                   &coze_response);
    resp->response = coze_response;
    free(json_body);
    if (err != COZE_OK) {
        return err;
    }

    resp->code = 0;
    resp->msg = "";

    if (err != COZE_OK) {
        return err;
    }

    return COZE_OK;
}

void coze_free_chat_stream_response(coze_chat_stream_response_t *resp) {
    if (!resp) return;

    if (resp->msg && strlen(resp->msg) > 0) {
        free((void *) resp->msg);
    }
    coze_free_response(&resp->response);
}


coze_error_t coze_chat_retrieve(const coze_chat_retrieve_request_t *req,
                                coze_chat_retrieve_response_t *resp) {
    if (!req || !resp || !req->conversation_id || !req->chat_id) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path), "/v3/chat/retrieve?conversation_id=%s&chat_id=%s",
             req->conversation_id, req->chat_id);


    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "GET", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_chat_t chat_data = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const cJSON *id = cJSON_GetObjectItem(data, "id");
        const cJSON *conversation_id = cJSON_GetObjectItem(data, "conversation_id");
        const cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        const cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        const cJSON *completed_at = cJSON_GetObjectItem(data, "completed_at");
        const cJSON *status = cJSON_GetObjectItem(data, "status");

        chat_data.id = id ? strdup(id->valuestring) : NULL;
        chat_data.conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
        chat_data.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        chat_data.created_at = created_at ? created_at->valueint : 0;
        chat_data.completed_at = completed_at ? completed_at->valueint : 0;
        chat_data.status = status ? strdup(status->valuestring) : NULL;
    }
    resp->data = chat_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_chat_retrieve_response(coze_chat_retrieve_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_chat(&resp->data);
}

coze_error_t coze_chat_messages_list(const coze_chat_messages_list_request_t *req,
                                     coze_chat_messages_list_response_t *resp) {
    if (!req || !resp || !req->conversation_id || !req->chat_id) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path),
             "/v3/chat/message/list?conversation_id=%s&chat_id=%s",
             req->conversation_id, req->chat_id);


    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "GET", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_chat_messages_list_data_t messages_data = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const int messages_count = cJSON_GetArraySize(data);
        messages_data.messages_count = messages_count;
        messages_data.messages = calloc(messages_count, sizeof(coze_message_t));

        for (int i = 0; i < messages_count; i++) {
            const cJSON *message = cJSON_GetArrayItem(data, i);
            if (message) {
                const cJSON *id = cJSON_GetObjectItem(message, "id");
                const cJSON *conversation_id = cJSON_GetObjectItem(message, "conversation_id");
                const cJSON *bot_id = cJSON_GetObjectItem(message, "bot_id");
                const cJSON *chat_id = cJSON_GetObjectItem(message, "chat_id");
                const cJSON *role = cJSON_GetObjectItem(message, "role");
                const cJSON *content = cJSON_GetObjectItem(message, "content");
                const cJSON *content_type = cJSON_GetObjectItem(message, "content_type");
                const cJSON *type = cJSON_GetObjectItem(message, "type");
                const cJSON *created_at = cJSON_GetObjectItem(message, "created_at");
                const cJSON *updated_at = cJSON_GetObjectItem(message, "updated_at");

                messages_data.messages[i].id = id ? strdup(id->valuestring) : NULL;
                messages_data.messages[i].conversation_id = conversation_id
                                                                ? strdup(conversation_id->valuestring)
                                                                : NULL;
                messages_data.messages[i].bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
                messages_data.messages[i].chat_id = chat_id ? strdup(chat_id->valuestring) : NULL;
                messages_data.messages[i].role = role ? strdup(role->valuestring) : NULL;
                messages_data.messages[i].content = content ? strdup(content->valuestring) : NULL;
                messages_data.messages[i].content_type = content_type ? strdup(content_type->valuestring) : NULL;
                messages_data.messages[i].type = type ? strdup(type->valuestring) : NULL;
                messages_data.messages[i].created_at = created_at ? created_at->valueint : 0;
                messages_data.messages[i].updated_at = updated_at ? updated_at->valueint : 0;
            }
        }
    }
    resp->data = messages_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_chat_messages_list_response(coze_chat_messages_list_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    for (int i = 0; i < resp->data.messages_count; i++) {
        coze_free_message(&resp->data.messages[i]);
    }
    free(resp->data.messages);
}


coze_error_t coze_chat_submit_tool_outputs_create(const coze_chat_submit_tool_outputs_create_request_t *req,
                                                  coze_chat_submit_tool_outputs_create_response_t *resp) {
    if (!req || !resp || !req->conversation_id || !req->chat_id) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path), "/v3/chat/submit_tool_outputs?conversation_id=%s&chat_id=%s",
             req->conversation_id, req->chat_id);


    cJSON *body = cJSON_CreateObject();
    if (req->tool_outputs && req->tool_outputs_count > 0) {
        cJSON *tool_outputs_array = cJSON_CreateArray();
        for (size_t i = 0; i < req->tool_outputs_count; i++) {
            cJSON *tool_output_obj = cJSON_CreateObject();
            if (req->tool_outputs[i].tool_call_id) {
                cJSON_AddStringToObject(tool_output_obj, "tool_call_id", req->tool_outputs[i].tool_call_id);
            }
            if (req->tool_outputs[i].output) {
                cJSON_AddStringToObject(tool_output_obj, "output", req->tool_outputs[i].output);
            }
            cJSON_AddItemToArray(tool_outputs_array, tool_output_obj);
        }
        cJSON_AddItemToObject(body, "tool_outputs", tool_outputs_array);
    }
    cJSON_AddFalseToObject(body, "stream");
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;
    free(json_body);
    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_chat_t chat_data = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const cJSON *id = cJSON_GetObjectItem(data, "id");
        const cJSON *conversation_id = cJSON_GetObjectItem(data, "conversation_id");
        const cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        const cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        const cJSON *completed_at = cJSON_GetObjectItem(data, "completed_at");
        const cJSON *status = cJSON_GetObjectItem(data, "status");

        chat_data.id = id ? strdup(id->valuestring) : NULL;
        chat_data.conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
        chat_data.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        chat_data.created_at = created_at ? created_at->valueint : 0;
        chat_data.completed_at = completed_at ? completed_at->valueint : 0;
        chat_data.status = status ? strdup(status->valuestring) : NULL;
    }
    resp->data = chat_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_chat_submit_tool_outputs_create_response(coze_chat_submit_tool_outputs_create_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_chat(&resp->data);
}

coze_error_t coze_chat_cancel(const coze_chat_cancel_request_t *req,
                              coze_chat_cancel_response_t *resp) {
    if (!req || !resp || !req->api_token || !req->conversation_id || !req->chat_id) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path), "/v3/chat/cancel");


    cJSON *body = cJSON_CreateObject();
    if (req->conversation_id) {
        cJSON_AddStringToObject(body, "conversation_id", req->conversation_id);
    }
    if (req->chat_id) {
        cJSON_AddStringToObject(body, "chat_id", req->chat_id);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;
    free(json_body);

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_chat_t chat_data = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const cJSON *id = cJSON_GetObjectItem(data, "id");
        const cJSON *conversation_id = cJSON_GetObjectItem(data, "conversation_id");
        const cJSON *bot_id = cJSON_GetObjectItem(data, "bot_id");
        const cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        const cJSON *completed_at = cJSON_GetObjectItem(data, "completed_at");
        const cJSON *status = cJSON_GetObjectItem(data, "status");

        chat_data.id = id ? strdup(id->valuestring) : NULL;
        chat_data.conversation_id = conversation_id ? strdup(conversation_id->valuestring) : NULL;
        chat_data.bot_id = bot_id ? strdup(bot_id->valuestring) : NULL;
        chat_data.created_at = created_at ? created_at->valueint : 0;
        chat_data.completed_at = completed_at ? completed_at->valueint : 0;
        chat_data.status = status ? strdup(status->valuestring) : NULL;
    }
    resp->data = chat_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_chat_cancel_response(coze_chat_cancel_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_chat(&resp->data);
}

coze_error_t coze_files_upload(const coze_files_upload_request_t *req,
                               coze_files_upload_response_t *resp) {
    if (!req || !resp || !req->api_token || !req->file) {
        return COZE_ERROR_INVALID_PARAM;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        return COZE_ERROR_API;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    // 设置 URL
    const char *path = "/v1/files/upload";

    // 设置请求头
    struct curl_slist *headers = NULL;
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", req->api_token);
    headers = curl_slist_append(headers, auth_header);

    // 设置 CURL 选项
    curl_easy_setopt(curl, CURLOPT_URL, build_url(req->api_base, path));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp->response);

    // 设置文件上传
    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part = curl_mime_addpart(mime);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, req->file);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    // 执行请求
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        free(chunk.memory);
        curl_slist_free_all(headers);
        curl_mime_free(mime);
        curl_easy_cleanup(curl);
        return COZE_ERROR_NETWORK;
    }

    // 解析响应
    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        curl_slist_free_all(headers);
        curl_mime_free(mime);
        curl_easy_cleanup(curl);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    coze_error_t err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    coze_file_t file_data = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const cJSON *id = cJSON_GetObjectItem(data, "id");
        const cJSON *file_name = cJSON_GetObjectItem(data, "file_name");
        const cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        const cJSON *bytes = cJSON_GetObjectItem(data, "bytes");

        file_data.id = id ? strdup(id->valuestring) : NULL;
        file_data.file_name = file_name ? strdup(file_name->valuestring) : NULL;
        file_data.created_at = created_at ? created_at->valueint : 0;
        file_data.bytes = bytes ? bytes->valueint : 0;
    }
    resp->data = file_data;

    cJSON_Delete(json);
    free(chunk.memory);
    curl_slist_free_all(headers);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    return err;
}

void coze_free_files_upload_response(coze_files_upload_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_file(&resp->data);
}

coze_error_t coze_files_retrieve(const coze_files_retrieve_request_t *req,
                                 coze_files_retrieve_response_t *resp) {
    if (!req || !resp || !req->api_token || !req->file_id) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path), "/v1/files/retrieve?file_id=%s", req->file_id);


    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "GET", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_file_t file_data = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const cJSON *id = cJSON_GetObjectItem(data, "id");
        const cJSON *bytes = cJSON_GetObjectItem(data, "bytes");
        const cJSON *created_at = cJSON_GetObjectItem(data, "created_at");
        const cJSON *file_name = cJSON_GetObjectItem(data, "file_name");

        file_data.id = id ? strdup(id->valuestring) : NULL;
        file_data.file_name = file_name ? strdup(file_name->valuestring) : NULL;
        file_data.created_at = created_at ? created_at->valueint : 0;
        file_data.bytes = bytes ? bytes->valueint : 0;
    }
    resp->data = file_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_files_retrieve_response(coze_files_retrieve_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    coze_free_file(&resp->data);
}


coze_error_t coze_workflows_runs_create(const coze_workflows_runs_create_request_t *req,
                                        coze_workflows_runs_create_response_t *resp) {
    if (!req || !resp || !req->api_token || !req->workflow_id) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    char path[512];
    snprintf(path, sizeof(path), "/v1/workflow/run");


    cJSON *body = cJSON_CreateObject();
    if (req->workflow_id) {
        cJSON_AddStringToObject(body, "workflow_id", req->workflow_id);
    }
    if (req->bot_id) {
        cJSON_AddStringToObject(body, "bot_id", req->bot_id);
    }
    if (req->is_async) {
        cJSON_AddBoolToObject(body, "is_async", req->is_async);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);


    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;
    free(json_body);
    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_workflow_run_result_t workflow_run_result = {0};
    const cJSON *data = cJSON_GetObjectItem(json, "data");
    const cJSON *debug_url = cJSON_GetObjectItem(json, "debug_url");
    const cJSON *execute_id = cJSON_GetObjectItem(json, "execute_id");

    workflow_run_result.data = data ? strdup(data->valuestring) : NULL;
    workflow_run_result.debug_url = debug_url ? strdup(debug_url->valuestring) : NULL;
    workflow_run_result.execute_id = execute_id ? strdup(execute_id->valuestring) : NULL;
    resp->data = workflow_run_result;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_workflows_runs_create_response(coze_workflows_runs_create_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    if (resp->data.data) {
        free((void *) resp->data.data);
    }
    if (resp->data.debug_url) {
        free((void *) resp->data.debug_url);
    }
    if (resp->data.execute_id) {
        free((void *) resp->data.execute_id);
    }
}

// Remove leading and trailing whitespace from a string
void trim_whitespace(char *str) {
    if (!str) return;

    // Skip leading whitespace
    char *start = str;
    while (*start && (*start == ' ' || *start == '\n' || *start == '\r' || *start == '\t')) {
        start++;
    }

    // If string is all whitespace
    if (!*start) {
        str[0] = '\0';
        return;
    }

    // Move string to beginning if needed
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t')) {
        *end = '\0';
        end--;
    }
}


struct WorkflowSSECallbackContext {
    void (*callback)(const coze_workflow_event_t *workflow_event);
};

void workflow_stream_handler(const char *data, void *biz_ctx) {
    printf("[coze_api] workflows.runs sse event: %s\n", data);

    const struct WorkflowSSECallbackContext *ctx = (struct WorkflowSSECallbackContext *) biz_ctx;

    char *id = NULL;
    char *event = NULL;
    char *sse_data = NULL;
    char *line = (char *) data;
    char *next_line;

    // Parse event: or data: prefix
    char *saveptr;
    char *line_copy = strdup(line);
    char *curr_line = strtok_r(line_copy, "\n", &saveptr);
    while (curr_line) {
        if (strncmp(curr_line, "id:", 3) == 0) {
            id = strdup(curr_line + 3);
            trim_whitespace(id);
        } else if (strncmp(curr_line, "event:", 6) == 0) {
            event = strdup(curr_line + 6);
            trim_whitespace(event);
        } else if (strncmp(curr_line, "data:", 5) == 0) {
            sse_data = strdup(curr_line + 5);
            while (sse_data[0] == ' ')
                memmove(sse_data, sse_data + 1, strlen(sse_data));
        }

        curr_line = strtok_r(NULL, "\n", &saveptr);
    }
    free(line_copy);

    if (!event || !sse_data || !ctx || !ctx->callback) {
        if (id) free(id);
        if (event) free(event);
        if (sse_data) free(sse_data);
        return;
    }

    // 在堆上创建事件
    coze_workflow_event_t *event_data = calloc(1, sizeof(coze_workflow_event_t));
    event_data->id = id ? strdup(id) : NULL;
    event_data->event = event ? strdup(event) : NULL;

    if (strcmp(event, COZE_WORKFLOW_EVENT_TYPE_DONE) == 0) {
        // return;
    } else if (strcmp(event, COZE_WORKFLOW_EVENT_TYPE_MESSAGE) == 0) {
        cJSON *json_data = cJSON_Parse(sse_data);
        if (json_data) {
            coze_workflow_event_message_t *message = calloc(1, sizeof(coze_workflow_event_message_t));
            const cJSON *content = cJSON_GetObjectItem(json_data, "content");
            const cJSON *node_title = cJSON_GetObjectItem(json_data, "node_title");
            const cJSON *node_seq_id = cJSON_GetObjectItem(json_data, "node_seq_id");
            const cJSON *node_is_finish = cJSON_GetObjectItem(json_data, "node_is_finish");
            // const cJSON *ext = cJSON_GetObjectItem(json_data, "ext");

            message->content = content ? strdup(content->valuestring) : NULL;
            message->node_title = node_title ? strdup(node_title->valuestring) : NULL;
            message->node_seq_id = node_seq_id ? strdup(node_seq_id->valuestring) : NULL;
            message->node_is_finish = node_is_finish ? node_is_finish->valueint : 0;
            // message->ext = ext ? strdup(ext->valuestring) : NULL;

            cJSON_Delete(json_data);
            event_data->message = message;
        }
    } else if (strcmp(event, COZE_WORKFLOW_EVENT_TYPE_ERROR) == 0) {
        cJSON *json_data = cJSON_Parse(sse_data);
        if (json_data) {
            coze_workflow_event_error_t *error = calloc(1, sizeof(coze_workflow_event_error_t));
            const cJSON *error_code = cJSON_GetObjectItem(json_data, "error_code");
            const cJSON *error_message = cJSON_GetObjectItem(json_data, "error_message");

            error->error_code = error_code ? error_code->valueint : 0;
            error->error_message = error_message ? strdup(error_message->valuestring) : NULL;

            cJSON_Delete(json_data);
            event_data->error = error;
        }
    } else if (strcmp(event, COZE_WORKFLOW_EVENT_TYPE_INTERRUPT) == 0) {
        cJSON *json_data = cJSON_Parse(sse_data);
        if (json_data) {
            coze_workflow_event_interrupt_t *interrupt = calloc(1, sizeof(coze_workflow_event_interrupt_t));
            const cJSON *interrupt_data = cJSON_GetObjectItem(json_data, "interrupt_data");
            const cJSON *node_title = cJSON_GetObjectItem(json_data, "node_title");

            if (interrupt_data) {
                coze_workflow_event_interrupt_data_t *interrupt_data_data = calloc(
                    1, sizeof(coze_workflow_event_interrupt_data_t));
                interrupt_data_data->event_id = cJSON_GetObjectItem(interrupt_data, "event_id")
                                                    ? strdup(
                                                        cJSON_GetObjectItem(interrupt_data, "event_id")->valuestring)
                                                    : NULL;
                interrupt_data_data->type = cJSON_GetObjectItem(interrupt_data, "type")
                                                ? cJSON_GetObjectItem(interrupt_data, "type")->valueint
                                                : 0;
                interrupt->interrupt_data = interrupt_data_data;
            }
            interrupt->node_title = node_title ? strdup(node_title->valuestring) : NULL;

            cJSON_Delete(json_data);
            event_data->interrupt = interrupt;
        }
    }

    if (ctx->callback) {
        ctx->callback(event_data);
    }

    // 释放临时数据
    if (id) free(id);
    if (event) free(event);
    if (sse_data) free(sse_data);
}

coze_error_t coze_workflows_runs_stream(const coze_workflows_runs_stream_request_t *req,
                                        coze_workflows_runs_stream_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }


    const char *path = "/v1/workflow/stream_run";


    cJSON *body = cJSON_CreateObject();
    if (req->workflow_id) {
        cJSON_AddStringToObject(body, "workflow_id", req->workflow_id);
    }
    if (req->bot_id) {
        cJSON_AddStringToObject(body, "bot_id", req->bot_id);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    struct WorkflowSSECallbackContext biz_ctx = {
        .callback = req->on_event,
    };
    const coze_error_t err = make_http_sse_request(req->api_base, req->api_token, path, "POST", json_body,
                                                   workflow_stream_handler,
                                                   &biz_ctx,
                                                   &coze_response);
    resp->response = coze_response;
    free(json_body);
    if (err != COZE_OK) {
        return err;
    }

    resp->code = 0;
    resp->msg = "";

    if (err != COZE_OK) {
        return err;
    }

    return COZE_OK;
}

void coze_free_workflows_runs_stream_response(coze_workflows_runs_stream_response_t *resp) {
    if (!resp) return;

    if (resp->msg && strlen(resp->msg) > 0) {
        free((void *) resp->msg);
    }
    coze_free_response(&resp->response);
}

coze_error_t coze_workflows_runs_resume(const coze_workflows_runs_resume_request_t *req,
                                        coze_workflows_runs_resume_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    const char *path = "/v1/workflow/stream_resume";


    cJSON *body = cJSON_CreateObject();
    if (req->workflow_id) {
        cJSON_AddStringToObject(body, "workflow_id", req->workflow_id);
    }
    if (req->bot_id) {
        cJSON_AddStringToObject(body, "bot_id", req->bot_id);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);

    coze_response_t coze_response = {0};
    struct WorkflowSSECallbackContext biz_ctx = {
        .callback = req->on_event,
    };
    const coze_error_t err = make_http_sse_request(req->api_base, req->api_token, path, "POST", json_body,
                                                   workflow_stream_handler,
                                                   &biz_ctx,
                                                   &coze_response);
    resp->response = coze_response;
    free(json_body);
    if (err != COZE_OK) {
        return err;
    }

    resp->code = 0;
    resp->msg = "";

    if (err != COZE_OK) {
        return err;
    }

    return COZE_OK;
}

void coze_free_workflows_runs_resume_response(coze_workflows_runs_resume_response_t *resp) {
    if (!resp) return;

    if (resp->msg && strlen(resp->msg) > 0) {
        free((void *) resp->msg);
    }
    coze_free_response(&resp->response);
}

coze_error_t coze_audio_voices_list(const coze_audio_voices_list_request_t *req,
                                    coze_audio_voices_list_response_t *resp) {
    if (!req || !resp || !req->api_token) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    const int page_size = req->page_size ? req->page_size : 100;
    const int page_num = req->page_num ? req->page_num : 1;
    const char *filter_system_voice = req->filter_system_voice ? "true" : "false";

    char path[512];
    snprintf(path, sizeof(path), "/v1/audio/voices?page_size=%d&page_num=%d&filter_system_voice=%s", page_size,
             page_num, filter_system_voice);

    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "GET", NULL, &chunk, &coze_response);
    resp->response = coze_response;

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    coze_audio_voices_list_data_t voices_data = {0};
    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const cJSON *has_more = cJSON_GetObjectItem(data, "has_more");
        const cJSON *voice_list = cJSON_GetObjectItem(data, "voice_list");
        const int voices_count = cJSON_GetArraySize(voice_list);

        voices_data.voices_count = voices_count;
        voices_data.voices = calloc(voices_count, sizeof(coze_voice_t));
        voices_data.has_more = has_more ? has_more->valueint : 0;

        for (int i = 0; i < voices_count; i++) {
            const cJSON *voice = cJSON_GetArrayItem(voice_list, i);
            if (voice) {
                const cJSON *preview_audio = cJSON_GetObjectItem(voice, "preview_audio");
                const cJSON *language_name = cJSON_GetObjectItem(voice, "language_name");
                const cJSON *is_system_voice = cJSON_GetObjectItem(voice, "is_system_voice");
                const cJSON *preview_text = cJSON_GetObjectItem(voice, "preview_text");
                const cJSON *create_time = cJSON_GetObjectItem(voice, "create_time");
                const cJSON *update_time = cJSON_GetObjectItem(voice, "update_time");
                const cJSON *name = cJSON_GetObjectItem(voice, "name");
                const cJSON *language_code = cJSON_GetObjectItem(voice, "language_code");
                const cJSON *voice_id = cJSON_GetObjectItem(voice, "voice_id");
                const cJSON *available_training_times = cJSON_GetObjectItem(voice, "available_training_times");

                voices_data.voices[i].preview_audio = preview_audio ? strdup(preview_audio->valuestring) : NULL;
                voices_data.voices[i].language_name = language_name ? strdup(language_name->valuestring) : NULL;
                voices_data.voices[i].is_system_voice = is_system_voice ? is_system_voice->valueint : 0;
                voices_data.voices[i].preview_text = preview_text ? strdup(preview_text->valuestring) : NULL;
                voices_data.voices[i].create_time = create_time ? create_time->valueint : 0;
                voices_data.voices[i].update_time = update_time ? update_time->valueint : 0;
                voices_data.voices[i].name = name ? strdup(name->valuestring) : NULL;
                voices_data.voices[i].language_code = language_code ? strdup(language_code->valuestring) : NULL;
                voices_data.voices[i].voice_id = voice_id ? strdup(voice_id->valuestring) : NULL;
                voices_data.voices[i].available_training_times = available_training_times
                                                                     ? available_training_times->valueint
                                                                     : 0;
            }
        }
    }
    resp->data = voices_data;

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_audio_voices_list_response(coze_audio_voices_list_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    for (int i = 0; i < resp->data.voices_count; i++) {
        coze_free_voice(&resp->data.voices[i]);
    }
    free(resp->data.voices);
}

coze_error_t coze_audio_rooms_create(const coze_audio_rooms_create_request_t *req,
                                     coze_audio_rooms_create_response_t *resp) {
    if (!req || !resp || !req->api_token || !req->bot_id) {
        return COZE_ERROR_INVALID_PARAM;
    }

    struct MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    const char *path = "/v1/audio/rooms";

    cJSON *body = cJSON_CreateObject();
    if (req->bot_id) {
        cJSON_AddStringToObject(body, "bot_id", req->bot_id);
    }
    if (req->conversation_id) {
        cJSON_AddStringToObject(body, "conversation_id", req->conversation_id);
    }
    if (req->voice_id) {
        cJSON_AddStringToObject(body, "voice_id", req->voice_id);
    }
    char *json_body = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);


    coze_response_t coze_response = {0};
    coze_error_t err = make_http_request(req->api_base, req->api_token, path, "POST", json_body, &chunk,
                                         &coze_response);
    resp->response = coze_response;
    free(json_body);

    if (err != COZE_OK) {
        free(chunk.memory);
        return err;
    }

    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        free(chunk.memory);
        return COZE_ERROR_API;
    }

    char *tmp_msg = NULL;
    err = parse_response_code(json, &tmp_msg, &resp->code);
    resp->msg = tmp_msg;
    if (err != COZE_OK) {
        cJSON_Delete(json);
        free(chunk.memory);
        return err;
    }

    // 解析数据
    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (data) {
        const cJSON *room_id = cJSON_GetObjectItem(data, "room_id");
        const cJSON *app_id = cJSON_GetObjectItem(data, "app_id");
        const cJSON *token = cJSON_GetObjectItem(data, "token");
        const cJSON *uid = cJSON_GetObjectItem(data, "uid");

        resp->data.room_id = room_id ? strdup(room_id->valuestring) : NULL;
        resp->data.app_id = app_id ? strdup(app_id->valuestring) : NULL;
        resp->data.token = token ? strdup(token->valuestring) : NULL;
        resp->data.uid = uid ? strdup(uid->valuestring) : NULL;
    }

    cJSON_Delete(json);
    free(chunk.memory);

    return COZE_OK;
}

void coze_free_audio_rooms_create_response(coze_audio_rooms_create_response_t *resp) {
    if (!resp) return;

    free((void *) resp->msg);
    coze_free_response(&resp->response);
    free((void *) resp->data.token);
    free((void *) resp->data.room_id);
    free((void *) resp->data.app_id);
    free((void *) resp->data.uid);
}

void coze_free_response(coze_response_t *resp) {
    if (!resp) return;

    free((void *) resp->logid);
}

void coze_free_bot(coze_bot_t *bot) {
    if (!bot) return;

    if (bot->bot_id) {
        free((void *) bot->bot_id);
    }
    if (bot->name) {
        free((void *) bot->name);
    }
    if (bot->description) {
        free((void *) bot->description);
    }
    if (bot->icon_url) {
        free((void *) bot->icon_url);
    }
    if (bot->version) {
        free((void *) bot->version);
    }
}

void coze_free_conversation(coze_conversation_t *conversation) {
    if (!conversation) return;

    free((void *) conversation->id);
    free((void *) conversation->last_section_id);
}

void coze_free_message(coze_message_t *message) {
    if (!message) return;

    free((void *) message->id);
    free((void *) message->conversation_id);
    free((void *) message->bot_id);
    free((void *) message->chat_id);
    free((void *) message->role);
    free((void *) message->content);
    free((void *) message->content_type);
    free((void *) message->type);
}

void coze_free_chat(coze_chat_t *chat) {
    if (!chat) return;

    free((void *) chat->id);
    free((void *) chat->conversation_id);
    free((void *) chat->bot_id);
    free((void *) chat->status);
    free((void *) chat->last_error.msg);
    free((void *) chat->required_action.type);
    for (int i = 0; i < chat->required_action.submit_tool_outputs.tool_calls_count; i++) {
        free((void *) chat->required_action.submit_tool_outputs.tool_calls[i].function->name);
        free((void *) chat->required_action.submit_tool_outputs.tool_calls[i].function->arguments);
    }
    free(chat->required_action.submit_tool_outputs.tool_calls);
}

void coze_free_file(coze_file_t *file) {
    if (!file) return;

    free((void *) file->id);
    free((void *) file->file_name);
}

void coze_free_voice(coze_voice_t *voice) {
    if (!voice) return;


    if (voice->preview_audio && strlen(voice->preview_audio) > 0) {
        free((void *) voice->preview_audio);
    }
    if (voice->language_name && strlen(voice->language_name) > 0) {
        free((void *) voice->language_name);
    }
    if (voice->preview_text && strlen(voice->preview_text) > 0) {
        free((void *) voice->preview_text);
    }
    if (voice->name && strlen(voice->name) > 0) {
        free((void *) voice->name);
    }
    if (voice->language_code && strlen(voice->language_code) > 0) {
        free((void *) voice->language_code);
    }
    if (voice->voice_id && strlen(voice->voice_id) > 0) {
        free((void *) voice->voice_id);
    }
}

void coze_free_oauth_token(coze_oauth_token_t *token) {
    if (!token) return;

    if (token->access_token && strlen(token->access_token) > 0) {
        free((void *) token->access_token);
    }
    if (token->refresh_token && strlen(token->refresh_token) > 0) {
        free((void *) token->refresh_token);
    }
    if (token->token_type && strlen(token->token_type) > 0) {
        free((void *) token->token_type);
    }
}

void coze_free_bots_list_data(coze_bots_list_data_t *data) {
    if (!data) return;

    for (int i = 0; i < data->space_bot_count; i++) {
        free((void *) data->space_bots[i].bot_id);
        free((void *) data->space_bots[i].bot_name);
        free((void *) data->space_bots[i].description);
        free((void *) data->space_bots[i].icon_url);
        free((void *) data->space_bots[i].publish_time);
    }

    free(data->space_bots);
}
