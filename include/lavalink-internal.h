#ifndef LAVALINK_INTERNAL_H
#define LAVALINK_INTERNAL_H

void __coglink_sendPayload(const struct lavaInfo *lavaInfo, char payload[], char *payloadOP);

int __coglink_checkParse(struct lavaInfo *lavaInfo, jsmnf_pair *field, char *fieldName);

#endif