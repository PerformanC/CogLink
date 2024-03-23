/*
  PerformanC's JSON builder, inspired on lcsmüller's json-build,
  however allocating memory manually.

  https://github.com/lcsmuller/json-build
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jsonb.h"

void pjsonb_init(struct pjsonb *builder) {
  builder->string = malloc(1);
  builder->string[0] = '{';
  builder->position = 1;
}

void pjsonb_end(struct pjsonb *builder) {
  builder->string[builder->position - 1] = '}';
}

void pjsonb_free(struct pjsonb *builder) {
  free(builder->string);
}

void pjsonb_set_int(struct pjsonb *builder, const char *key, int value) {
  int key_length = strlen(key);
  int value_length = snprintf(NULL, 0, "%d", value);

  builder->string = realloc(builder->string, builder->position + value_length + 3 + key_length);
  builder->position += snprintf(builder->string + builder->position, value_length + 3 + key_length, "\"%s\":%d,", key, value);
  builder->key_state = PJSONB_TO_CLOSE;
}

void pjsonb_set_float(struct pjsonb *builder, const char *key, float value) {
  int key_length = strlen(key);
  int value_length = snprintf(NULL, 0, "%f", value);

  builder->string = realloc(builder->string, builder->position + value_length + 3 + key_length);
  builder->position += snprintf(builder->string + builder->position, value_length + 3 + key_length, "\"%s\":%f,", key, value);
  builder->key_state = PJSONB_TO_CLOSE;
}

void pjsonb_set_bool(struct pjsonb *builder, const char *key, int value) {
  int key_length = strlen(key);
  int value_length = snprintf(NULL, 0, "%s", value ? "true" : "false");

  builder->string = realloc(builder->string, builder->position + value_length + 3 + key_length);
  builder->position += snprintf(builder->string + builder->position, value_length + 3 + key_length, "\"%s\":%s,", key, value ? "true" : "false");
  builder->key_state = PJSONB_TO_CLOSE;
}

void pjsonb_set_string(struct pjsonb *builder, const char *key, const char *value) {
  int key_length = strlen(key);
  int value_length = strlen(value);

  builder->string = realloc(builder->string, builder->position + value_length + 6 + key_length);
  builder->position += snprintf(builder->string + builder->position, value_length + 6 + key_length, "\"%s\":\"%s\",", key, value);
  builder->key_state = PJSONB_TO_CLOSE;
}

void pjson_enter_object(struct pjsonb *builder, const char *key) {
  int key_length = strlen(key);

  builder->string = realloc(builder->string, builder->position + 5 + key_length);
  builder->position += snprintf(builder->string + builder->position, 5 + key_length, "\"%s\":{", key);
  builder->key_state = PJSONB_NONE;
}

void pjson_leave_object(struct pjsonb *builder) {
  if (builder->key_state == PJSONB_TO_CLOSE) {
    builder->string[builder->position - 1] = '}';
    builder->string = realloc(builder->string, builder->position + 1);
    builder->string[builder->position] = ',';
    builder->position++;
  } else {
    builder->string = realloc(builder->string, builder->position + 2 + 1);
    builder->position += snprintf(builder->string + builder->position, 2 + 1, "},");
  }

  builder->key_state = PJSONB_TO_CLOSE;
}

void pjson_enter_array(struct pjsonb *builder, const char *key) {
  int key_length = strlen(key);

  builder->string = realloc(builder->string, builder->position + 3 + key_length);
  builder->position += snprintf(builder->string + builder->position, 3 + key_length, "\"%s\":[", key);
  builder->key_state = PJSONB_NONE;
}

void pjson_leave_array(struct pjsonb *builder) {
  if (builder->key_state == PJSONB_TO_CLOSE) {
    builder->string[builder->position - 1] = ']';
  } else {
    builder->string = realloc(builder->string, builder->position + 2);
    builder->position += snprintf(builder->string + builder->position, 2, "]");
  }
}
