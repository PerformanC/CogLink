#ifndef LAVALINK_INTERNAL_H
#define LAVALINK_INTERNAL_H

struct lavaInfo;
struct httpRequest;

int __coglink_performRequest(struct lavaInfo *lavaInfo, int additionalDebuggingSuccess, int additionalDebuggingError, char *path, int pathLength, char *body, long bodySize, struct httpRequest *res, int getResponse, CURL *reUsedCurl);

void __coglink_sendPayload(const struct lavaInfo *lavaInfo, char payload[], int payloadMaxSize, char *payloadOP);

int __coglink_checkParse(struct lavaInfo *lavaInfo, jsmnf_pair *field, char *fieldName);

#endif
