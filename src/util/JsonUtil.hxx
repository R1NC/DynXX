#ifndef NGENXX_UTIL_JSON_H_
#define NGENXX_UTIL_JSON_H_

#define JSON_READ_STR(j, x)                                                                               \
    do                                                                                                    \
    {                                                                                                     \
        if (j->string && strcmp(j->string, #x) == 0 && j->type == cJSON_String && j->valuestring != NULL) \
        {                                                                                                 \
            x = (char *)malloc(strlen(j->valuestring) + 1);                                               \
            strcpy(x, j->valuestring);                                                                    \
        }                                                                                                 \
    } while (0)

#define JSON_READ_NUM(j, x)                                                     \
    do                                                                          \
    {                                                                           \
        if (j->string && strcmp(j->string, #x) == 0 && j->type == cJSON_Number) \
        {                                                                       \
            x = j->valuedouble;                                                 \
        }                                                                       \
    } while (0)

#define JSON_READ_STR_ARRAY(j, v, vc, ic)                                                  \
    do                                                                                     \
    {                                                                                      \
        if (j->string && strcmp(j->string, #v) == 0 && j->type == cJSON_Array && j->child) \
        {                                                                                  \
            v = (char **)malloc(vc * sizeof(char *));                                      \
            cJSON *vj = j->child;                                                          \
            int i = 0;                                                                     \
            while (vj)                                                                     \
            {                                                                              \
                v[i] = (char *)malloc(ic * sizeof(char));                                  \
                strcpy(v[i], vj->valuestring);                                             \
                i++;                                                                       \
                vj = vj->next;                                                             \
            }                                                                              \
        }                                                                                  \
    } while (0)

#endif // NGENXX_UTIL_JSON_H_