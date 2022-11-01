#ifndef REST_LAVALINK_H
#define REST_LAVALINK_H

int coglink_searchSong(struct lavaInfo *lavaInfo, char *song, struct httpRequest *res);

void coglink_searchCleanup(struct httpRequest req);

int coglink_parseSearch(struct lavaInfo *lavaInfo, struct httpRequest req, char *songPos, struct lavaSong **songStruct);

#endif