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

int __coglink_performRequest(struct lavaInfo *lavaInfo, int requestType, int additionalDebuggingSuccess, int additionalDebuggingError, char *path, int pathLength, int useV3Path, char *body, long bodySize, struct requestInformation *res, int getResponse, CURL *reUsedCurl);

int __coglink_checkParse(struct lavaInfo *lavaInfo, jsmnf_pair *field, char *fieldName);

void __coglink_randomString(char *dest, size_t length);

int __coglink_IOPoller(struct io_poller *io, CURLM *multi, void *user_data);

#endif
