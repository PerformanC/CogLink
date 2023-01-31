#ifndef LAVALINK_INTERNAL_H
#define LAVALINK_INTERNAL_H

#include <curl/curl.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

struct lavaInfo;
struct requestInformation;
struct io_poller;

#define __COGLINK_GET_REQ 0
#define __COGLINK_POST_REQ 1
#define __COGLINK_DELETE_REQ 2
#define __COGLINK_PATCH_REQ 3

struct __coglink_requestConfig {
  int requestType;
  _Bool additionalDebuggingSuccess;
  _Bool additionalDebuggingError;
  char *path;
  int pathLength;
  _Bool useV3Path;
  char *body;
  long bodySize;
  _Bool getResponse;
  CURL *usedCURL;
};

int __coglink_performRequest(struct lavaInfo *lavaInfo, struct requestInformation *res, struct __coglink_requestConfig *config);

int __coglink_checkParse(struct lavaInfo *lavaInfo, jsmnf_pair *field, char *fieldName);

void __coglink_randomString(char *dest, size_t length);

int __coglink_IOPoller(struct io_poller *io, CURLM *multi, void *user_data);

#endif
