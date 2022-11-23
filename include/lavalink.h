#ifndef LAVALINK_H
#define LAVALINK_H

struct httpRequest {
  char *body;
  size_t size;
};

struct lavaNode {
  char *name;
  char *hostname;
  char *password;
  char *shards;
  char *botId;
  _Bool ssl;
};

struct lavaMemory {
  char reservable[16];
  char used[16];
  char free[16];
  char allocated[16];
};

struct lavaFStats {
  char sent[16];
  char deficit[16];
  char nulled[16];
};

struct lavaCPU {
  char cores[8];
  char systemLoad[16];
  char lavalinkLoad[16];
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

struct IOPollerVars {
  struct discord_gateway *gw;
  struct lavaInfo *lavaInfo;
};

struct lavaInfo {
  struct lavaNode *node;
  struct lavaEvents *events;
  struct io_poller *io_poller;
  struct websockets *ws;
  CURLM *mhandle;
  uint64_t tstamp;
  struct coglinkDebugging *debugging;
  _Bool allowResuming;
  _Bool lavaResumeSend;
  char *resumeKey;
};

struct lavaParsedTrack {
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

struct lavaParsedPlaylist {
  char *name;
  char *selectedTrack;
};

struct lavaParsedError {
  char *message;
  char *severity;
};

struct lavaEvents {
  // BASICS
  int (*onRaw)(struct lavaInfo *lavaInfo, const char *data, size_t length);
  void (*onConnect)(void);
  void (*onClose)(enum ws_close_reason wscode, const char *reason);
  // EVENTS
  void (*onTrackStart)(char *track, u64snowflake guildId);
  void (*onTrackEnd)(char *reason, char *track, u64snowflake guildId);
  void (*onTrackException)(char *track, char *message, char *severity, char *cause, u64snowflake guildId);
  void (*onTrackStuck)(char *track, int thresholdMs, u64snowflake guildId);
  void (*onWebSocketClosed)(int code, char *reason, char *byRemote, u64snowflake guildId);
  void (*onUnknownEvent)(char *type, const char *text, u64snowflake guildId);
  // PLAYER EVENTS
  void (*onPlayerUpdate)(int time, int position, char *connected, int ping, u64snowflake guildId);
  // OTHER EVENTS
  void (*onStats)(int playingPlayers, struct lavaMemory *infoMemory, int players, struct lavaFStats *infoFrameStats, struct lavaCPU *infoCPU, int uptime);
  void (*onUnknownOp)(char *op, const char *text);
};

struct StringBucket {
  char *key;
  char *value;
  int state;
};

struct StringHashtable {
  int length;
  int capacity;
  struct StringBucket *buckets;
};

struct ws_info;

void onConnectEvent(void *data, struct websockets *ws, struct ws_info *info, const char *protocols);

void onCloseEvent(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t len);

void onTextEvent(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t len);

void coglink_joinVoiceChannel(struct lavaInfo *lavaInfo, struct discord *client, u64snowflake voiceChannelId, u64snowflake guildId);

void coglink_freeNodeInfo(struct lavaInfo *lavaInfo);

void coglink_disconnectNode(struct lavaInfo *lavaInfo);

void coglink_setEvents(struct lavaInfo *lavaInfo, struct lavaEvents *lavaEvents);

void coglink_connectNodeCleanup(struct lavaInfo *lavaInfo);

int coglink_connectNode(struct lavaInfo *lavaInfo, struct discord *client, struct lavaNode *node);

#endif
