/** \file
 * File containing the macros of Coglink.
 */

#ifndef DEFINATIONS_H
#define DEFINATIONS_H

/** Macro for defininition of volume filter. */
#define COGLINK_FILTER_VOLUME 0
/** Macro for defininition of equalizer filte. */
#define COGLINK_FILTER_EQUALIZER 1
/** Macro for defininition of karaoke filte. */
#define COGLINK_FILTER_KARAOKE 2
/** Macro for defininition of timescale filte. */
#define COGLINK_FILTER_TIMESCALE 3
/** Macro for defininition of tremolo filte. */
#define COGLINK_FILTER_TREMOLO 4
/** Macro for defininition of rotation filter. */
#define COGLINK_FILTER_ROTATION 5
/** Macro for defininition of distortion filter. */
#define COGLINK_FILTER_DISTORTION 6
/** Macro for defininition of channelMix filter. */
#define COGLINK_FILTER_CHANNELMIX 7
/** Macro for defininition of lowPass filter. */
#define COGLINK_FILTER_LOWPASS 8
/** Macro for defininition of "remove filter". */
#define COGLINK_FILTER_REMOVE 9

#define COGLINK_ROUTERPLANNER_CLASS_LENGTH 26
#define COGLINK_LAVALINK_SESSIONID_LENGTH 17

#define COGLINK_VOLUME_LENGTH 8
#define COGLINK_TOKEN_LENGTH 128
#define COGLINK_ENDPOINT_LENGTH 64
#define COGLINK_SESSIONID_LENGTH 16
#define COGLINK_PING_LENGTH 8
#define COGLINK_BANDS_LENGTH 512
#define COGLINK_GAIN_LENGTH 128

#define COGLINK_DISCORD_TOKEN_LENGTH 17
#define COGLINK_VOICE_ID_LENGTH 20
#define COGLINK_GUILD_ID_LENGTH 20
#define COGLINK_USER_ID_LENGTH 20
#define COGLINK_SESSION_ID_LENGTH 128
#define COGLINK_TRUE_FALSE_LENGTH 8

#define COGLINK_TRACK_LENGTH 512
#define COGLINK_IDENTIFIER_LENGTH 13
#define COGLINK_TRACK_TITLE_LENGTH 100
#define COGLINK_PLAYLIST_NAME_LENGTH 64
#define COGLINK_AUTHOR_NAME_LENGTH 30
#define COGLINK_VIDEO_LENGTH 16
#define COGLINK_TIME_LENGTH 16
#define COGLINK_URL_LENGTH 44
#define COGLINK_SOURCENAME_LENGTH 10

#define COGLINK_WAIT 103

/** Macro for defininition of when Lavalink did not found any match to query. */
#define COGLINK_LOADTYPE_NO_MATCHES 5
/** Macro for defininition of when **Lavalink** failed to load result. */
#define COGLINK_LOADTYPE_LOAD_FAILED 4
/** Macro for defininition of when Lavalink returned the search resukt. */
#define COGLINK_LOADTYPE_SEARCH_RESULT 3
/** Macro for defininition of when Lavalink found a playlist. */
#define COGLINK_LOADTYPE_PLAYLIST_LOADED 2
/** Macro for defininition of when Lavalink found a track. */
#define COGLINK_LOADTYPE_TRACK_LOADED 1

/** Macro for defininition of when Coglink should proceed handling event. */
#define COGLINK_PROCEED 1
/** Macro for defininition of when the function occured with no issues. */
#define COGLINK_SUCCESS 0
/** Macro for defininition of when Coglink should not proceeding handling event. */
#define COGLINK_STOP -1

/** Macro for defininition of when jsmnf failed to parse the JSON. */
#define COGLINK_JSMNF_ERROR_PARSE -2
/** Macro for defininition of when jsmnf failed to load. */
#define COGLINK_JSMNF_ERROR_LOAD -3
/** Macro for defininition of when jsmnf failed to find the JSON key. */
#define COGLINK_JSMNF_ERROR_FIND -4

/** Macro for defininition of when libcurl failed to initialize. */
#define COGLINK_LIBCURL_FAILED_INITIALIZE -5
/** Macro for defininition of when libcurl failed to execute setopt function. */
#define COGLINK_LIBCURL_FAILED_SETOPT -6
/** Macro for defininition of when libcurl failed to send a request. */
#define COGLINK_LIBCURL_FAILED_PERFORM -7

/** Macro for defininition of when a router planner was not set. */
#define COGLINK_ROUTERPLANNER_NOT_SET -8

/** Macro for defininition of when some unknown error happened. */
#define COGLINK_ERROR -9

/** Macro for defininition of when there is no players. */
#define COGLINK_NO_PLAYERS -10

/** Macro for defination of when TableC does not find anything related to the key. */
#define COGLINK_TABLEC_NOT_FOUND -11

/** Macro for defination of when there is no nodes avaible. */
#define COGLINK_NO_NODES -12

#endif
