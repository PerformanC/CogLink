/** \file
 * File containing the macros of Coglink.
 */

#ifndef DEFINATIONS_H
#define DEFINATIONS_H

/** Macro for definination of volume filter. */
#define FILTER_VOLUME 0
/** Macro for definination of equalizer filte. */
#define FILTER_EQUALIZER 1
/** Macro for definination of karaoke filte. */
#define FILTER_KARAOKE 2
/** Macro for definination of timescale filte. */
#define FILTER_TIMESCALE 3
/** Macro for definination of tremolo filte. */
#define FILTER_TREMOLO 4
/** Macro for definination of rotation filter. */
#define FILTER_ROTATION 5
/** Macro for definination of distortion filter. */
#define FILTER_DISTORTION 6
/** Macro for definination of channelMix filter. */
#define FILTER_CHANNELMIX 7
/** Macro for definination of lowPass filter. */
#define FILTER_LOWPASS 8
/** Macro for definination of "remove filter". */
#define FILTER_REMOVE 9

#define ROUTERPLANNER_CLASS_LENGTH 26
#define LAVALINK_SESSIONID_LENGTH 17

#define VOLUME_LENGTH 8
#define TOKEN_LENGTH 128
#define ENDPOINT_LENGTH 64
#define SESSIONID_LENGTH 16
#define PING_LENGTH 8

#define DISCORD_TOKEN_LENGTH 17
#define VOICE_ID_LENGTH 32
#define GUILD_ID_LENGTH 32
#define USER_ID_LENGTH 32
#define SESSION_ID_LENGTH 128
#define TRUE_FALSE_LENGTH 8

#define TRACK_LENGTH 256
#define IDENTIFIER_LENGTH 13
#define TRACK_TITLE_LENGTH 100
#define PLAYLIST_NAME_LENGTH 64
#define AUTHOR_NAME_LENGTH 30
#define VIDEO_LENGTH 16
#define URL_LENGTH 44
#define SOURCENAME_LENGTH 10

#define COGLINK_WAIT 103

/** Macro for definination of when Lavalink did not found any match to query. */
#define COGLINK_LOADTYPE_NO_MATCHES 5
/** Macro for definination of when **Lavalink** failed to load result. */
#define COGLINK_LOADTYPE_LOAD_FAILED 4
/** Macro for definination of when Lavalink returned the search resukt. */
#define COGLINK_LOADTYPE_SEARCH_RESULT 3
/** Macro for definination of when Lavalink found a playlist. */
#define COGLINK_LOADTYPE_PLAYLIST_LOADED 2
/** Macro for definination of when Lavalink found a track. */
#define COGLINK_LOADTYPE_TRACK_LOADED 1

/** Macro for definination of when Coglink should proceed handling event. */
#define COGLINK_PROCEED 1
/** Macro for definination of when the function occured with no issues. */
#define COGLINK_SUCCESS 0
/** Macro for definination of when Coglink should not proceeding handling event. */
#define COGLINK_STOP -1

/** Macro for definination of when jsmnf failed to parse the JSON. */
#define COGLINK_JSMNF_ERROR_PARSE -2
/** Macro for definination of when jsmnf failed to load. */
#define COGLINK_JSMNF_ERROR_LOAD -3
/** Macro for definination of when jsmnf failed to find the JSON key. */
#define COGLINK_JSMNF_ERROR_FIND -4

/** Macro for definination of when libcurl failed to initialize. */
#define COGLINK_LIBCURL_FAILED_INITIALIZE -5
/** Macro for definination of when libcurl failed to execute setopt function. */
#define COGLINK_LIBCURL_FAILED_SETOPT -6
/** Macro for definination of when libcurl failed to send a request. */
#define COGLINK_LIBCURL_FAILED_PERFORM -7

/** Macro for definination of when a router planner was not set. */
#define COGLINK_ROUTERPLANNER_NOT_SET -8

/** Macro for definination of when some unknown error happened. */
#define COGLINK_ERROR -9

/** Macro for definination of when there is no players. */
#define COGLINK_NO_PLAYERS -10

#endif
