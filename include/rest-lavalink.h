#ifndef REST_LAVALINK_H
#define REST_LAVALINK_H

int coglink_searchSong(const struct lavaInfo *lavaInfo, char *song, struct requestInformation *res);

void coglink_searchCleanup(struct requestInformation req);

int coglink_parseLoadtype(struct lavaInfo *lavaInfo, struct requestInformation *req, int *loadTypeValue);

int coglink_parseTrack(const struct lavaInfo *lavaInfo, struct requestInformation *req, char *songPos, struct parsedTrack **songStruct);

int coglink_parsePlaylist(const struct lavaInfo *lavaInfo, struct requestInformation *req, struct parsedPlaylist **playlistStruct);

int coglink_parseError(const struct lavaInfo *lavaInfo, struct requestInformation *req, struct parsedError **errorStruct);

void coglink_parseTrackCleanup(const struct lavaInfo *lavaInfo, struct parsedTrack *songStruct);

void coglink_parsePlaylistCleanup(const struct lavaInfo *lavaInfo, struct parsedPlaylist *playlistStruct);

void coglink_parseErrorCleanup(const struct lavaInfo *lavaInfo, struct parsedError *errorStruct);

#endif
