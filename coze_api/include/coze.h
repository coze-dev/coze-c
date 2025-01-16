#ifndef COZE_H
#define COZE_H

#include <stdbool.h>

// *** coze common ***
typedef enum {
    COZE_OK = 0, // 成功
    COZE_ERROR_INVALID_PARAM, // 无效参数
    COZE_ERROR_NETWORK, // 网络错误
    COZE_ERROR_API, // API 错误
    COZE_ERROR_MEMORY // 内存分配错误
} coze_error_t;

typedef struct {
    const char *logid; // x-tt-logid header 值
} coze_response_t;

// *** coze common ***
// *** enum ***

// Indicates that the content of the message is sent by the user.
#define COZE_MESSAGE_ROLE_USER "user"
// Indicates that the content of the message is sent by the bot.
#define COZE_MESSAGE_ROLE_ASSISTANT "assistant"

// User input content.  
#define COZE_MESSAGE_TYPE_QUESTION "question"
// The message content returned by the Bot to the user, supporting incremental return. If the workflow is bound to a message node, there may be multiple answer scenarios, and the end flag of the streaming return can be used to determine that all answers are completed.
#define COZE_MESSAGE_TYPE_ANSWER "answer"
// Intermediate results of the function (function call) called during the Bot conversation process.
#define COZE_MESSAGE_TYPE_FUNCTION_CALL "function_call"
// Results returned after calling the tool (function call).
#define COZE_MESSAGE_TYPE_TOOL_OUTPUT "tool_output"
// Results returned after calling the tool (function call).
#define COZE_MESSAGE_TYPE_TOOL_RESPONSE "tool_response"
// If the user question suggestion switch is turned on in the Bot configuration, the reply content related to the recommended questions will be returned.
#define COZE_MESSAGE_TYPE_FOLLOW_UP "follow_up"
// In the scenario of multiple answers, the server will return a verbose package, and the corresponding content is in JSON format. content.msg_type = generate_answer_finish represents that all answers have been replied to.
#define COZE_MESSAGE_TYPE_VERBOSE "verbose"

// Text.    
#define COZE_MESSAGE_CONTENT_TYPE_TEXT "text"
// Multimodal content, that is, a combination of text and files, or a combination of text and images.
#define COZE_MESSAGE_CONTENT_TYPE_OBJECT_STRING "object_string"
// message card. This enum value only appears in the interface response and is not supported as an input parameter.
#define COZE_MESSAGE_CONTENT_TYPE_CARD "card"
// If there is a voice message in the input message, the conversation.audio.delta event will be returned in the
// streaming response event. The data of this event corresponds to the Message Object. The content_type is audio,
// and the content is a PCM audio clip with a sampling rate of 24kHz, raw 16 bit, 1 channel, little-endian.
#define COZE_MESSAGE_CONTENT_TYPE_AUDIO "audio"

// The content type of the multimodal message.
#define COZE_MESSAGE_OBJECT_STRING_TYPE_TEXT "text"
#define COZE_MESSAGE_OBJECT_STRING_TYPE_FILE "file"
#define COZE_MESSAGE_OBJECT_STRING_TYPE_IMAGE "image"
#define COZE_MESSAGE_OBJECT_STRING_TYPE_AUDIO "audio"

// The running status of the session
// The session has been created
#define COZE_CHAT_STATUS_CREATED "created"
// The Bot is processing
#define COZE_CHAT_STATUS_COMPLETED "completed"
// The Bot has finished processing, and the session has ended
#define COZE_CHAT_STATUS_FAILED "failed"
// The session is interrupted and requires further processing
#define COZE_CHAT_STATUS_REQUIRES_ACTION "requires_action"

// Event for creating a conversation, indicating the start of the conversation.
#define COZE_EVENT_TYPE_CONVERSATION_CHAT_CREATED "conversation.chat.created"
// The server is processing the conversation.
#define COZE_EVENT_TYPE_CONVERSATION_CHAT_IN_PROGRESS "conversation.chat.in_progress"
// Incremental message, usually an incremental message when type=answer.
#define COZE_EVENT_TYPE_CONVERSATION_MESSAGE_DELTA "conversation.message.delta"
// The message has been completely replied to. At this point, the streaming package contains the spliced results of all message.delta, and each message is in a completed state.
#define COZE_EVENT_TYPE_CONVERSATION_MESSAGE_COMPLETED "conversation.message.completed"
// The conversation is completed.
#define COZE_EVENT_TYPE_CONVERSATION_CHAT_COMPLETED "conversation.chat.completed"
// This event is used to mark a failed conversation.
#define COZE_EVENT_TYPE_CONVERSATION_CHAT_FAILED "conversation.chat.failed"
// The conversation is interrupted and requires the user to report the execution results of the tool.
#define COZE_EVENT_TYPE_CONVERSATION_CHAT_REQUIRES_ACTION "conversation.chat.requires_action"
// If there is a voice message in the input message, the conversation.audio.delta event will be returned in the
// streaming response event. The data of this event corresponds to the Message Object. The content_type is audio,
// and the content is a PCM audio clip with a sampling rate of 24kHz, raw 16 bit, 1 channel, little-endian.
#define COZE_EVENT_TYPE_CONVERSATION_AUDIO_DELTA "conversation.audio.delta"
// Error events during the streaming response process. For detailed explanations of code and msg, please refer to Error codes.
#define COZE_EVENT_TYPE_ERROR "error"
// The streaming response for this session ended normally.
#define COZE_EVENT_TYPE_DONE "done"

// 倒序
#define COZE_ORDER_DESC "desc"
// 正序
#define COZE_ORDER_ASC "asc"

// chat.status
#define COZE_CHAT_STATUS_CREATED "created" // 对话已创建。
#define COZE_CHAT_STATUS_IN_PROGRESS "in_progress" // 智能体正在处理中。
#define COZE_CHAT_STATUS_COMPLETED "completed" // 智能体已完成处理，本次对话结束。
#define COZE_CHAT_STATUS_FAILED "failed"
#define COZE_CHAT_STATUS_REQUIRES_ACTION "requires_action" // 对话中断，需要进一步处理。
#define COZE_CHAT_STATUS_CANCELED "canceled" // 对话已取消。

// required_action.type
#define COZE_CHAT_REQUIRED_ACTION_TYPE_SUBMIT_TOOL_OUTPUTS "submit_tool_outputs"

// workflows.runs.stream
// The output message from the workflow node, such as the output message from
// the message node or end node. You can view the specific message content in data.
// 工作流节点输出消息，例如消息节点、结束节点的输出消息。可以在 data 中查看具体的消息内容。
#define COZE_WORKFLOW_EVENT_TYPE_MESSAGE "Message"

// An error has occurred. You can view the error_code and error_message in data to
// troubleshoot the issue.
// 报错。可以在 data 中查看 error_code 和 error_message，排查问题。
#define COZE_WORKFLOW_EVENT_TYPE_ERROR "Error"

// End. Indicates the end of the workflow execution, where data is empty.
// 结束。表示工作流执行结束，此时 data 为空。
#define COZE_WORKFLOW_EVENT_TYPE_DONE "Done"

// Interruption. Indicates the workflow has been interrupted, where the data field
// contains specific interruption information.
// 中断。表示工作流中断，此时 data 字段中包含具体的中断信息。
#define COZE_WORKFLOW_EVENT_TYPE_INTERRUPT "Interrupt"

// *** common ***

typedef struct {
    // enter message
    const char *role; // COZE_MESSAGE_ROLE_USER, COZE_MESSAGE_ROLE_ASSISTANT
    const char *type; // COZE_MESSAGE_TYPE_QUESTION, COZE_MESSAGE_TYPE_ANSWER
    const char *content; // 消息内容
    const char *content_type; // COZE_MESSAGE_CONTENT_TYPE_TEXT, COZE_MESSAGE_CONTENT_TYPE_OBJECT_STRING
    // const char *meta_data; // 附加消息, map[string]string

    // response message
    const char *id; // 消息 ID
    const char *conversation_id; // 会话 ID
    const char *bot_id; // Bot ID
    const char *chat_id; // 聊天 ID
    long created_at; // 创建时间
    long updated_at; // 更新时间
} coze_message_t;

// *** common ***

// *** auth.web_oauth.get_oauth_url ***

typedef struct {
    const char *client_id; // 客户端 ID
    const char *redirect_uri; // 重定向 URI
    const char *state; // 状态
    const char *workspace_id; // 工作空间 ID
} coze_web_oauth_get_oauth_url_request_t;

// *** auth.web_oauth.get_oauth_url ***

// *** auth.web_oauth.get_access_token ***

typedef struct {
    const char *access_token; // 访问令牌
    long expires_in; // 过期时间, unix 时间戳
    const char *refresh_token; // 刷新令牌
    const char *token_type; // 令牌类型, Bearer
} coze_oauth_token_t;

typedef struct {
    const char *api_base; // API 基础 URL

    const char *client_id; // 客户端 ID
    const char *client_secret; // 客户端密钥
    const char *redirect_uri; // 重定向 URI
    const char *code; // 授权码
} coze_web_oauth_get_access_token_request_t;

typedef struct {
    int code; // 返回码
    const char *msg; // 返回信息
    coze_oauth_token_t data; // 返回数据
    coze_response_t response; // 返回响应
} coze_web_oauth_get_access_token_response_t;

// *** auth.web_oauth.get_access_token ***

// *** auth.web_oauth.refresh_access_token ***

typedef struct {
    int code; // 返回码
    const char *msg; // 返回信息
    coze_oauth_token_t data; // 返回数据
    coze_response_t response; // 返回响应
} coze_web_oauth_refresh_access_token_response_t;

typedef struct {
    const char *api_base; // API 基础 URL

    const char *client_id; // 客户端 ID
    const char *client_secret; // 客户端密钥
    const char *refresh_token; // 刷新令牌
} coze_web_oauth_refresh_access_token_request_t;

// *** auth.web_oauth.refresh_access_token ***

// *** bots.create ***

typedef struct {
    const char *model_id; // 模型 ID
    const char *model_name; // 模型名称
} coze_bots_model_info_t;

typedef struct {
    const char *api_id; // API ID
    const char *name; // API 名称
    const char *description; // API 描述
} coze_bots_plugin_info_api_info_t;

typedef struct {
    const char *plugin_id; // 插件 ID
    const char *name; // 插件名称
    const char *description; // 插件描述
    const char *icon_url; // 插件图标 URL
    coze_bots_plugin_info_api_info_t *api_info_list; // API 信息列表
    int api_info_count; // API 信息数量
} coze_bots_plugin_info_t;

typedef struct {
    const char *prompt; // 提示词
} coze_bots_prompt_info_t;

typedef struct {
    const char *prologue; // 开场白
    const char **suggested_questions; // 建议问题列表
    int suggested_questions_count; // 建议问题数量
} coze_bots_onboarding_info_t;

typedef struct {
    const char *bot_id; // Bot ID
    const char *name; // Bot 名称
    const char *description; // Bot 描述
    const char *icon_url; // 图标 URL
    long create_time; // 创建时间
    long update_time; // 更新时间
    const char *version; // 版本
    coze_bots_prompt_info_t prompt_info; // 提示词信息
    coze_bots_onboarding_info_t onboarding_info; // 引导信息
    int bot_mode; // Bot 模式
    coze_bots_model_info_t model_info; // 模型信息
    coze_bots_plugin_info_t *plugin_info_list; // 插件信息列表
    int plugin_count; // 插件数量
} coze_bot_t;

typedef struct {
    int code;
    const char *msg;
    coze_bot_t data;
    coze_response_t response;
} coze_bots_create_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *space_id; // 工作空间 ID
    const char *name; // Bot 名称
    const char *description; // Bot 描述
    const char *icon_file_id; // 图标文件 ID
    coze_bots_prompt_info_t *prompt_info; // 提示词信息
    coze_bots_onboarding_info_t *onboarding_info; // 引导信息
} coze_bots_create_request_t;

// *** bots.create ***

// *** bots.update ***

typedef struct {
    int code;
    const char *msg;
    coze_response_t response;
} coze_bots_update_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *bot_id; // Bot ID
    const char *name; // Bot 名称
    const char *description; // Bot 描述
    const char *icon_file_id; // 图标文件 ID
    coze_bots_prompt_info_t *prompt_info; // 提示词信息
    coze_bots_onboarding_info_t *onboarding_info; // 引导信息
} coze_bots_update_request_t;

// *** bots.update ***

// *** bots.publish ***

typedef struct {
    int code;
    const char *msg;
    coze_bot_t data;
    coze_response_t response;
} coze_bots_publish_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *bot_id; // Bot ID
    const char **connector_ids; // 连接器 ID 列表
    int connector_ids_count; // 连接器 ID 数量
} coze_bots_publish_request_t;

// *** bots.publish ***

// *** bots.list ***

typedef struct {
    const char *bot_id; // Bot ID
    const char *bot_name; // Bot 名称
    const char *description; // Bot 描述
    const char *icon_url; // 图标 URL
    const char *publish_time; // 发布时间
} coze_simple_bot_t;

typedef struct {
    coze_simple_bot_t *space_bots;
    int space_bot_count;
    int total;
} coze_bots_list_data_t;

typedef struct {
    int code;
    const char *msg;
    coze_bots_list_data_t data;
    coze_response_t response;
} coze_bots_list_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *space_id; // 空间 ID
    int page_num; // 页码，从 1 开始
    int page_size; // 每页数量
} coze_bots_list_request_t;

// *** bots.list ***

// *** bots.retrieve ***

typedef struct {
    int code;
    const char *msg;
    coze_bot_t data;
    coze_response_t response;
} coze_bots_retrieve_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *bot_id; // Bot ID
} coze_bots_retrieve_request_t;

// *** bots.retrieve ***

// *** workspaces list ***

typedef struct {
    const char *id; // 工作空间 ID
    const char *name; // 工作空间名称
    const char *icon_url; // 图标 URL
    const char *role_type; // 角色类型
    const char *workspace_type; // 工作空间类型
} coze_workspace_t;

typedef struct {
    coze_workspace_t *workspaces; // 工作空间列表
    int workspace_count; // 工作空间数量
    int total_count; // 总数
} coze_workspaces_data_t;

typedef struct {
    int code; // 返回码
    const char *msg; // 返回信息
    coze_workspaces_data_t data; // 返回数据
    coze_response_t response;
} coze_workspaces_list_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    int page_num; // 页码，从 1 开始
    int page_size; // 每页数量
} coze_workspaces_list_request_t;

// *** workspaces list ***

// *** conversation ***

typedef struct {
    const char *id; // 会话 ID
    long created_at; // 创建时间
    const char *last_section_id; // 最后一个 section ID
    // const char *meta_data;
} coze_conversation_t;

// *** conversation ***

// *** conversations.create ***

typedef struct {
    int code;
    const char *msg;
    coze_conversation_t data;
    coze_response_t response;
} coze_conversations_create_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn


    const char *bot_id; // Bot ID
    coze_message_t *messages; // 消息列表
    int message_count; // 消息数量
    // const char *meta_data; // 元数据，可选的键值对
} coze_conversations_create_request_t;

// *** conversations.create ***

// *** conversations.retrieve ***

typedef struct {
    int code;
    const char *msg;
    coze_conversation_t data;
    coze_response_t response;
} coze_conversations_retrieve_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
} coze_conversations_retrieve_request_t;

// *** conversations.retrieve ***

// *** conversations.messages.create ***

typedef struct {
    int code;
    const char *msg;
    coze_message_t data;
    coze_response_t response;
} coze_conversations_messages_create_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn


    const char *conversation_id; // 会话 ID
    const char *role; // COZE_MESSAGE_ROLE_USER 或 COZE_MESSAGE_ROLE_ASSISTANT
    const char *content; // 消息内容
    const char *content_type; // COZE_MESSAGE_CONTENT_TYPE_TEXT 或 COZE_MESSAGE_CONTENT_TYPE_OBJECT_STRING
    // const char *meta_data; // 元数据，可选的键值对
} coze_conversations_messages_create_request_t;

// *** conversations.messages.create ***

// *** conversations.messages.list ***

typedef struct {
    coze_message_t *messages; // array of messages
    int messages_count; // number of messages
    const char *first_id; // 返回的消息列表中，第一条消息的 Message ID。
    const char *last_id; // 返回的消息列表中，最后一条消息的 Message ID。
    bool has_more; // 是否已返回全部消息。
} coze_conversations_messages_list_data_t;

typedef struct {
    int code;
    const char *msg;
    coze_conversations_messages_list_data_t data;
    coze_response_t response;
} coze_conversations_messages_list_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *order; // COZE_ORDER_DESC 或 COZE_ORDER_ASC
    const char *chat_id; // 待查看的 Chat ID。
    const char *before_id; // 查看指定位置之前的消息。
    const char *after_id; // 查看指定位置之后的消息。
    int limit; // 每次查询返回的数据量。默认为 50，取值范围为 1~50。
} coze_conversations_messages_list_request_t;

// *** conversations.messages.list ***

// *** conversations.messages.retrieve ***

typedef struct {
    int code;
    const char *msg;
    coze_message_t data;
    coze_response_t response;
} coze_conversations_messages_retrieve_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *message_id; // 消息 ID
} coze_conversations_messages_retrieve_request_t;

// *** conversations.messages.retrieve ***

// *** conversations.messages.update ***

typedef struct {
    int code;
    const char *msg;
    coze_message_t data;
    coze_response_t response;
} coze_conversations_messages_update_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *message_id; // 消息 ID
    const char *content; // 消息内容
    const char *content_type; // COZE_MESSAGE_CONTENT_TYPE_TEXT 或 COZE_MESSAGE_CONTENT_TYPE_OBJECT_STRING
} coze_conversations_messages_update_request_t;

// *** conversations.messages.update ***

// *** conversations.messages.delete ***

typedef struct {
    int code;
    const char *msg;
    coze_message_t data;
    coze_response_t response;
} coze_conversations_messages_delete_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *message_id; // 消息 ID
} coze_conversations_messages_delete_request_t;

// *** conversations.messages.delete ***

// *** chat.create ***

typedef struct {
    int code; // 返回码
    const char *msg; // 返回信息
} coze_last_error_t;

typedef struct {
    const char *name; // The name of the method.
    const char *arguments; // The parameters of the method.
} coze_chat_tool_call_function_t;

typedef struct {
    const char *id; // The ID for reporting the running results.
    const char *type; // The type of tool, with the enum value of function.
    coze_chat_tool_call_function_t *function; // The definition of the execution method function.
} coze_chat_tool_call_t;

typedef struct {
    coze_chat_tool_call_t *tool_calls; // Details of the specific reported information.
    int tool_calls_count; // 工具调用数量
} coze_chat_submit_tool_outputs_t;

typedef struct {
    const char *type; // COZE_CHAT_REQUIRED_ACTION_TYPE_SUBMIT_TOOL_OUTPUTS
    coze_chat_submit_tool_outputs_t submit_tool_outputs; // Details of the specific reported information.
} coze_chat_required_action_t;

typedef struct {
    int token_count;
    // The total number of Tokens consumed in this chat, including the consumption for both the input and output parts.
    int output_count; // The total number of Tokens consumed for the output part.
    int input_count; // The total number of Tokens consumed for the input part.
} coze_chat_usage_t;

typedef struct {
    const char *id; // 对话 ID
    const char *conversation_id; // 会话 ID
    const char *bot_id; // Bot ID
    long created_at; // 创建时间
    long completed_at; // 结束时间
    // const char *meta_data; // 元数据
    coze_last_error_t last_error; // 错误信息
    const char *status;
    // 状态: COZE_CHAT_STATUS_CREATED, COZE_CHAT_STATUS_IN_PROGRESS, COZE_CHAT_STATUS_COMPLETED, COZE_CHAT_STATUS_FAILED, COZE_CHAT_STATUS_REQUIRES_ACTION, COZE_CHAT_STATUS_CANCELED
    coze_chat_required_action_t required_action; // 需要运行的信息详情
    coze_chat_usage_t usage; // Token 消耗的详细信息
} coze_chat_t;

typedef struct {
    int code;
    const char *msg;
    coze_chat_t data;
    coze_response_t response;
} coze_chat_create_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *bot_id; // Bot ID
    const char *user_id; // 用户 ID
    coze_message_t *additional_messages; // 消息内容
    int additional_messages_count; // 消息数量
} coze_chat_create_request_t;

// *** chat.create ***

// *** chat.stream ***

typedef struct {
    const char *event;
    // COZE_EVENT_TYPE_CONVERSATION_CHAT_CREATED, COZE_EVENT_TYPE_CONVERSATION_CHAT_IN_PROGRESS, COZE_EVENT_TYPE_CONVERSATION_CHAT_COMPLETED, COZE_EVENT_TYPE_CONVERSATION_CHAT_FAILED, COZE_EVENT_TYPE_CONVERSATION_CHAT_REQUIRES_ACTION, COZE_EVENT_TYPE_CONVERSATION_CHAT_CANCELED
    coze_chat_t *chat; // 对话信息
    coze_message_t *message; // 消息信息，可选
} coze_chat_event_t;

typedef struct {
    int code;
    const char *msg;
    coze_response_t response;
} coze_chat_stream_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *bot_id; // Bot ID
    const char *user_id; // 用户 ID
    coze_message_t *additional_messages; // 消息内容
    int additional_messages_count; // 消息数量
    bool auto_save_history; // 是否自动保存历史

    void (*on_event)(const coze_chat_event_t *event);
} coze_chat_stream_request_t;

// *** chat.stream ***

// *** chat.retrieve ***

typedef struct {
    int code;
    const char *msg;
    coze_chat_t data;
    coze_response_t response;
} coze_chat_retrieve_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *chat_id; // 对话 ID
} coze_chat_retrieve_request_t;

// *** chat.retrieve ***

// *** chat.messages.list ***

typedef struct {
    coze_message_t *messages; // 消息列表
    int messages_count; // 消息数量
} coze_chat_messages_list_data_t;

typedef struct {
    int code;
    const char *msg;
    coze_chat_messages_list_data_t data;
    coze_response_t response;
} coze_chat_messages_list_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *chat_id; // 对话 ID
} coze_chat_messages_list_request_t;

// *** chat.messages.list ***

// *** chat.submit_tool_outputs_create ***

typedef struct {
    const char *tool_call_id; // 工具调用 ID
    const char *output; // 工具输出
} coze_tool_output_t;

typedef struct {
    int code;
    const char *msg;
    coze_chat_t data;
    coze_response_t response;
} coze_chat_submit_tool_outputs_create_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *chat_id; // 对话 ID
    coze_tool_output_t *tool_outputs; // 工具输出列表
    int tool_outputs_count; // 工具输出数量
} coze_chat_submit_tool_outputs_create_request_t;

// *** chat.submit_tool_outputs_create ***

// *** chat.cancel ***

typedef struct {
    int code;
    const char *msg;
    coze_chat_t data;
    coze_response_t response;
} coze_chat_cancel_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *conversation_id; // 会话 ID
    const char *chat_id; // 对话 ID
} coze_chat_cancel_request_t;

// *** chat.cancel ***

// *** files.upload ***

typedef struct {
    const char *id; // 文件 ID
    const char *file_name; // 文件名
    int created_at; // 创建时间
    long bytes; // 文件大小
} coze_file_t;

typedef struct {
    int code;
    const char *msg;
    coze_file_t data;
    coze_response_t response;
} coze_files_upload_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *file; // 文件路径
} coze_files_upload_request_t;

// *** files.upload ***

// *** files.retrieve ***

typedef struct {
    int code;
    const char *msg;
    coze_file_t data;
    coze_response_t response;
} coze_files_retrieve_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *file_id; // 文件 ID
} coze_files_retrieve_request_t;

// *** files.retrieve ***

// *** workflows.runs.create ***

typedef struct {
    const char *data; // 执行结果
    const char *debug_url; // 调试 URL
    const char *execute_id; // 执行 ID
} coze_workflow_run_result_t;

typedef struct {
    int code;
    const char *msg;
    coze_workflow_run_result_t data;
    coze_response_t response;
} coze_workflows_runs_create_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *workflow_id; // 工作流 ID
    const char *bot_id; // Bot ID
    bool is_async; // 是否异步执行
} coze_workflows_runs_create_request_t;

// *** workflows.runs.create ***

// *** workflows.runs.stream ***

typedef struct {
    const char *content; // 流式输出的消息内容
    const char *node_title; // 输出消息的节点名称，例如消息节点、结束节点
    const char *node_seq_id; // 此消息在节点中的消息 ID，从 0 开始计数，例如消息节点的第 5 条消息
    bool node_is_finish; // 当前消息是否为此节点的最后一个数据包
    // const char *ext; // 额外字段
} coze_workflow_event_message_t;

typedef struct {
    const char *event_id; // 工作流中断事件 ID，恢复运行时应回传此字段。
    int type; // 工作流中断类型，恢复运行时应回传此字段。
} coze_workflow_event_interrupt_data_t;

typedef struct {
    coze_workflow_event_interrupt_data_t *interrupt_data; // 中断控制内容
    const char *node_title; // 输出消息的节点名称，例如“问答”
} coze_workflow_event_interrupt_t;

typedef struct {
    int error_code; // 调用状态码。0 表示调用成功。其他值表示调用失败。你可以通过 error_message 字段判断详细的错误原因。
    const char *error_message; // 状态信息。API 调用失败时可通过此字段查看详细错误信息。
} coze_workflow_event_error_t;

typedef struct {
    const char *id; // 事件 ID
    const char *event;
    // COZE_WORKFLOW_EVENT_TYPE_MESSAGE, COZE_WORKFLOW_EVENT_TYPE_ERROR, COZE_WORKFLOW_EVENT_TYPE_DONE, COZE_WORKFLOW_EVENT_TYPE_INTERRUPT
    coze_chat_t *chat; // 对话信息
    coze_workflow_event_message_t *message; // 消息信息，可选
    coze_workflow_event_interrupt_t *interrupt; // 中断控制信息
    coze_workflow_event_error_t *error; // 错误信息
} coze_workflow_event_t;

typedef struct {
    int code;
    const char *msg;
    coze_response_t response;
} coze_workflows_runs_stream_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *workflow_id; // 工作流 ID
    const char *bot_id; // Bot ID

    void (*on_event)(const coze_workflow_event_t *event);
} coze_workflows_runs_stream_request_t;

// *** workflows.runs.stream ***

// *** workflows.runs.resume ***

typedef struct {
    int code;
    const char *msg;
    coze_response_t response;
} coze_workflows_runs_resume_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *workflow_id; // 工作流 ID
    const char *bot_id; // Bot ID

    void (*on_event)(const coze_workflow_event_t *event);
} coze_workflows_runs_resume_request_t;

// *** workflows.runs.resume ***

// *** audio.voices.list ***

typedef struct {
    const char *preview_audio; // 预览音频 URL
    const char *language_name; // 语言名称
    bool is_system_voice; // 是否是系统语音
    const char *preview_text; // 预览文本
    int create_time; // 创建时间
    int update_time; // 更新时间
    const char *name; // 音色名称
    const char *language_code; // 语言代码
    const char *voice_id; // 语音 ID
    int available_training_times; // 当前音色还可以训练的次数
} coze_voice_t;

typedef struct {
    coze_voice_t *voices; // 语音列表
    int voices_count; // 语音数量
    bool has_more; // 是否还有更多数据
} coze_audio_voices_list_data_t;

typedef struct {
    int code;
    const char *msg;
    coze_audio_voices_list_data_t data;
    coze_response_t response;
} coze_audio_voices_list_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    bool filter_system_voice; // 是否过滤系统语音, 默认不过滤
    int page_num; // 页码, 从 1 开始
    int page_size; // 每页数量, 不传默认 100，传值需要(0, 100]
} coze_audio_voices_list_request_t;

// *** audio.voices.list ***

// *** audio.rooms.create ***

typedef struct {
    const char *room_id; // 房间 ID
    const char *app_id; // 应用 ID
    const char *token; // 房间 token
    const char *uid; // 用户 ID
} coze_audio_rooms_create_data_t;

typedef struct {
    int code;
    const char *msg;
    coze_audio_rooms_create_data_t data;
    coze_response_t response;
} coze_audio_rooms_create_response_t;

typedef struct {
    const char *api_token;
    const char *api_base; // default: api.coze.cn

    const char *bot_id; // Bot ID
    const char *conversation_id; // 会话 ID
    const char *voice_id; // 语音 ID
} coze_audio_rooms_create_request_t;

// *** audio.rooms.create ***

// auth - web_oauth

// Get Web OAuth URL
// 获取 Web OAuth 授权 URL

char *coze_web_oauth_get_oauth_url(const coze_web_oauth_get_oauth_url_request_t *req);

// Get Web OAuth Access Token
// 获取 Web OAuth 令牌

coze_error_t coze_web_oauth_get_access_token(const coze_web_oauth_get_access_token_request_t *req,
                                             coze_web_oauth_get_access_token_response_t *resp);

void coze_free_web_oauth_get_access_token_response(coze_web_oauth_get_access_token_response_t *resp);

// Refresh Web OAuth Access Token
// 刷新 Web OAuth 令牌

coze_error_t coze_web_oauth_refresh_access_token(const coze_web_oauth_refresh_access_token_request_t *req,
                                                 coze_web_oauth_refresh_access_token_response_t *resp);

void coze_free_web_oauth_refresh_access_token_response(coze_web_oauth_refresh_access_token_response_t *resp);


// workspaces

// List Workspaces
// 获取空间列表
coze_error_t coze_workspaces_list(const coze_workspaces_list_request_t *req, coze_workspaces_list_response_t *resp);

void coze_free_workspaces_list_response(coze_workspaces_list_response_t *resp);

// bots

// Create Bot
// 创建 Bot
coze_error_t coze_bots_create(const coze_bots_create_request_t *req, coze_bots_create_response_t *resp);

void coze_free_bots_create_response(coze_bots_create_response_t *resp);

// Update Bot
// 更新 Bot
coze_error_t coze_bots_update(const coze_bots_update_request_t *req, coze_bots_update_response_t *resp);

void coze_free_bots_update_response(coze_bots_update_response_t *resp);

// Publish Bot
// 发布 Bot
coze_error_t coze_bots_publish(const coze_bots_publish_request_t *req, coze_bots_publish_response_t *resp);

void coze_free_bots_publish_response(coze_bots_publish_response_t *resp);

// List Bots
// 获取 Bot 列表
coze_error_t coze_bots_list(const coze_bots_list_request_t *req, coze_bots_list_response_t *resp);

void coze_free_bots_list_response(coze_bots_list_response_t *resp);

// Retrieve Bot
// 获取 Bot 信息
coze_error_t coze_bots_retrieve(const coze_bots_retrieve_request_t *req, coze_bots_retrieve_response_t *resp);

void coze_free_bots_retrieve_response(coze_bots_retrieve_response_t *resp);


// conversations

// Create Conversation
// 创建会话
// docs en: https://www.coze.com/docs/developer_guides/create_conversation
// docs zh: https://www.coze.cn/docs/developer_guides/create_conversation
coze_error_t coze_conversations_create(const coze_conversations_create_request_t *req,
                                       coze_conversations_create_response_t *resp);

void coze_free_conversations_create_response(coze_conversations_create_response_t *resp);

// Retrieve Conversation
// 获取会话信息
coze_error_t coze_conversations_retrieve(const coze_conversations_retrieve_request_t *req,
                                         coze_conversations_retrieve_response_t *resp);

void coze_free_conversations_retrieve_response(coze_conversations_retrieve_response_t *resp);

// Create Conversation Message
// 创建会话消息
coze_error_t coze_conversations_messages_create(const coze_conversations_messages_create_request_t *req,
                                                coze_conversations_messages_create_response_t *resp);

void coze_free_conversations_messages_create_response(coze_conversations_messages_create_response_t *resp);

// List Conversation Messages
// 获取会话消息列表
coze_error_t coze_conversations_messages_list(const coze_conversations_messages_list_request_t *req,
                                              coze_conversations_messages_list_response_t *resp);

void coze_free_conversations_messages_list_response(coze_conversations_messages_list_response_t *resp);

// Retrieve Conversation Message
// 获取会话中的消息
coze_error_t coze_conversations_messages_retrieve(const coze_conversations_messages_retrieve_request_t *req,
                                                  coze_conversations_messages_retrieve_response_t *resp);

void coze_free_conversations_messages_retrieve_response(coze_conversations_messages_retrieve_response_t *resp);

// Update Conversation Message
// 更新会话消息
coze_error_t coze_conversations_messages_update(const coze_conversations_messages_update_request_t *req,
                                                coze_conversations_messages_update_response_t *resp);

void coze_free_conversations_messages_update_response(coze_conversations_messages_update_response_t *resp);

// Delete Conversation Message
// 删除会话消息
coze_error_t coze_conversations_messages_delete(const coze_conversations_messages_delete_request_t *req,
                                                coze_conversations_messages_delete_response_t *resp);

void coze_free_conversations_messages_delete_response(coze_conversations_messages_delete_response_t *resp);

// Create Chat
// 发起对话
coze_error_t coze_chat_create(const coze_chat_create_request_t *req,
                              coze_chat_create_response_t *resp);

void coze_free_chat_create_response(coze_chat_create_response_t *resp);

// Stream Chat
// 流式对话
coze_error_t coze_chat_stream(const coze_chat_stream_request_t *req,
                              coze_chat_stream_response_t *resp);

void coze_free_chat_stream_response(coze_chat_stream_response_t *resp);

// Retrieve Chat
// 获取对话信息
coze_error_t coze_chat_retrieve(const coze_chat_retrieve_request_t *req,
                                coze_chat_retrieve_response_t *resp);

void coze_free_chat_retrieve_response(coze_chat_retrieve_response_t *resp);

// List Chat Messages
// 获取对话消息列表
coze_error_t coze_chat_messages_list(const coze_chat_messages_list_request_t *req,
                                     coze_chat_messages_list_response_t *resp);

void coze_free_chat_messages_list_response(coze_chat_messages_list_response_t *resp);

// Submit Chat Tool Outputs
// 提交对话工具输出
coze_error_t coze_chat_submit_tool_outputs_create(const coze_chat_submit_tool_outputs_create_request_t *req,
                                                  coze_chat_submit_tool_outputs_create_response_t *resp);

void coze_free_chat_submit_tool_outputs_create_response(coze_chat_submit_tool_outputs_create_response_t *resp);

// Cancel Chat
// 取消对话
coze_error_t coze_chat_cancel(const coze_chat_cancel_request_t *req,
                              coze_chat_cancel_response_t *resp);

void coze_free_chat_cancel_response(coze_chat_cancel_response_t *resp);

// files

// Upload File
// 上传文件
coze_error_t coze_files_upload(const coze_files_upload_request_t *req,
                               coze_files_upload_response_t *resp);

void coze_free_files_upload_response(coze_files_upload_response_t *resp);

// Retrieve File
// 获取文件信息
coze_error_t coze_files_retrieve(const coze_files_retrieve_request_t *req,
                                 coze_files_retrieve_response_t *resp);

void coze_free_files_retrieve_response(coze_files_retrieve_response_t *resp);


// workflows

// Create Workflows Runs
// 执行工作流
coze_error_t coze_workflows_runs_create(const coze_workflows_runs_create_request_t *req,
                                        coze_workflows_runs_create_response_t *resp);

void coze_free_workflows_runs_create_response(coze_workflows_runs_create_response_t *resp);

// Stream Workflows Runs
// 流式执行工作流
coze_error_t coze_workflows_runs_stream(const coze_workflows_runs_stream_request_t *req,
                                        coze_workflows_runs_stream_response_t *resp);

void coze_free_workflows_runs_stream_response(coze_workflows_runs_stream_response_t *resp);

// Resume Workflows Runs
// 恢复工作流
coze_error_t coze_workflows_runs_resume(const coze_workflows_runs_resume_request_t *req,
                                        coze_workflows_runs_resume_response_t *resp);

void coze_free_workflows_runs_resume_response(coze_workflows_runs_resume_response_t *resp);

// audio

// List Audio Voices
// 获取音频语音列表
coze_error_t coze_audio_voices_list(const coze_audio_voices_list_request_t *req,
                                    coze_audio_voices_list_response_t *resp);

void coze_free_audio_voices_list_response(coze_audio_voices_list_response_t *resp);

// Create Audio Room
// 创建音频房间
coze_error_t coze_audio_rooms_create(const coze_audio_rooms_create_request_t *req,
                                     coze_audio_rooms_create_response_t *resp);

void coze_free_audio_rooms_create_response(coze_audio_rooms_create_response_t *resp);

#endif //COZE_H
