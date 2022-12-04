#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>
#include <concord/chash.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include <coglink/lavalink.h>
#include <coglink/lavalink-internal.h>
#include <coglink/definitions.h>
#include <coglink/player.h>

#define STRING_TABLE_HEAP 0
#define STRING_TABLE_BUCKET struct StringBucket
#define STRING_TABLE_HASH(key, hash) chash_string_hash(key, hash)
#define STRING_TABLE_FREE_KEY(key) free(key)
#define STRING_TABLE_FREE_VALUE(value) free(value)
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

void onCloseEvent(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t length) {
  (void) ws; (void) info; (void) length;

  struct lavaInfo *lavaInfo = data;

  if (lavaInfo->plugins && lavaInfo->plugins->events->onLavalinkClose[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onLavalinkClose[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onLavalinkClose[i](lavaInfo, info, wscode, reason, length);
        if (pluginResultCode != COGLINK_PROCEED) return;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onLavalinkClose[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onLavalinkClose[i](lavaInfoPlugin, info, wscode, reason, length);
        if (pluginResultCode != COGLINK_PROCEED) return;
      }
    }
  }

  if (lavaInfo->events->onClose) lavaInfo->events->onClose(wscode, reason);
}

void onTextEvent(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t length) {
  (void) ws; (void) info;
  struct lavaInfo *lavaInfo = data;

  if (lavaInfo->plugins && lavaInfo->plugins->events->onLavalinkPacketReceived[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onLavalinkPacketReceived[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onLavalinkPacketReceived[i](lavaInfo, text, length);
        if (pluginResultCode != COGLINK_PROCEED) return;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onLavalinkPacketReceived[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onLavalinkPacketReceived[i](lavaInfoPlugin, text, length);
        if (pluginResultCode != COGLINK_PROCEED) return;
      }
    }
  }

  if (lavaInfo->events->onRaw && lavaInfo->events->onRaw(lavaInfo, text, length) != COGLINK_PROCEED) return;

  jsmn_parser parser;
  jsmntok_t tokens[64];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, text, length, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[64];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, text, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return;
  }

  jsmnf_pair *op = jsmnf_find(pairs, text, "op", 2);
  if (__coglink_checkParse(lavaInfo, op, "op") != COGLINK_PROCEED) return;

  char Op[16];

  snprintf(Op, sizeof(Op), "%.*s", (int)op->v.len, text + op->v.pos);

  switch(Op[0]) {
    case 'r': { /* ready */
      jsmnf_pair *sessionId = jsmnf_find(pairs, text, "sessionId", 9);
      if (__coglink_checkParse(lavaInfo, sessionId, "sessionId") != COGLINK_PROCEED) return;

      char SessionId[LAVALINK_SESSIONID_LENGTH];

      snprintf(SessionId, sizeof(SessionId), "%.*s", (int)sessionId->v.len, text + sessionId->v.pos);

      strlcpy(lavaInfo->sessionId, SessionId, LAVALINK_SESSIONID_LENGTH);

      if (lavaInfo->allowResuming && lavaInfo->resumeKey[0] == '\0') {
        char resumeKey[] = { [8] = '\1' };
        __coglink_randomString(resumeKey, sizeof(resumeKey) - 1);

        strlcpy(lavaInfo->resumeKey, resumeKey, 8);

        char reqPath[27];
        snprintf(reqPath, sizeof(reqPath), "/sessions/%s", SessionId);

        char payload[27];
        snprintf(payload, sizeof(payload), "{\"resumingKey\":\"%s\"}", resumeKey);

        __coglink_performRequest(lavaInfo, NULL, &(struct __coglink_requestConfig) {
                                              .requestType = __COGLINK_PATCH_REQ,
                                              .path = reqPath,
                                              .pathLength = sizeof(reqPath),
                                              .useV3Path = true,
                                              .body = payload,
                                              .bodySize = sizeof(payload)
                                            });
      }

      if (lavaInfo->events->onConnect) lavaInfo->events->onConnect();
      break;
    }
    case 'e': {
      jsmnf_pair *type = jsmnf_find(pairs, text, "type", 4);
      if (__coglink_checkParse(lavaInfo, type, "type") != COGLINK_PROCEED) return;

      jsmnf_pair *jsmnf_guildId = jsmnf_find(pairs, text, "guildId", 7);
      if (__coglink_checkParse(lavaInfo, jsmnf_guildId, "guildId") != COGLINK_PROCEED) return;

      char Type[32], guildId[GUILD_ID_LENGTH];

      snprintf(Type, sizeof(Type), "%.*s", (int)type->v.len, text + type->v.pos);
      snprintf(guildId, sizeof(guildId), "%.*s", (int)jsmnf_guildId->v.len, text + jsmnf_guildId->v.pos);

      switch(Type[7]) {
        case 'a': { /* TrackStartEvent */
          if (!lavaInfo->events->onTrackStart) return;

          jsmnf_pair *track = jsmnf_find(pairs, text, "encodedTrack", 12);
          if (__coglink_checkParse(lavaInfo, track, "encodedTrack") != COGLINK_PROCEED) return;

          char Track[TRACK_LENGTH];

          snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

          lavaInfo->events->onTrackStart(Track, strtoull(guildId, NULL, 10));
          break;
        }
        case 'd': { /* TrackEndEvent */
          if (!lavaInfo->events->onTrackEnd) return;

          jsmnf_pair *reason = jsmnf_find(pairs, text, "reason", 6);
          if (__coglink_checkParse(lavaInfo, reason, "reason") != COGLINK_PROCEED) return;

          jsmnf_pair *track = jsmnf_find(pairs, text, "encodedTrack", 12);
          if (__coglink_checkParse(lavaInfo, track, "encodedTrack") != COGLINK_PROCEED) return;

          char Reason[16], Track[TRACK_LENGTH];

          snprintf(Reason, sizeof(Reason), "%.*s", (int)reason->v.len, text + reason->v.pos);
          snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

          lavaInfo->events->onTrackEnd(Track, Reason, strtoull(guildId, NULL, 10));
          break;
        }
        case 'c': { /* TrackExceptionEvent */
          if (!lavaInfo->events->onTrackException) return;

          jsmnf_pair *track = jsmnf_find(pairs, text, "encodedTrack", 12);
          if (__coglink_checkParse(lavaInfo, track, "encodedTrack") != COGLINK_PROCEED) return;

          char *path[] = { "exception", "message" };
          jsmnf_pair *message = jsmnf_find_path(pairs, text, path, 2);
          if (__coglink_checkParse(lavaInfo, message, "message") != COGLINK_PROCEED) return;

          path[1] = "severity";
          jsmnf_pair *severity = jsmnf_find_path(pairs, text, path, 2);
          if (__coglink_checkParse(lavaInfo, severity, "severity") != COGLINK_PROCEED) return;

          path[1] = "cause";
          jsmnf_pair *cause = jsmnf_find_path(pairs, text, path, 2);
          if (__coglink_checkParse(lavaInfo, cause, "cause") != COGLINK_PROCEED) return;

          char Track[TRACK_LENGTH], Message[128], Severity[16], Cause[256];

          snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);
          snprintf(Message, sizeof(Message), "%.*s", (int)message->v.len, text + message->v.pos);
          snprintf(Severity, sizeof(Severity), "%.*s", (int)severity->v.len, text + severity->v.pos);
          snprintf(Cause, sizeof(Cause), "%.*s", (int)cause->v.len, text + cause->v.pos);

          lavaInfo->events->onTrackException(Track, Message, Severity, Cause, strtoull(guildId, NULL, 10));
          break;
        }
        case 'u': { /* TrackStuckEvent */
          if (!lavaInfo->events->onTrackStuck) return;

          jsmnf_pair *track = jsmnf_find(pairs, text, "encodedTrack", 12);
          if (__coglink_checkParse(lavaInfo, track, "encodedTrack") != COGLINK_PROCEED) return;

          jsmnf_pair *thresholdMs = jsmnf_find(pairs, text, "thresholdMs", 11);
          if (__coglink_checkParse(lavaInfo, thresholdMs, "thresholdMs") != COGLINK_PROCEED) return;

          char Track[TRACK_LENGTH], ThresholdMs[16];

          snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);
          snprintf(ThresholdMs, sizeof(ThresholdMs), "%.*s", (int)thresholdMs->v.len, text + thresholdMs->v.pos);

          lavaInfo->events->onTrackStuck(Track, atoi(ThresholdMs), strtoull(guildId, NULL, 10));
          break;
        }
        case 't': { /* WebSocketClosedEvent */
          if (!lavaInfo->events->onWebSocketClosed) return;

          jsmnf_pair *code = jsmnf_find(pairs, text, "code", 4);
          if (__coglink_checkParse(lavaInfo, code, "code") != COGLINK_PROCEED) return;

          jsmnf_pair *reason = jsmnf_find(pairs, text, "reason", 6);
          if (__coglink_checkParse(lavaInfo, reason, "reason") != COGLINK_PROCEED) return;

          jsmnf_pair *byRemote = jsmnf_find(pairs, text, "byRemote", 8);
          if (__coglink_checkParse(lavaInfo, byRemote, "byRemote") != COGLINK_PROCEED) return;

          char Code[16], Reason[128], ByRemote[TRUE_FALSE_LENGTH];

          snprintf(Code, sizeof(Code), "%.*s", (int)code->v.len, text + code->v.pos);
          snprintf(Reason, sizeof(Reason), "%.*s", (int)reason->v.len, text + reason->v.pos);
          snprintf(ByRemote, sizeof(ByRemote), "%.*s", (int)byRemote->v.len, text + byRemote->v.pos);

          lavaInfo->events->onWebSocketClosed(atoi(Code), Reason, (ByRemote[0] == 't' ? 1 : 0), strtoull(guildId, NULL, 10));
          break;
        }
        default: {
          if (lavaInfo->events->onUnknownEvent) lavaInfo->events->onUnknownEvent(Type, text, strtoull(guildId, NULL, 10));
          break;
        }
      }
      break;
    }
    case 's': { /* Stats */
      if (!lavaInfo->events->onStats) return;

      jsmnf_pair *players = jsmnf_find(pairs, text, "players", 7);
      if (__coglink_checkParse(lavaInfo, players, "players") != COGLINK_PROCEED) return;

      jsmnf_pair *playingPlayers = jsmnf_find(pairs, text, "playingPlayers", 14);
      if (__coglink_checkParse(lavaInfo, playingPlayers, "playingPlayers") != COGLINK_PROCEED) return;

      jsmnf_pair *uptime = jsmnf_find(pairs, text, "uptime", 6);
      if (__coglink_checkParse(lavaInfo, uptime, "uptime") != COGLINK_PROCEED) return;
      char *path[] = { "memory", "free" };
      jsmnf_pair *lavaFree = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, lavaFree, "lavaFree") != COGLINK_PROCEED) return;

      path[1] = "used";
      jsmnf_pair *used = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, used, "used") != COGLINK_PROCEED) return;

      path[1] = "allocated";
      jsmnf_pair *allocated = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, allocated, "allocated") != COGLINK_PROCEED) return;

      path[1] = "reservable";
      jsmnf_pair *reservable = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, reservable, "reservable") != COGLINK_PROCEED) return;

      path[0] = "cpu";
      path[1] = "cores";
      jsmnf_pair *cores = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, cores, "cores") != COGLINK_PROCEED) return;

      path[1] = "systemLoad";
      jsmnf_pair *systemLoad = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, systemLoad, "systemLoad") != COGLINK_PROCEED) return;

      path[1] = "lavalinkLoad";
      jsmnf_pair *lavalinkLoad = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, lavalinkLoad, "lavalinkLoad") != COGLINK_PROCEED) return;

      char Players[8], PlayingPlayers[8], Uptime[32], Free[16], Used[16], Allocated[16], Reservable[16], Cores[8], SystemLoad[16], LavalinkLoad[16], Sent[16], Nulled[16], Deficit[16];

      path[0] = "frameStats";
      path[1] = "sent";
      jsmnf_pair *sent = jsmnf_find_path(pairs, text, path, 2);
      if (sent) {
        snprintf(Sent, sizeof(Sent), "%.*s", (int)sent->v.len, text + sent->v.pos);

        path[1] = "deficit";
        jsmnf_pair *deficit = jsmnf_find_path(pairs, text, path, 2);
        if (__coglink_checkParse(lavaInfo, deficit, "deficit") != COGLINK_PROCEED) return;

        snprintf(Deficit, sizeof(Deficit), "%.*s", (int)deficit->v.len, text + deficit->v.pos);

        path[1] = "nulled";
        jsmnf_pair *nulled = jsmnf_find_path(pairs, text, path, 2);
        if (__coglink_checkParse(lavaInfo, nulled, "nulled") != COGLINK_PROCEED) return;

        snprintf(Nulled, sizeof(Nulled), "%.*s", (int)nulled->v.len, text + nulled->v.pos);
      }

      snprintf(Players, sizeof(Players), "%.*s", (int)players->v.len, text + players->v.pos);
      snprintf(PlayingPlayers, sizeof(PlayingPlayers), "%.*s", (int)playingPlayers->v.len, text + playingPlayers->v.pos);
      snprintf(Uptime, sizeof(Uptime), "%.*s", (int)uptime->v.len, text + uptime->v.pos);
      snprintf(Free, sizeof(Free), "%.*s", (int)lavaFree->v.len, text + lavaFree->v.pos);
      snprintf(Used, sizeof(Used), "%.*s", (int)used->v.len, text + used->v.pos);
      snprintf(Allocated, sizeof(Allocated), "%.*s", (int)allocated->v.len, text + allocated->v.pos);
      snprintf(Reservable, sizeof(Reservable), "%.*s", (int)reservable->v.len, text + reservable->v.pos);
      snprintf(Cores, sizeof(Cores), "%.*s", (int)cores->v.len, text + cores->v.pos);
      snprintf(SystemLoad, sizeof(SystemLoad), "%.*s", (int)systemLoad->v.len, text + systemLoad->v.pos);
      snprintf(LavalinkLoad, sizeof(LavalinkLoad), "%.*s", (int)lavalinkLoad->v.len, text + lavalinkLoad->v.pos);

      if (sent) { if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> Players: %s\n> PlayingPlayers: %s\n> Uptime: %s\n> Free: %s\n> Used: %s\n> Allocated: %s\n> Reservable: %s\n> Cores: %s\n> SystemLoad: %s\n> LavalinkLoad: %s\n> Sent: %s\n> Nulled: %s\n> Deficit: %s", Players, PlayingPlayers, Uptime, Free, Used, Allocated, Reservable, Cores, SystemLoad, LavalinkLoad, Sent, Nulled, Deficit); }
      else if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> Players: %s\n> PlayingPlayers: %s\n> Uptime: %s\n> Free: %s\n> Used: %s\n> Allocated: %s\n> Reservable: %s\n> Cores: %s\n> SystemLoad: %s\n> LavalinkLoad: %s", Players, PlayingPlayers, Uptime, Free, Used, Allocated, Reservable, Cores, SystemLoad, LavalinkLoad);

      struct lavalinkStats *lavalinkStatsStruct = &(struct lavalinkStats) {
        .players = Players,
        .playingPlayers = PlayingPlayers,
        .uptime = Uptime,
        .memory = &(struct lavalinkStatsMemory) {
          .free = Free,
          .used = Used,
          .allocated = Allocated,
          .reservable = Reservable
        },
        .cpu = &(struct lavalinkStatsCPU) {
          .cores = Cores,
          .systemLoad = SystemLoad,
          .lavalinkLoad = LavalinkLoad
        },
        .frameStats = &(struct lavalinkStatsFrameStats) {
          .sent = (sent ? Sent : NULL),
          .nulled = (sent ? Nulled : NULL),
          .deficit = (sent ? Deficit : NULL)
        }
      };

      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-management] Set the value for struct members of lavalinkStatsStruct.");

      lavaInfo->events->onStats(lavalinkStatsStruct);
      break;
    }
    case 'p': { /* PlayerUpdate */
      if (!lavaInfo->events->onPlayerUpdate) return;

      jsmnf_pair *jsmnf_guildId = jsmnf_find(pairs, text, "guildId", 7);
      if (__coglink_checkParse(lavaInfo, jsmnf_guildId, "guildId") != COGLINK_PROCEED) return;

      char *path[] = { "state", "time" };
      jsmnf_pair *time = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, time, "time") != COGLINK_PROCEED) return;

      path[1] = "position";
      jsmnf_pair *position = jsmnf_find_path(pairs, text, path, 2);

      path[1] = "connected";
      jsmnf_pair *connected = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, connected, "connected") != COGLINK_PROCEED) return;

      path[1] = "ping";
      jsmnf_pair *ping = jsmnf_find_path(pairs, text, path, 2);

      char guildId[GUILD_ID_LENGTH], Time[16], Position[16], Connected[TRUE_FALSE_LENGTH], Ping[8];

      snprintf(guildId, sizeof(guildId), "%.*s", (int)jsmnf_guildId->v.len, text + jsmnf_guildId->v.pos);
      snprintf(Time, sizeof(Time), "%.*s", (int)time->v.len, text + time->v.pos);
      snprintf(Position, sizeof(Position), "%.*s", (int)position->v.len, text + position->v.pos);
      snprintf(Ping, sizeof(Ping), "%.*s", (int)ping->v.len, text + ping->v.pos);
      snprintf(Connected, sizeof(Connected), "%.*s", (int)connected->v.len, text + connected->v.pos);

      if (Position[0] != '0') lavaInfo->events->onPlayerUpdate(atof(Time), atoi(Position), (Connected[0] == 't' ? 1 : 0), atoi(Ping), strtoull(guildId, NULL, 10));
      else lavaInfo->events->onPlayerUpdate(atof(Time), 0, (Connected[0] == 't' ? 1 : 0), 0, strtoull(guildId, NULL, 10));
      break;
    }
    default: {
      if (lavaInfo->events->onUnknownOp) lavaInfo->events->onUnknownOp(Op, text);
      break;
    }
  }
}

enum discord_event_scheduler __coglink_handleScheduler(struct discord *client, const char data[], size_t length, enum discord_gateway_events event) {
  struct lavaInfo *lavaInfo = discord_get_data(client);

  if (lavaInfo->plugins && lavaInfo->plugins->events->onCoglinkScheduler[0]) {
    if (lavaInfo->plugins->security->allowReadBotToken) client->token = NULL;
    if (lavaInfo->plugins->security->allowReadConcordWebsocket) client->gw = (struct discord_gateway) { 0 };

    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onCoglinkScheduler[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onCoglinkScheduler[i](lavaInfo, client, data, length, event);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      client->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onCoglinkScheduler[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onCoglinkScheduler[i](lavaInfoPlugin, client, data, length, event);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    }
  }

  switch(event) {
    case DISCORD_EV_VOICE_STATE_UPDATE: {
      jsmn_parser parser;
      jsmntok_t tokens[128];

      jsmn_init(&parser);
      int r = jsmn_parse(&parser, data, length, tokens, sizeof(tokens));

      if (r < 0) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_loader loader;
      jsmnf_pair pairs[128];

      jsmnf_init(&loader);
      r = jsmnf_load(&loader, data, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

      if (r < 0) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", 8);
      if (__coglink_checkParse(lavaInfo, VGI, "guild_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      jsmnf_pair *VUI = jsmnf_find(pairs, data, "user_id", 7);
      if (__coglink_checkParse(lavaInfo, VUI, "user_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char guildId[GUILD_ID_LENGTH], userId[USER_ID_LENGTH];

      snprintf(guildId, sizeof(guildId), "%.*s", (int)VGI->v.len, data + VGI->v.pos);
      snprintf(userId, sizeof(userId), "%.*s", (int)VUI->v.len, data + VUI->v.pos);

      if (0 == strcmp(userId, lavaInfo->node->botId)) {
        jsmnf_pair *SSI = jsmnf_find(pairs, data, "session_id", 10);
        if (__coglink_checkParse(lavaInfo, SSI, "session_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

        char *sessionId = malloc(SESSION_ID_LENGTH);

        snprintf(sessionId, SESSION_ID_LENGTH, "%.*s", (int)SSI->v.len, data + SSI->v.pos);

        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging)  log_debug("[coglink:memory] Allocated %d bytes for sessionId to be saved in the hashtable.", SESSION_ID_LENGTH);
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_debug("[coglink:hashtable] Parsed voice state update json, results:\n> guild_id: %s\n> user_id: %s\n> session_id: %s", guildId, userId, sessionId);

        if (sessionId[0] != 'n') {
          if (!hashtable) {
            hashtable = chash_init(hashtable, STRING_TABLE);
            if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->chashSuccessDebugging) log_warn("[coglink:hashtable] Created hashtable, since it wasn't created before.");
          }

          chash_assign(hashtable, guildId, sessionId, STRING_TABLE);

          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->chashSuccessDebugging) log_debug("[coglink:hashtable] The user that got updated is the bot, saving the sessionId.");
        } else {
          if (!hashtable) return DISCORD_EVENT_IGNORE;

          int exists = chash_contains(hashtable, guildId, exists, STRING_TABLE);
          if (exists == 0) return DISCORD_EVENT_IGNORE;

          chash_delete(hashtable, guildId, STRING_TABLE);
          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->chashSuccessDebugging) log_debug("[coglink:hashtable] The user that got updated is the bot, but the sessionId is null, removing the sessionId from the hashtable.");
        }
      }
    } return DISCORD_EVENT_IGNORE;
    case DISCORD_EV_VOICE_SERVER_UPDATE: {
      jsmn_parser parser;
      jsmntok_t tokens[256];

      jsmn_init(&parser);
      int r = jsmn_parse(&parser, data, length, tokens, sizeof(tokens));

      if (r < 0) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[jsmn-find] Failed to parse JSON.");
        return DISCORD_EVENT_IGNORE;
      }
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

      jsmnf_loader loader;
      jsmnf_pair pairs[128];

      jsmnf_init(&loader);
      r = jsmnf_load(&loader, data, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

      if (r < 0) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
        return DISCORD_EVENT_IGNORE;
      }
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", 8);
      if (__coglink_checkParse(lavaInfo, VGI, "guild_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char guildId[GUILD_ID_LENGTH];
      snprintf(guildId, sizeof(guildId), "%.*s", (int)VGI->v.len, data + VGI->v.pos);

      if (!hashtable) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->chashErrorsDebugging) log_error("[coglink:jsmn-find] The hashtable is not initialized.");
        return DISCORD_EVENT_IGNORE;
      }
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->chashSuccessDebugging) log_debug("[coglink:jsmn-find] The hashtable is initialized.");

      int exists = chash_contains(hashtable, guildId, exists, STRING_TABLE);
      if (0 == exists) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->chashErrorsDebugging) log_error("[coglink:jsmn-find] The hashtable does not contain any data related to the guildId.");
        return DISCORD_EVENT_IGNORE;
      }
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->chashSuccessDebugging) log_debug("[coglink:jsmn-find] The hashtable contains the sessionId related to the guildId.");

      char *sessionId = chash_lookup(hashtable, guildId, sessionId, STRING_TABLE);

      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Successfully found the sessionID in the hashtable.");
  
      jsmnf_pair *token = jsmnf_find(pairs, data, "token", 5);
      if (__coglink_checkParse(lavaInfo, token, "token") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      jsmnf_pair *endpoint = jsmnf_find(pairs, data, "endpoint", 8);
      if (__coglink_checkParse(lavaInfo, endpoint, "endpoint") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char Token[DISCORD_TOKEN_LENGTH], Endpoint[ENDPOINT_LENGTH];

      snprintf(Token, sizeof(Token), "%.*s", (int)token->v.len, data + token->v.pos);
      snprintf(Endpoint, sizeof(Endpoint), "%.*s", (int)endpoint->v.len, data + endpoint->v.pos);

      char reqPath[64];
      snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%s", lavaInfo->sessionId, guildId);

      char payload[256];
      snprintf(payload, sizeof(payload), "{\"voice\":{\"token\":\"%s\",\"endpoint\":\"%s\",\"sessionId\":\"%s\"}}", Token, Endpoint, sessionId);

      __coglink_performRequest(lavaInfo, NULL, &(struct __coglink_requestConfig) {
                                            .requestType = __COGLINK_PATCH_REQ,
                                            .additionalDebuggingSuccess = lavaInfo->debugging->handleSchedulerVoiceServerDebugging,
                                            .additionalDebuggingError = lavaInfo->debugging->handleSchedulerVoiceServerDebugging,
                                            .path = reqPath,
                                            .pathLength = sizeof(reqPath),
                                            .useV3Path = true,
                                            .body = payload,
                                            .bodySize = sizeof(payload)
                                          });
    } return DISCORD_EVENT_IGNORE;
    default:
      return DISCORD_EVENT_MAIN_THREAD;
  }
}

void coglink_joinVoiceChannel(struct lavaInfo *lavaInfo, struct discord *client, u64snowflake voiceChannelId, u64snowflake guildId) {
  char joinVCPayload[512];
  snprintf(joinVCPayload, sizeof(joinVCPayload), "{\"op\":4,\"d\":{\"guild_id\":%"PRIu64",\"channel_id\":\"%"PRIu64"\",\"self_mute\":false,\"self_deaf\":true}}", guildId, voiceChannelId);

  if (!ws_send_text(client->gw.ws, NULL, joinVCPayload, strnlen(joinVCPayload, sizeof(joinVCPayload)))) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadErrorsDebugging) log_fatal("[coglink:libcurl] Something went wrong while sending a payload with op 4 to Discord.");
    return;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadSuccessDebugging) log_debug("[coglink:libcurl] Successfully sent the payload with op 4 to Discord.");
}

void coglink_freeNodeInfo(struct lavaInfo *lavaInfo) {
  if (lavaInfo) lavaInfo = NULL;
}

void coglink_disconnectNode(struct lavaInfo *lavaInfo) {
  ws_close(lavaInfo->ws, 1000, "Requested to be closed", sizeof("Requested to be closed"));
}

void coglink_setEvents(struct lavaInfo *lavaInfo, struct lavalinkEvents *lavalinkEvents) {
  lavaInfo->events = lavalinkEvents;
}

void coglink_connectNodeCleanup(struct lavaInfo *lavaInfo) {
  if (hashtable) chash_free(hashtable, STRING_TABLE);
  ws_close(lavaInfo->ws, 1000, "Normal close", 14);
  io_poller_curlm_del(lavaInfo->io_poller, lavaInfo->mhandle);
  ws_cleanup(lavaInfo->ws);
  curl_multi_cleanup(lavaInfo->mhandle);
  curl_global_cleanup();
  coglink_freeNodeInfo(lavaInfo);
}

int coglink_connectNode(struct lavaInfo *lavaInfo, struct discord *client, struct lavalinkNode *node) {
  curl_global_init(CURL_GLOBAL_ALL);

  discord_set_data(client, lavaInfo);
  discord_set_event_scheduler(client, __coglink_handleScheduler);

  lavaInfo->node = node;
  lavaInfo->tstamp = (uint64_t)0;
  lavaInfo->mhandle = curl_multi_init();

  io_poller_curlm_add(client->io_poller, lavaInfo->mhandle, __coglink_IOPoller, lavaInfo);
  io_poller_curlm_enable_perform(client->io_poller, lavaInfo->mhandle);
  lavaInfo->io_poller = client->io_poller;

  struct ws_callbacks callbacks = {
    .on_text = &onTextEvent,
    .on_close = &onCloseEvent,
    .data = (void *)lavaInfo
  };
  struct websockets *ws = ws_init(&callbacks, client->gw.mhandle, NULL);

  char hostname[strnlen(node->hostname, 128) + (node->ssl ? 21 : 20)];
  if (node->ssl) snprintf(hostname, sizeof(hostname), "wss://%s/v3/websocket", node->hostname);
  else snprintf(hostname, sizeof(hostname), "ws://%s/v3/websocket", node->hostname);

  ws_set_url(ws, hostname, NULL);
  ws_start(ws);

  if (lavaInfo->resumeKey[0] != '\0') ws_add_header(ws, "Resume-Key", lavaInfo->resumeKey);
  ws_add_header(ws, "Authorization", node->password);
  ws_add_header(ws, "Num-Shards", node->shards);
  ws_add_header(ws, "User-Id", node->botId);
  ws_add_header(ws, "Client-Name", "Coglink");

  lavaInfo->ws = ws;

  return COGLINK_SUCCESS;
}
