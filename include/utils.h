#ifndef COGLINK_UTILS_H
#define COGLINK_UTILS_H

#define COGLINK_SESSION_ID_LENGTH 16

#define FIND_FIELD(pairs, json, field, fieldName)                                \
  jsmnf_pair *field = jsmnf_find(pairs, json, fieldName, sizeof(fieldName) - 1); \
                                                                                 \
  if (field == NULL) {                                                           \
    ERROR("[coglink] Failed to find field: %s", fieldName);                      \
                                                                                 \
    return COGLINK_FAILED;                                                       \
  }

#define FIND_FIELD_PATH(json, pairs, field, fieldName, pathSize) \
  jsmnf_pair *field = jsmnf_find_path(pairs, json, path, pathSize);  \
                                                                     \
  if (field == NULL) {                                               \
    ERROR("[coglink] Failed to find field: %s", fieldName);          \
                                                                     \
    return COGLINK_FAILED;                                           \
  }

#define PAIR_TO_SIZET(json, pair, fieldName, outputName, size) \
  char fieldName[size];                                        \
  memcpy(fieldName, json + pair->v.pos, pair->v.len);          \
  fieldName[pair->v.len] = '\0';                               \
                                                               \
  outputName = strtoull(fieldName, NULL, 10);

#define PAIR_TO_D_STRING(json, pair, output)         \
  output = malloc((pair->v.len + 1) * sizeof(char)); \
  memcpy(output, json + pair->v.pos, pair->v.len);   \
  output[pair->v.len] = '\0';

#ifdef COGLINK_DEBUG
  #define FATAL(...)          \
    log_fatal(__VA_ARGS__);   \
                              \
    printf(__VA_ARGS__);      \

  #define DEBUG(...) log_debug(__VA_ARGS__)
  #define INFO(...)  log_info(__VA_ARGS__)
  #define WARN(...)  log_warn(__VA_ARGS__)
  #define ERROR(...) log_error(__VA_ARGS__)
#else
  #define FATAL(...) exit(1);
  #define DEBUG(...)
  #define INFO(...)
  #define WARN(...)
  #define ERROR(...)
#endif

#include "lavalink.h"

struct coglink_request_params {
  char *endpoint;
  char *method;
  char *body;
  size_t body_length;
  bool get_response;
  bool unversioned;
};

struct coglink_response {
  char *body;
  size_t size;
};

int _coglink_perform_request(struct coglink_node *nodeInfo, struct coglink_request_params *req, struct coglink_response *res);

#endif
