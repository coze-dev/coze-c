#include <stdio.h>
#include "coze.h"
#include "cJSON.h"

int main() {
    char *string = "{\"name\": \"John\", \"age\": 30, \"city\": \"New York\"}";

    cJSON *json = cJSON_Parse(string);

    printf("%s\n", cJSON_Print(json));

    cJSON_Delete(json);

    return 0;
}
