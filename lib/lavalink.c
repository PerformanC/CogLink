#include <stdio.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>
#include <concord/chash.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include <coglink/lavalink-internal.h>
#include <coglink/definations.h>

/*
  STRUCTURES
*/

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
  char reservable[16];
  char used[16];
  char free[32];
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

struct lavaInfo {
  struct lavaEvents *events;
  CURLM *mhandle;
  struct websockets *ws;
  uint64_t tstamp;
  struct lavaNode *node;
  int debug;
};

struct lavaSong {
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

/*
  HASHTABLE
*/

#define STRING_TABLE_HEAP 0
#define STRING_TABLE_BUCKET struct StringBucket
#define STRING_TABLE_FREE_KEY(key) free(key)
#define STRING_TABLE_HASH(key, hash) chash_string_hash(key, hash)
#define STRING_TABLE_FREE_VALUE(value) // free(value)
#define STRING_TABLE_COMPARE(cmp_a, cmp_b) chash_string_compare(cmp_a, cmp_b)
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

struct StringHashtable *hashtable = NULL;

/*
  EVENTS
*/

void onConnectEvent(void *data, struct websockets *ws, struct ws_info *info, const char *protocols) {
  (void)ws; (void)info; (void)protocols;
  struct lavaInfo *lavaInfo = data;
  if (lavaInfo->events->onConnect) lavaInfo->events->onConnect();
}

void onCloseEvent(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t len) {
  (void) ws; (void) info; (void) len;
  struct lavaInfo *lavaInfo = data;
  if (lavaInfo->events->onClose) lavaInfo->events->onClose(wscode, reason);
}

void onTextEvent(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t len) {
  (void) ws; (void) info;
  struct lavaInfo *lavaInfo = data;

  if (lavaInfo->events->onRaw && lavaInfo->events->onRaw(lavaInfo, text, len) != COGLINK_PROCEED) return;

  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, text, len, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, text, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return;
  }

  jsmnf_pair *op = jsmnf_find(pairs, text, "op", 2);
  if (__coglink_checkParse(lavaInfo, op, "op") != COGLINK_PROCEED) return;

  char Op[16];
  snprintf(Op, sizeof(Op), "%.*s", (int)op->v.len, text + op->v.pos);

  if (0 == strcmp(Op, "event")) {
    jsmnf_pair *type = jsmnf_find(pairs, text, "type", 4);

    if (__coglink_checkParse(lavaInfo, type, "type") != COGLINK_PROCEED) return;

    char Type[32];
    snprintf(Type, sizeof(Type), "%.*s", (int)type->v.len, text + type->v.pos);

    jsmnf_pair *jsmnf_guildId = jsmnf_find(pairs, text, "guildId", 7);
    if (__coglink_checkParse(lavaInfo, jsmnf_guildId, "guildId") != COGLINK_PROCEED) return;

    char guildId[32];
    snprintf(guildId, sizeof(guildId), "%.*s", (int)jsmnf_guildId->v.len, text + jsmnf_guildId->v.pos);

    if (0 == strcmp(Type, "TrackStartEvent")) {
      jsmnf_pair *track = jsmnf_find(pairs, text, "track", 5);
      if (__coglink_checkParse(lavaInfo, track, "track") != COGLINK_PROCEED) return;

      char Track[512];
      snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

      if (lavaInfo->events->onTrackStart) lavaInfo->events->onTrackStart(Track, strtoull(guildId, NULL, 10));
    } else if (0 == strcmp(Type, "TrackEndEvent")) {
      jsmnf_pair *reason = jsmnf_find(pairs, text, "reason", 6);
      if (__coglink_checkParse(lavaInfo, reason, "reason") != COGLINK_PROCEED) return;

      char Reason[16];
      snprintf(Reason, sizeof(Reason), "%.*s", (int)reason->v.len, text + reason->v.pos);

      jsmnf_pair *track = jsmnf_find(pairs, text, "track", 5);
      if (__coglink_checkParse(lavaInfo, track, "track") != COGLINK_PROCEED) return;

      char Track[512];
      snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

      if (lavaInfo->events->onTrackEnd) lavaInfo->events->onTrackEnd(Track, Reason, strtoull(guildId, NULL, 10));
    } else if (0 == strcmp(Type, "TrackExceptionEvent")) {
      jsmnf_pair *track = jsmnf_find(pairs, text, "track", 5);
      if (__coglink_checkParse(lavaInfo, track, "track") != COGLINK_PROCEED) return;

      char Track[512];
      snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

      char *path[] = { "exception", "message" };
      jsmnf_pair *message = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, message, "message") != COGLINK_PROCEED) return;

      char Message[216];
      snprintf(Message, sizeof(Message), "%.*s", (int)message->v.len, text + message->v.pos);

      path[1] = "severity";
      jsmnf_pair *severity = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, severity, "severity") != COGLINK_PROCEED) return;

      char Severity[16];
      snprintf(Severity, sizeof(Severity), "%.*s", (int)severity->v.len, text + severity->v.pos);

      path[1] = "cause";
      jsmnf_pair *cause = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, cause, "cause") != COGLINK_PROCEED) return;

      char Cause[512];
      snprintf(Cause, sizeof(Cause), "%.*s", (int)cause->v.len, text + cause->v.pos);

      if (lavaInfo->events->onTrackException) lavaInfo->events->onTrackException(Track, Message, Severity, Cause, strtoull(guildId, NULL, 10));
    } else if (0 == strcmp(Type, "TrackStuckEvent")) {
      jsmnf_pair *track = jsmnf_find(pairs, text, "track", 5);
      if (__coglink_checkParse(lavaInfo, track, "track") != COGLINK_PROCEED) return;

      char Track[512];
      snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

      jsmnf_pair *thresholdMs = jsmnf_find(pairs, text, "thresholdMs", 11);
      if (__coglink_checkParse(lavaInfo, thresholdMs, "thresholdMs") != COGLINK_PROCEED) return;

      char ThresholdMs[16];
      snprintf(ThresholdMs, sizeof(ThresholdMs), "%.*s", (int)thresholdMs->v.len, text + thresholdMs->v.pos);

      if (lavaInfo->events->onTrackStuck) lavaInfo->events->onTrackStuck(Track, atoi(ThresholdMs), strtoull(guildId, NULL, 10));
    } else if (0 == strcmp(Type, "WebSocketClosedEvent")) {
      jsmnf_pair *code = jsmnf_find(pairs, text, "code", 4);
      if (__coglink_checkParse(lavaInfo, code, "code") != COGLINK_PROCEED) return;

      char Code[16];
      snprintf(Code, sizeof(Code), "%.*s", (int)code->v.len, text + code->v.pos);

      jsmnf_pair *reason = jsmnf_find(pairs, text, "reason", 6);
      if (__coglink_checkParse(lavaInfo, reason, "reason") != COGLINK_PROCEED) return;

      char Reason[216];
      snprintf(Reason, sizeof(Reason), "%.*s", (int)reason->v.len, text + reason->v.pos);

      jsmnf_pair *byRemote = jsmnf_find(pairs, text, "byRemote", 8);
      if (__coglink_checkParse(lavaInfo, byRemote, "byRemote") != COGLINK_PROCEED) return;

      char ByRemote[8];
      snprintf(ByRemote, sizeof(ByRemote), "%.*s", (int)byRemote->v.len, text + byRemote->v.pos);

      if (lavaInfo->events->onWebSocketClosed) lavaInfo->events->onWebSocketClosed(atoi(Code), Reason, ByRemote, strtoull(guildId, NULL, 10));
    } else {
      if (lavaInfo->events->onUnknownEvent) lavaInfo->events->onUnknownEvent(Type, text, strtoull(guildId, NULL, 10));
    }
  } else if (0 == strcmp(Op, "stats")) {
    log_warn("%s", text);
    jsmnf_pair *playingPlayers = jsmnf_find(pairs, text, "playingPlayers", 14);
    if (__coglink_checkParse(lavaInfo, playingPlayers, "playingPlayers") != COGLINK_PROCEED) return;

    char PlayingPlayers[8];
    snprintf(PlayingPlayers, sizeof(PlayingPlayers), "%.*s", (int)playingPlayers->v.len, text + playingPlayers->v.pos);

    struct lavaMemory infoMemory = { 0 };

    char *path[] = { "memory", "reservable" };
    jsmnf_pair *reservable = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, reservable, "reservable") != COGLINK_PROCEED) return;

    snprintf(infoMemory.reservable, 16, "%.*s", (int)reservable->v.len, text + reservable->v.pos);

    path[1] = "used";
    jsmnf_pair *used = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, used, "used") != COGLINK_PROCEED) return;

    snprintf(infoMemory.used, 16, "%.*s", (int)used->v.len, text + used->v.pos);

    path[1] = "free";
    jsmnf_pair *lavaFree = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, lavaFree, "lavaFree") != COGLINK_PROCEED) return;

    snprintf(infoMemory.free, 32, "%.*s", (int)lavaFree->v.len, text + lavaFree->v.pos);

    path[1] = "allocated";
    jsmnf_pair *allocated = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, allocated, "allocated") != COGLINK_PROCEED) return;

    snprintf(infoMemory.allocated, 16, "%.*s", (int)allocated->v.len, text + allocated->v.pos);

    jsmnf_pair *players = jsmnf_find(pairs, text, "players", 7);
    if (__coglink_checkParse(lavaInfo, players, "players") != COGLINK_PROCEED) return;

    char Players[8];
    snprintf(Players, sizeof(Players), "%.*s", (int)players->v.len, text + players->v.pos);

    struct lavaFStats infoFrameStats = { 0 };

    path[0] = "frameStats";
    path[1] = "sent";
    jsmnf_pair *sent = jsmnf_find_path(pairs, text, path, 2);
    if (sent) {
      snprintf(infoFrameStats.sent, 16, "%.*s", (int)sent->v.len, text + sent->v.pos);
    
      path[1] = "deficit";
      jsmnf_pair *deficit = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, deficit, "deficit") != COGLINK_PROCEED) return;

      snprintf(infoFrameStats.deficit, 16, "%.*s", (int)deficit->v.len, text + deficit->v.pos);

      path[1] = "nulled";
      jsmnf_pair *nulled = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, nulled, "nulled") != COGLINK_PROCEED) return;

      snprintf(infoFrameStats.nulled, 16, "%.*s", (int)nulled->v.len, text + nulled->v.pos);
    }

    struct lavaCPU infoCPU = { 0 };

    path[0] = "cpu";
    path[1] = "cores";
    jsmnf_pair *cores = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, cores, "cores") != COGLINK_PROCEED) return;

    snprintf(infoCPU.cores, 8, "%.*s", (int)cores->v.len, text + cores->v.pos);

    path[1] = "systemLoad";
    jsmnf_pair *systemLoad = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, systemLoad, "systemLoad") != COGLINK_PROCEED) return;

    snprintf(infoCPU.systemLoad, 16, "%.*s", (int)systemLoad->v.len, text + systemLoad->v.pos);

    path[1] = "lavalinkLoad";
    jsmnf_pair *lavalinkLoad = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, lavalinkLoad, "lavalinkLoad") != COGLINK_PROCEED) return;

    snprintf(infoCPU.lavalinkLoad, 16, "%.*s", (int)lavalinkLoad->v.len, text + lavalinkLoad->v.pos);

    jsmnf_pair *uptime = jsmnf_find(pairs, text, "uptime", 6);
    if (__coglink_checkParse(lavaInfo, uptime, "uptime") != COGLINK_PROCEED) return;

    char Uptime[32];
    snprintf(Uptime, sizeof(Uptime), "%.*s", (int)uptime->v.len, text + uptime->v.pos);

    if (lavaInfo->events->onStats) lavaInfo->events->onStats(atoi(PlayingPlayers), &infoMemory, atoi(Players), &infoFrameStats, &infoCPU, atoi(Uptime));
  } else if (0 == strcmp(Op, "playerUpdate")) {
    jsmnf_pair *jsmnf_guildId = jsmnf_find(pairs, text, "guildId", 7);
    if (__coglink_checkParse(lavaInfo, jsmnf_guildId, "guildId") != COGLINK_PROCEED) return;

    char guildId[32];
    snprintf(guildId, sizeof(guildId), "%.*s", (int)jsmnf_guildId->v.len, text + jsmnf_guildId->v.pos);

    char *path[] = { "state", "time" };
    jsmnf_pair *time = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, time, "time") != COGLINK_PROCEED) return;

    char Time[32];
    snprintf(Time, sizeof(Time), "%.*s", (int)time->v.len, text + time->v.pos);

    path[1] = "position";
    jsmnf_pair *position = jsmnf_find_path(pairs, text, path, 2);

    char Position[32];
    if (position) snprintf(Position, sizeof(Position), "%.*s", (int)position->v.len, text + position->v.pos);
    else snprintf(Position, sizeof(Position), "NULL");

    path[1] = "connected";
    jsmnf_pair *connected = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, connected, "connected") != COGLINK_PROCEED) return;

    char Connected[8];
    snprintf(Connected, sizeof(Connected), "%.*s", (int)connected->v.len, text + connected->v.pos);

    path[1] = "ping";
    jsmnf_pair *ping = jsmnf_find_path(pairs, text, path, 2);
    if (__coglink_checkParse(lavaInfo, ping, "ping") != COGLINK_PROCEED) return;

    char Ping[8];
    snprintf(Ping, sizeof(Ping), "%.*s", (int)ping->v.len, text + ping->v.pos);

    if (lavaInfo->events->onPlayerUpdate) lavaInfo->events->onPlayerUpdate(atoi(Time), atoi(Position), Connected, atoi(Ping), strtoull(guildId, NULL, 10));
  } else {
    if (lavaInfo->events->onUnknownOp) lavaInfo->events->onUnknownOp(Op, text);
  }
}

/*
  EVENTS
*/

void coglink_parseCleanup(struct lavaSong *songStruct) {
  free(songStruct);
}

void coglink_wsLoop(struct lavaInfo *lavaInfo) {
  ws_easy_run(lavaInfo->ws, 5, &lavaInfo->tstamp);
}

void coglink_playSong(struct lavaInfo *lavaInfo, char *track, u64snowflake guildId) {
  char payload[2048];
  snprintf(payload, sizeof(payload), "{\"op\":\"play\",\"guildId\":\"%"PRIu64"\",\"track\":\"%s\",\"noReplace\":false,\"pause\":false}", guildId, track);

  __coglink_sendPayload(lavaInfo, payload, "play");
}

void coglink_joinVoiceChannel(struct lavaInfo *lavaInfo, struct discord *client, u64snowflake voiceChannelId, u64snowflake guildId) {
  char joinVCPayload[512];
  snprintf(joinVCPayload, sizeof(joinVCPayload), "{\"op\":4,\"d\":{\"guild_id\":%"PRIu64",\"channel_id\":\"%"PRIu64"\",\"self_mute\":false,\"self_deaf\":true}}", guildId, voiceChannelId);

  if (ws_send_text(client->gw.ws, NULL, joinVCPayload, strlen(joinVCPayload)) == false) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] Something went wrong while sending a payload with op 4 to Discord.");
    return;
  } else {
    if (lavaInfo->debug) log_debug("[coglink:libcurl] Successfully sent the payload with op 4 to Discord.");
  }
}

int coglink_handleScheduler(struct lavaInfo *lavaInfo, struct discord *client, const char data[], size_t size, enum discord_gateway_events event) {
  (void)client;
  switch (event) {
    case DISCORD_EV_VOICE_STATE_UPDATE: {
      jsmn_parser parser;
      jsmntok_t tokens[256];

      jsmn_init(&parser);
      int r = jsmn_parse(&parser, data, size, tokens, sizeof(tokens));

      if (r < 0) {
        if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to parse JSON.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_loader loader;
      jsmnf_pair pairs[128];

      jsmnf_init(&loader);
      r = jsmnf_load(&loader, data, tokens, parser.toknext, pairs, 128);

      if (r < 0) {
        if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", 8);

      if (__coglink_checkParse(lavaInfo, VGI, "guild_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char guildId[32];
      snprintf(guildId, sizeof(guildId), "%.*s", (int)VGI->v.len, data + VGI->v.pos);

      jsmnf_pair *VUI = jsmnf_find(pairs, data, "user_id", 7);

      if (__coglink_checkParse(lavaInfo, VUI, "user_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char userId[32];
      snprintf(userId, sizeof(userId), "%.*s", (int)VUI->v.len, data + VUI->v.pos);

      jsmnf_pair *SSI = jsmnf_find(pairs, data, "session_id", 10);

      if (__coglink_checkParse(lavaInfo, SSI, "session_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char *sessionId = malloc(128);
      snprintf(sessionId, 128, "%.*s", (int)SSI->v.len, data + SSI->v.pos);

      if (lavaInfo->debug) log_debug("[coglink:jsmn-find] Parsed voice state update json, results:\n> guild_id: %s\n> user_id: %s\n> session_id: %s", guildId, userId, sessionId);

      if (0 == strcmp(userId, lavaInfo->node->botId)) {
        if (0 != strcmp(sessionId, "null")) {
          if (!hashtable) {
            hashtable = chash_init(hashtable, STRING_TABLE);
          }

          chash_assign(hashtable, guildId, sessionId, STRING_TABLE);

          if (lavaInfo->debug) log_debug("[coglink:jsmn-find] The user that got updated is the bot, saving the sessionId.");
        } else {
          if (!hashtable) return DISCORD_EVENT_IGNORE;

          int exists = chash_contains(hashtable, guildId, exists, STRING_TABLE);
          if (exists == 0) return DISCORD_EVENT_IGNORE;

          chash_delete(hashtable, guildId, STRING_TABLE);
          if (lavaInfo->debug) log_debug("[coglink:jsmn-find] The user that got updated is the bot, but the sessionId is null, removing the sessionId from the hashtable.");
        }
      }

    } return DISCORD_EVENT_IGNORE;
    case DISCORD_EV_VOICE_SERVER_UPDATE: {
      jsmn_parser parser;
      jsmntok_t tokens[256];

      jsmn_init(&parser);
      int r = jsmn_parse(&parser, data, size, tokens, sizeof(tokens));

      if (r < 0) {
        if (lavaInfo->debug) log_error("[jsmn-find] Failed to parse JSON.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_loader loader;
      jsmnf_pair pairs[128];

      jsmnf_init(&loader);
      r = jsmnf_load(&loader, data, tokens, parser.toknext, pairs, 128);

      if (r < 0) {
        if (lavaInfo->debug) log_error("[jsmn-find] Failed to load jsmn-find.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", 8);

      if (__coglink_checkParse(lavaInfo, VGI, "guild_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char guildId[32];
      snprintf(guildId, sizeof(guildId), "%.*s", (int)VGI->v.len, data + VGI->v.pos);

      if (!hashtable) {
        if (lavaInfo->debug) log_error("[coglink:jsmn-find] The hashtable is not initialized.");
        return DISCORD_EVENT_IGNORE;
      }

      int exists = chash_contains(hashtable, guildId, exists, STRING_TABLE);
      if (0 == exists) {
        if (lavaInfo->debug) log_error("[coglink:jsmn-find] The hashtable does not contain the guildId.");
        return DISCORD_EVENT_IGNORE;
      }

      char *sessionID = chash_lookup(hashtable, guildId, sessionID, STRING_TABLE);
  
      char VUP[1024];
      snprintf(VUP, sizeof(VUP), "{\"op\":\"voiceUpdate\",\"guildId\":\"%s\",\"sessionId\":\"%s\",\"event\":%.*s}", guildId, sessionID, (int)size, data);

      __coglink_sendPayload(lavaInfo, VUP, "voiceUpdate");
    } return DISCORD_EVENT_IGNORE;
    default:
      return DISCORD_EVENT_MAIN_THREAD;
  }
}

void coglink_connectNodeCleanup(struct lavaInfo *lavaInfo) {
  if (hashtable) chash_free(hashtable, STRING_TABLE);
  ws_end(lavaInfo->ws);
  ws_cleanup(lavaInfo->ws);
  curl_multi_cleanup(lavaInfo->mhandle);
}

int coglink_connectNode(struct lavaInfo *lavaInfo, struct lavaNode *node) {
  struct ws_callbacks callbacks = {
    .on_text = &onTextEvent,
    .on_connect = &onConnectEvent,
    .on_close = &onCloseEvent,
    .data = (void *)lavaInfo
  };

  CURLM *mhandle = curl_multi_init();
  struct websockets *ws = ws_init(&callbacks, mhandle, NULL);

  char hostname[strlen(node->hostname) + 7];
  if (node->ssl) snprintf(hostname, sizeof(hostname), "wss://%s", node->hostname);
  else snprintf(hostname, sizeof(hostname), "ws://%s", node->hostname);

  ws_set_url(ws, hostname, NULL);

  ws_start(ws);

  ws_add_header(ws, "Authorization", node->password);
  ws_add_header(ws, "Num-Shards", node->shards);
  ws_add_header(ws, "User-Id", node->botId);
  ws_add_header(ws, "Client-Name", "coglink");

  lavaInfo->mhandle = mhandle;
  lavaInfo->ws = ws;
  uint64_t tstamp = 0;
  lavaInfo->tstamp = tstamp;
  lavaInfo->node = node;

  return COGLINK_SUCCESS;
}