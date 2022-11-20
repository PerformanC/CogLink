#ifndef LAVALINK_INTERNAL_H
#define LAVALINK_INTERNAL_H

struct lavaInfo;

void __coglink_sendPayload(const struct lavaInfo *lavaInfo, char payload[], int payloadMaxSize, char *payloadOP);

int __coglink_checkParse(struct lavaInfo *lavaInfo, jsmnf_pair *field, char *fieldName);

#endif
