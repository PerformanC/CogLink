#ifndef LAVALINK_H
#define LAVALINK_H

#include <concord/websockets.h>

#include <coglink/plugins.h>

struct requestInformation {
  char *body;
  size_t size;
};

struct lavalinkStatsMemory {
  char *free;
  char *used;
  char *allocated;
  char *reservable;
};

struct lavalinkStatsFrameStats {
  char *sent;
  char *deficit;
  char *nulled;
};

struct lavalinkStatsCPU {
  char *cores;
  char *systemLoad;
  char *lavalinkLoad;
};

struct lavalinkStats {
  char *players;
  char *playingPlayers;
  char *uptime;
  struct lavalinkStatsMemory *memory;
  struct lavalinkStatsCPU *cpu;
  struct lavalinkStatsFrameStats *frameStats;
};

struct coglinkDebugging {
  _Bool allDebugging;
  _Bool sendPayloadErrorsDebugging;
  _Bool sendPayloadSuccessDebugging;
  _Bool checkParseErrorsDebugging;
  _Bool checkParseSuccessDebugging;
  _Bool joinVoiceDebugging;
  _Bool jsmnfErrorsDebugging;
  _Bool jsmnfSuccessDebugging;
  _Bool handleSchedulerVoiceStateDebugging;
  _Bool handleSchedulerVoiceServerDebugging;
  _Bool chashErrorsDebugging;
  _Bool chashSuccessDebugging;
  _Bool parseTrackErrorsDebugging;
  _Bool parseTrackSuccessDebugging;
  _Bool parsePlaylistErrorsDebugging;
  _Bool parsePlaylistSuccessDebugging;
  _Bool parseErrorsDebugging;
  _Bool parseSuccessDebugging;
  _Bool parseLoadtypeErrorsDebugging;
  _Bool parseLoadtypeSuccessDebugging;
  _Bool searchSongErrorsDebugging;
  _Bool searchSongSuccessDebugging;
  _Bool curlErrorsDebugging;
  _Bool curlSuccessDebugging;
  _Bool memoryDebugging;
};

struct lavalinkNode {
  char *name;
  char *hostname;
  char *password;
  char *shards;
  char *botId;
  _Bool ssl;
};

struct lavalinkEvents {
  int (*onRaw)(struct lavaInfo *lavaInfo, const char *data, size_t length);
  void (*onConnect)(void);
  void (*onClose)(enum ws_close_reason wscode, const char *reason);
  void (*onTrackStart)(char *track, u64snowflake guildId);
  void (*onTrackEnd)(char *reason, char *track, u64snowflake guildId);
  void (*onTrackException)(char *track, char *message, char *severity, char *cause, u64snowflake guildId);
  void (*onTrackStuck)(char *track, int thresholdMs, u64snowflake guildId);
  void (*onWebSocketClosed)(int code, char *reason, int byRemote, u64snowflake guildId);
  void (*onUnknownEvent)(char *type, const char *text, u64snowflake guildId);
  void (*onPlayerUpdate)(float time, int position, int connected, int ping, u64snowflake guildId);
  void (*onStats)(struct lavalinkStats *stats);
  void (*onUnknownOp)(char *op, const char *text);
};

struct lavaInfo {
  struct lavalinkNode *node;
  struct lavalinkEvents *events;
  struct io_poller *io_poller;
  struct websockets *ws;
  struct coglinkPlugins *plugins;
  CURLM *mhandle;
  uint64_t tstamp;
  struct coglinkDebugging *debugging;
  _Bool allowResuming;
  char *resumeKey;
  char *sessionId;
};

struct parsedTrack {
  char *track;
  char *identifier;
  char *isSeekable;
  char *author;
  char *length;
  char *isStream;
  char *position;
  char *title;
  char *uri;
  char *sourceName;
};

struct parsedPlaylist {
  char *name;
  char *selectedTrack;
};

struct parsedError {
  char *message;
  char *severity;
};

struct ws_info;
struct discord;

void onConnectEvent(void *data, struct websockets *ws, struct ws_info *info, const char *protocols);

void onCloseEvent(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t len);

void onTextEvent(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t len);

void coglink_joinVoiceChannel(struct lavaInfo *lavaInfo, struct discord *client, u64snowflake voiceChannelId, u64snowflake guildId);

void coglink_freeNodeInfo(struct lavaInfo *lavaInfo);

void coglink_disconnectNode(struct lavaInfo *lavaInfo);

void coglink_setEvents(struct lavaInfo *lavaInfo, struct lavalinkEvents *lavalinkEvents);

void coglink_connectNodeCleanup(struct lavaInfo *lavaInfo);

int coglink_connectNode(struct lavaInfo *lavaInfo, struct discord *client, struct lavalinkNode *node);

#endif
