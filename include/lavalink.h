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
  int ssl;
};

struct lavaMemory {
  char reservable[32];
  char used[32];
  char free[32];
  char allocated[32];
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
  int allDebugging;
  int sendPayloadErrorsDebugging;
  int sendPayloadSuccessDebugging;
  int checkParseErrorsDebugging;
  int checkParseSuccessDebugging;
  int getCachedUserVoiceIdErrorsDebugging;
  int getCachedUserVoiceIdSuccessDebugging;
  int joinVoiceDebugging;
  int jsmnfErrorsDebugging;
  int jsmnfSuccessDebugging;
  int handleSchedulerVoiceStateDebugging;
  int handleSchedulerVoiceServerDebugging;
  int chashErrorsDebugging;
  int chashSuccessDebugging;
  int parseTrackErrorsDebugging;
  int parseTrackSuccessDebugging;
  int parsePlaylistErrorsDebugging;
  int parsePlaylistSuccessDebugging;
  int parseErrorErrorsDebugging;
  int parseErrorSuccessDebugging;
  int parseLoadtypeErrorsDebugging;
  int parseLoadtypeSuccessDebugging;
  int searchSongErrorsDebugging;
  int searchSongSuccessDebugging;
  int curlErrorsDebugging;
  int curlSuccessDebugging;
  int memoryDebugging;
};

struct lavaInfo {
  struct lavaEvents *events;
  CURLM *mhandle;
  struct websockets *ws;
  uint64_t tstamp;
  struct lavaNode node;
  struct coglinkDebugging *debugging;
  int allowJoinVoiceCaching;
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
  void (*onConnect)();
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

#define STRING_TABLE_HEAP 1
#define STRING_TABLE_BUCKET struct StringBucket
#define STRING_TABLE_HASH(key, hash) chash_string_hash(key, hash)
#define STRING_TABLE_FREE_KEY(key) NULL
#define STRING_TABLE_FREE_VALUE(value)// free(value)
#define STRING_TABLE_COMPARE(cmp_a, cmp_b) (0 == strcmp(cmp_a, cmp_b))//chash_string_compare(cmp_a, cmp_b)
#define STRING_TABLE_INIT(bucket, _key, _value) chash_default_init(bucket, _key, _value)

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

void onConnectEvent(void *data, struct websockets *ws, struct ws_info *info, const char *protocols);

void onCloseEvent(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t len);

void onTextEvent(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t len);

void coglink_wsLoop(struct lavaInfo *lavaInfo);

void coglink_joinVoiceChannel(struct lavaInfo lavaInfo, struct discord *client, u64snowflake voiceChannelId, u64snowflake guildId);

int coglink_getCachedUserVoiceId(struct lavaInfo lavaInfo, char *userId, char **voiceId);

int coglink_handleScheduler(struct lavaInfo *lavaInfo, struct discord *client, const char data[], size_t size, enum discord_gateway_events event);

void coglink_freeNodeInfo(struct lavaInfo *lavaInfo);

void coglink_disconnectNode(struct lavaInfo *lavaInfo);

void coglink_connectNodeCleanup(struct lavaInfo *lavaInfo);

int coglink_connectNode(struct lavaInfo *lavaInfo, struct lavaNode node);

#endif