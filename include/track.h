/** \file
 * File containing the Lavalink track related functions.
 */

#ifndef TRACK_H
#define TRACK_H

/**
 * Decodes a single track.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param track Track string that will be decoded
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_decodeTrack(struct lavaInfo *lavaInfo, char *track, struct requestInformation *res);

/**
 * Parses the response body of the function coglink_decodeTrack.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param parsedTrackStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseDecodeTrack(struct lavaInfo *lavaInfo, struct requestInformation *res, struct parsedTrack **parsedTrackStruct);

/**
 * Frees the allocations generated while performing the function coglink_decodeTrack and coglink_decodeTracks.
 * @param res Structure with the information of the request.
 */
void coglink_decodeTrackCleanup(struct requestInformation *res);

/**
 * Decodes multiple tracks at a single time.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param trackArray Track string that will be decoded
 * @param trackArrayLength Length of the array of tracks that will be decoded.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_decodeTracks(struct lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct requestInformation *res);

/**
 * Parses the response body of the function coglink_decodeTracks.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param songPos Position of the song that will be parsed.
 * @param parsedTrackStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseDecodeTracks(struct lavaInfo *lavaInfo, struct requestInformation *res, char *songPos, struct parsedTrack **parsedTrackStruct);

/**
 * Searches for the input query in YouTube.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param song Input query, can be either a link or a video title.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_searchSong(const struct lavaInfo *lavaInfo, char *song, struct requestInformation *res);

/**
 * Frees the allocations generated while performing the function coglink_searchSong.
 * @param res Structure with the information of the request.
 */
void coglink_searchSongCleanup(struct requestInformation *res);

/**
 * Parses the loadType returned by coglink_searchSong.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param loadTypeValue type of the LoadType parsed. 
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseLoadtype(struct lavaInfo *lavaInfo, struct requestInformation *res, int *loadTypeValue);

/**
 * Parses the track returned by coglink_searchSong.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param songPos Position of the track that will be parsed.
 * @param parsedTrackStruct Structure that will be filled with the parsed information of the track. 
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseTrack(const struct lavaInfo *lavaInfo, struct requestInformation *res, char *songPos, struct parsedTrack **parsedTrackStruct);

/**
 * Parses the playlist returned by coglink_searchSong.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param parsedPlaylistStruct Structure that will be filled with the parsed information of the playlist. 
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parsePlaylist(const struct lavaInfo *lavaInfo, struct requestInformation *res, struct parsedPlaylist **parsedPlaylistStruct);

/**
 * Parses the error returned by coglink_searchSong.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param parsedErrorStruct Structure that will be filled with the parsed information of the error. 
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseError(const struct lavaInfo *lavaInfo, struct requestInformation *res, struct parsedError **parsedErrorStruct);

#endif
