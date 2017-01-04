#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_type t,
                              char* l, lept_value* v) {
    int i = 0;
    const char *p = c->json;

    while (l[i] == p[i] && l[i] != '\0'
           && p[i] != '\0')
      i++;

    if(l[i] != '\0')
      return LEPT_PARSE_INVALID_VALUE;

    c->json += i;
    v->type = t;

    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {

    const char* p = c->json;

    if (*p == '-')
      p++;

    if (*p == '0') {
      p++;
    } else {
      if(ISDIGIT1TO9(*p))
        while(ISDIGIT(*p))
          p++;
      else
        return LEPT_PARSE_INVALID_VALUE;
    }

    if (*p == '.') {
      p++;
      if(ISDIGIT(*p))
        while(ISDIGIT(*p))
          p++;
      else
        return LEPT_PARSE_INVALID_VALUE;
    }

    if (*p == 'e' || *p == 'E') {
      p++;
      if (*p == '+' || *p == '-')
        p++;

      if(ISDIGIT(*p))
        while(ISDIGIT(*p))
          p++;
      else
        return LEPT_PARSE_INVALID_VALUE;
    }

    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;

    c->json = p;

    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 'n':  return lept_parse_literal(c, LEPT_NULL, "null", v);
        case 't':  return lept_parse_literal(c, LEPT_TRUE, "true", v);
        case 'f':  return lept_parse_literal(c, LEPT_FALSE, "false", v);
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
