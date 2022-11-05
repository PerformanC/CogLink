#ifndef REST_LAVALINK_H
#define REST_LAVALINK_H

int coglink_searchSong(struct lavaInfo lavaInfo, char *song, struct httpRequest *res);

void coglink_searchCleanup(struct httpRequest req);

int coglink_parseLoadtype(struct lavaInfo lavaInfo, struct httpRequest req, int *loadTypeValue);

int coglink_parseTrack(struct lavaInfo lavaInfo, struct httpRequest req, char *songPos, struct lavaParsedTrack **songStruct);

int coglink_parsePlaylist(struct lavaInfo lavaInfo, struct httpRequest req, struct lavaParsedPlaylist **playlistStruct);

int coglink_parseError(struct lavaInfo lavaInfo, struct httpRequest req, struct lavaParsedError **errorStruct);

void coglink_parseTrackCleanup(struct lavaInfo lavaInfo, struct lavaParsedTrack *songStruct);

void coglink_parsePlaylistCleanup(struct lavaInfo lavaInfo, struct lavaParsedPlaylist *playlistStruct);

void coglink_parseErrorCleanup(struct lavaInfo lavaInfo, struct lavaParsedError *errorStruct);

#endif