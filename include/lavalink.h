/** \file
 * File containing the Lavalink WebSocket related functions.
 */

#ifndef LAVALINK_H
#define LAVALINK_H

#include <concord/websockets.h>

#include <coglink/plugins.h>
#include <coglink/definitions.h>

struct coglink_requestInformation {
  char *body;
  size_t size;
};

struct coglink_lavalinkStatsMemory {
  char free[16];
  char used[16];
  char allocated[16];
  char reservable[16];
};

struct coglink_lavalinkStatsFrameStats {
  char sent[16];
  char deficit[16];
  char nulled[16];
};

struct coglink_lavalinkStatsCPU {
  char cores[8];
  char systemLoad[16];
  char lavalinkLoad[16];
};

struct coglink_lavalinkStats {
  char players[8];
  char playingPlayers[8];
  char uptime[32];
  struct coglink_lavalinkStatsMemory *memory;
  struct coglink_lavalinkStatsCPU *cpu;
  struct coglink_lavalinkStatsFrameStats *frameStats;
};

struct coglink_coglinkDebugging {
  int allDebugging;
  int sendPayloadErrorsDebugging;
  int sendPayloadSuccessDebugging;
  int checkParseErrorsDebugging;
  int checkParseSuccessDebugging;
  int joinVoiceDebugging;
  int jsmnfErrorsDebugging;
  int jsmnfSuccessDebugging;
  int handleSchedulerVoiceStateDebugging;
  int handleSchedulerVoiceServerDebugging;
  int joinUserVoiceChannelDebugging;
  int tablecErrorsDebugging;
  int tablecSuccessDebugging;
  int parseTrackErrorsDebugging;
  int parseTrackSuccessDebugging;
  int parsePlaylistErrorsDebugging;
  int parsePlaylistSuccessDebugging;
  int parseErrorsDebugging;
  int parseSuccessDebugging;
  int parseLoadtypeErrorsDebugging;
  int parseLoadtypeSuccessDebugging;
  int searchSongErrorsDebugging;
  int searchSongSuccessDebugging;
  int curlErrorsDebugging;
  int curlSuccessDebugging;
  int memoryDebugging;
};

struct coglink_lavalinkNode {
  char *name;
  char *hostname;
  char *password;
  char *shards;
  char *botId;
  int ssl;
  char resumeKey[8];
  char sessionId[COGLINK_LAVALINK_SESSIONID_LENGTH];
};

struct coglink_lavalinkEvents {
  int (*onRaw)(struct coglink_lavaInfo *lavaInfo, const char *data, size_t length);
  void (*onConnect)(char *sessionId);
  void (*onClose)(enum ws_close_reason wscode, const char *reason);
  void (*onTrackStart)(char *track, char *guildId);
  void (*onTrackEnd)(char *reason, char *track, char *guildId);
  void (*onTrackException)(char *track, char *message, char *severity, char *cause, char *guildId);
  void (*onTrackStuck)(char *track, char *thresholdMs, char *guildId);
  void (*onWebSocketClosed)(char *code, char *reason, int byRemote, char *guildId);
  void (*onUnknownEvent)(char *type, const char *text, char *guildId);
  void (*onPlayerUpdate)(char *time, char *position, int connected, char *ping, char *guildId);
  void (*onStats)(struct coglink_lavalinkStats *stats);
  void (*onUnknownOp)(char *op, const char *text);
};

struct coglink_lavaInfo {
  struct coglink_lavalinkNode *node;
  struct coglink_lavalinkEvents *events;
  struct io_poller *io_poller;
  struct websockets *ws;
  struct coglink_coglinkPlugins *plugins;
  struct coglink_coglinkDebugging *debugging;
  CURLM *mhandle;
  uint64_t tstamp;
  int allowResuming;
  int allowCachingVoiceChannelIds;
};

struct coglink_parsedTrack {
  char track[COGLINK_TRACK_LENGTH];
  char identifier[COGLINK_IDENTIFIER_LENGTH];
  char isSeekable[COGLINK_TRUE_FALSE_LENGTH];
  char author[COGLINK_AUTHOR_NAME_LENGTH];
  char length[COGLINK_VIDEO_LENGTH];
  char isStream[COGLINK_TRUE_FALSE_LENGTH];
  char position[COGLINK_VIDEO_LENGTH];
  char title[COGLINK_TRACK_TITLE_LENGTH];
  char uri[COGLINK_URL_LENGTH];
  char isrc[64];
  char artworkUrl[256];
  char sourceName[COGLINK_SOURCENAME_LENGTH];
};

struct coglink_parsedPlaylist {
  char name[COGLINK_PLAYLIST_NAME_LENGTH];
  char selectedTrack[8];
};

struct coglink_parsedError {
  char message[128];
  char severity[16];
};

struct ws_info;
struct discord;

void onConnectEvent(void *data, struct websockets *ws, struct ws_info *info, const char *protocols);

void onCloseEvent(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t len);

void onTextEvent(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t len);

/**
 * Joins a voice channel.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param client Concord's client stucture generated with discord_init.
 * @param voiceChannelId ID of the voice channel that the bot will join.
 * @param guildId ID of the guild of the voice channel that the bot will join.
 */
void coglink_joinVoiceChannel(struct coglink_lavaInfo *lavaInfo, struct discord *client, u64snowflake voiceChannelId, u64snowflake guildId);

/**
 * Joins a voice channel that a user is in, must have the optional settings enabled for it.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param client Concord's client stucture generated with discord_init.
 * @param userId ID of the user that the bot will join in its voice channel.
 * @param guildId ID of the guild of the voice channel that the bot will join.
 */
int coglink_joinUserVoiceChannel(struct coglink_lavaInfo *lavaInfo, struct discord *client, u64snowflake userId, u64snowflake guildId);

/**
 * Removes all information in the lavaInfo struct
 * @param lavaInfo Structure with important informations of the Lavalink.
 */
void coglink_freeNodeInfo(struct coglink_lavaInfo *lavaInfo);

/**
 * Closes the WebSocket connection with the Lavalink node.
 * @param lavaInfo Structure with important informations of the Lavalink.
 */
void coglink_disconnectNode(struct coglink_lavaInfo *lavaInfo);

/**
 * Sets the Lavalink events when not including it directly into lavaInfo structure.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param coglink_lavalinkEvents Lavalink event's structure with all pointers to the functions that will be executed when the event it emitted.
 */
void coglink_setEvents(struct coglink_lavaInfo *lavaInfo, struct coglink_lavalinkEvents *lavalinkEvents);

/**
 * Frees all mallocs used while connecting to a node and in other functions.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param client Concord's client stucture generated with discord_init.
 */
void coglink_connectNodeCleanup(struct coglink_lavaInfo *lavaInfo, struct discord *client);

/**
 * Creates a WebSocket connecting with the Lavalink node.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param client Concord's client stucture generated with discord_init.
 * @param node Structure with all Lavalink node information.
 * @returns COGLINK_SUCCESS
 */
int coglink_connectNode(struct coglink_lavaInfo *lavaInfo, struct discord *client, struct coglink_lavalinkNode *node);

#endif
