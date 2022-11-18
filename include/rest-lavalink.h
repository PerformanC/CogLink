#ifndef REST_LAVALINK_H
#define REST_LAVALINK_H

int coglink_searchSong(const struct lavaInfo *lavaInfo, char *song, struct httpRequest *res);

void coglink_searchCleanup(struct httpRequest req);

int coglink_parseLoadtype(struct lavaInfo *lavaInfo, struct httpRequest req, int *loadTypeValue);

int coglink_parseTrack(const struct lavaInfo *lavaInfo, struct httpRequest req, char *songPos, struct lavaParsedTrack **songStruct);

int coglink_parsePlaylist(const struct lavaInfo *lavaInfo, struct httpRequest req, struct lavaParsedPlaylist **playlistStruct);

int coglink_parseError(const struct lavaInfo *lavaInfo, struct httpRequest req, struct lavaParsedError **errorStruct);

void coglink_parseTrackCleanup(const struct lavaInfo *lavaInfo, struct lavaParsedTrack *songStruct);

void coglink_parsePlaylistCleanup(const struct lavaInfo *lavaInfo, struct lavaParsedPlaylist *playlistStruct);

void coglink_parseErrorCleanup(const struct lavaInfo *lavaInfo, struct lavaParsedError *errorStruct);

#endif
