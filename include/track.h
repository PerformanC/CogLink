/** \file
 * File containing the Lavalink track related functions.
 */

#ifndef TRACK_H
#define TRACK_H

struct coglink_parsedTrackStruct {
  jsmnf_pair pairs[1024];
};

/**
 * Searches for the input query in YouTube.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param song Input query, can be either a link or a video title.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_searchSong(struct coglink_lavaInfo *lavaInfo, char *song, struct coglink_requestInformation *res);

/**
 * Frees the allocations generated while performing the function coglink_searchSong.
 * @param res Structure with the information of the request.
 */
void coglink_searchSongCleanup(struct coglink_requestInformation *res);

/**
 * Decodes a single track.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param track Track string that will be decoded
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_decodeTrack(struct coglink_lavaInfo *lavaInfo, char *track, struct coglink_requestInformation *res);

/**
 * Initializes the structure that will be used to parse the response body of any function that used it.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param cStruct Structure that will be filled with the pairs.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_initParseTrack(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res);

/**
 * Parses the response body of the function coglink_decodeTrack.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param parsedTrackStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseDecodeTrack(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, struct coglink_parsedTrack *parsedTrackStruct);

/**
 * Frees the allocations generated while performing the function coglink_decodeTrack and coglink_decodeTracks.
 * @param res Structure with the information of the request.
 */
void coglink_decodeTrackCleanup(struct coglink_requestInformation *res);

/**
 * Decodes multiple tracks at a single time.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param trackArray Track string that will be decoded
 * @param trackArrayLength Length of the array of tracks that will be decoded.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_decodeTracks(struct coglink_lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct coglink_requestInformation *res);

/**
 * Parses the response body of the function coglink_decodeTracks.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param pStruct Structure that is filled with the pairs.
 * @param res Structure with the information of the request.
 * @param songPos Position of the song that will be parsed.
 * @param parsedTrackStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseDecodeTracks(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, char *songPos, struct coglink_parsedTrack *parsedTrackStruct);

/**
 * Parses the loadType returned by coglink_searchSong.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param pStruct Structure that is filled with the pairs.
 * @param res Structure with the information of the request.
 * @param loadTypeValue type of the LoadType parsed. 
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseLoadtype(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, int *loadTypeValue);

/**
 * Parses the track returned by coglink_searchSong.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param pStruct Structure that is filled with the pairs.
 * @param res Structure with the information of the request.
 * @param songPos Position of the track that will be parsed.
 * @param parsedTrackStruct Structure that will be filled with the parsed information of the track. 
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseTrack(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, char *songPos, struct coglink_parsedTrack *parsedTrackStruct);

/**
 * Parses the playlist returned by coglink_searchSong.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param pStruct Structure that is filled with the pairs.
 * @param res Structure with the information of the request.
 * @param parsedPlaylistStruct Structure that will be filled with the parsed information of the playlist. 
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parsePlaylist(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, struct coglink_parsedPlaylist *parsedPlaylistStruct);

/**
 * Parses the error returned by coglink_searchSong.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param pStruct Structure that is filled with the pairs.
 * @param res Structure with the information of the request.
 * @param parsedErrorStruct Structure that will be filled with the parsed information of the error. 
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseError(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, struct coglink_parsedError *parsedErrorStruct);

#endif

