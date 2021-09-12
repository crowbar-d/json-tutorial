#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <string.h>
#include <stdbool.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

// static int lept_parse_fixed(lept_context* c, lept_value* v, const char* str) {
//     const char* candidates[] = {"null", "true", "false"};
//     const lept_type types[] = {LEPT_NULL, LEPT_TRUE, LEPT_FALSE};

//     char* candidate;
//     lept_type type;
//     for (int i = 0; i < sizeof(candidates); i++) {
//         if (strcmp(str, candidates[i]) == 0) {
//             candidate = candidates[i];
//             type = types[i];
//         }
//     }


//     int j;
//     for (j = 0; j < strlen(candidate); j++) {
//         if (candidate[j] == *c->json) {
//             c->json++;
//         } else {
//             return LEPT_PARSE_INVALID_VALUE;
//         }
//     }
//     if (j == strlen(candidate)) {
//         v->type = type;
//         return LEPT_PARSE_OK;
//     }

//     return LEPT_PARSE_INVALID_VALUE;
// }

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    char* p = c->json;
    char* q = c->json;
    // bool ok;
    while (*q != '\0') {
        if (*q == '-' || *q == '+') {
            // must be first or after e/E
            if (q != p && *(q-1) != 'e' && *(q-1) != 'E') {
                return LEPT_PARSE_INVALID_VALUE;
            }
            q++;
        } else if (*q == '0') {
            // q是第一个
            if (p == q) {
                // 0后面不是'.'
                char* next = q+1;
                if (*next == '.') {
                    return LEPT_PARSE_INVALID_VALUE;
                }
            }
            q++;
        } else if (*q == '.') {
            // . cannot be first
            if(p == q) {
                return LEPT_PARSE_INVALID_VALUE;
            }
            // . cannot be last
            char* next = q+1;
            if (*next == '\0') {
                return LEPT_PARSE_INVALID_VALUE;
            }
            // must hvae number next
            if (!ISDIGIT(*next)) {
                return LEPT_PARSE_INVALID_VALUE;
            }

            q++;
        } else if(*q == 'e' || *q == 'E'){
            // . cannot be first
            if(p == q) {
                return LEPT_PARSE_INVALID_VALUE;
            }
            // . cannot be last
            char* next = q+1;
            if (*next == '\0') {
                return LEPT_PARSE_INVALID_VALUE;
            }
            // must hvae number next
            if (!(ISDIGIT(*next) || *next == '-' || *next == '+')) {
                return LEPT_PARSE_INVALID_VALUE;
            }
            q++;
        } else if (ISDIGIT(*q)) {
            q++;
        } else {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    // p ... q
    if (p >= q) {
        return LEPT_PARSE_INVALID_VALUE;
    }


    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
