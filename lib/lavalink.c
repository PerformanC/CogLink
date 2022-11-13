#include <stdio.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>
#include <concord/chash.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include <coglink/lavalink.h>
#include <coglink/lavalink-internal.h>
#include <coglink/definations.h>

#define STRING_TABLE_HEAP 0
#define STRING_TABLE_BUCKET struct StringBucket
#define STRING_TABLE_HASH(key, hash) chash_string_hash(key, hash)
#define STRING_TABLE_FREE_KEY(key) NULL
#define STRING_TABLE_FREE_VALUE(value) NULL
#define STRING_TABLE_COMPARE(cmp_a, cmp_b) chash_string_compare(cmp_a, cmp_b)
#define STRING_TABLE_INIT(bucket, _key, _value) chash_default_init(bucket, _key, _value)

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
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, text, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return;
  }

  jsmnf_pair *op = jsmnf_find(pairs, text, "op", 2);
  if (__coglink_checkParse(lavaInfo, op, "op") != COGLINK_PROCEED) return;

  char Op[16];
  snprintf(Op, sizeof(Op), "%.*s", (int)op->v.len, text + op->v.pos);

  switch(Op[0]) {
    case 'e': {
      jsmnf_pair *type = jsmnf_find(pairs, text, "type", 4);
      if (__coglink_checkParse(lavaInfo, type, "type") != COGLINK_PROCEED) return;

      char Type[32];
      snprintf(Type, sizeof(Type), "%.*s", (int)type->v.len, text + type->v.pos);

      jsmnf_pair *jsmnf_guildId = jsmnf_find(pairs, text, "guildId", 7);
      if (__coglink_checkParse(lavaInfo, jsmnf_guildId, "guildId") != COGLINK_PROCEED) return;

      char guildId[GUILD_ID_LENGTH];
      snprintf(guildId, sizeof(guildId), "%.*s", (int)jsmnf_guildId->v.len, text + jsmnf_guildId->v.pos);

      switch(Type[7]) {
        case 'a': { /* TrackStartEvent */
          jsmnf_pair *track = jsmnf_find(pairs, text, "track", 5);
          if (__coglink_checkParse(lavaInfo, track, "track") != COGLINK_PROCEED) return;

          char Track[TRACK_LENGTH];
          snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

          if (lavaInfo->events->onTrackStart) lavaInfo->events->onTrackStart(Track, strtoull(guildId, NULL, 10));
          break;
        }
        case 'd': { /* TrackEndEvent */
          jsmnf_pair *reason = jsmnf_find(pairs, text, "reason", 6);
          if (__coglink_checkParse(lavaInfo, reason, "reason") != COGLINK_PROCEED) return;

          char Reason[16];
          snprintf(Reason, sizeof(Reason), "%.*s", (int)reason->v.len, text + reason->v.pos);

          jsmnf_pair *track = jsmnf_find(pairs, text, "track", 5);
          if (__coglink_checkParse(lavaInfo, track, "track") != COGLINK_PROCEED) return;

          char Track[TRACK_LENGTH];
          snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

          if (lavaInfo->events->onTrackEnd) lavaInfo->events->onTrackEnd(Track, Reason, strtoull(guildId, NULL, 10));
          break;
        }
        case 'e': { /* TrackExceptionEvent */
          jsmnf_pair *track = jsmnf_find(pairs, text, "track", 5);
          if (__coglink_checkParse(lavaInfo, track, "track") != COGLINK_PROCEED) return;

          char Track[TRACK_LENGTH];
          snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

          char *path[] = { "exception", "message" };
          jsmnf_pair *message = jsmnf_find_path(pairs, text, path, 2);
          if (__coglink_checkParse(lavaInfo, message, "message") != COGLINK_PROCEED) return;

          char Message[128];
          snprintf(Message, sizeof(Message), "%.*s", (int)message->v.len, text + message->v.pos);

          path[1] = "severity";
          jsmnf_pair *severity = jsmnf_find_path(pairs, text, path, 2);
          if (__coglink_checkParse(lavaInfo, severity, "severity") != COGLINK_PROCEED) return;

          char Severity[16];
          snprintf(Severity, sizeof(Severity), "%.*s", (int)severity->v.len, text + severity->v.pos);

          path[1] = "cause";
          jsmnf_pair *cause = jsmnf_find_path(pairs, text, path, 2);
          if (__coglink_checkParse(lavaInfo, cause, "cause") != COGLINK_PROCEED) return;

          char Cause[256];
          snprintf(Cause, sizeof(Cause), "%.*s", (int)cause->v.len, text + cause->v.pos);

          if (lavaInfo->events->onTrackException) lavaInfo->events->onTrackException(Track, Message, Severity, Cause, strtoull(guildId, NULL, 10));
          break;
        }
        case 'c': { /* TrackStuckEvent */
          jsmnf_pair *track = jsmnf_find(pairs, text, "track", 5);
          if (__coglink_checkParse(lavaInfo, track, "track") != COGLINK_PROCEED) return;

          char Track[TRACK_LENGTH];
          snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

          jsmnf_pair *thresholdMs = jsmnf_find(pairs, text, "thresholdMs", 11);
          if (__coglink_checkParse(lavaInfo, thresholdMs, "thresholdMs") != COGLINK_PROCEED) return;

          char ThresholdMs[16];
          snprintf(ThresholdMs, sizeof(ThresholdMs), "%.*s", (int)thresholdMs->v.len, text + thresholdMs->v.pos);

          if (lavaInfo->events->onTrackStuck) lavaInfo->events->onTrackStuck(Track, atoi(ThresholdMs), strtoull(guildId, NULL, 10));
          break;
        }
        case 't': { /* WebSocketClosedEvent */
          jsmnf_pair *code = jsmnf_find(pairs, text, "code", 4);
          if (__coglink_checkParse(lavaInfo, code, "code") != COGLINK_PROCEED) return;

          char Code[16];
          snprintf(Code, sizeof(Code), "%.*s", (int)code->v.len, text + code->v.pos);

          jsmnf_pair *reason = jsmnf_find(pairs, text, "reason", 6);
          if (__coglink_checkParse(lavaInfo, reason, "reason") != COGLINK_PROCEED) return;

          char Reason[128];
          snprintf(Reason, sizeof(Reason), "%.*s", (int)reason->v.len, text + reason->v.pos);

          jsmnf_pair *byRemote = jsmnf_find(pairs, text, "byRemote", 8);
          if (__coglink_checkParse(lavaInfo, byRemote, "byRemote") != COGLINK_PROCEED) return;

          char ByRemote[TRUE_FALSE_LENGTH];
          snprintf(ByRemote, sizeof(ByRemote), "%.*s", (int)byRemote->v.len, text + byRemote->v.pos);

          if (lavaInfo->events->onWebSocketClosed) lavaInfo->events->onWebSocketClosed(atoi(Code), Reason, ByRemote, strtoull(guildId, NULL, 10));
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
      break;
    }
    case 'p': { /* PlayerUpdate */
      jsmnf_pair *jsmnf_guildId = jsmnf_find(pairs, text, "guildId", 7);
      if (__coglink_checkParse(lavaInfo, jsmnf_guildId, "guildId") != COGLINK_PROCEED) return;

      char guildId[GUILD_ID_LENGTH];
      snprintf(guildId, sizeof(guildId), "%.*s", (int)jsmnf_guildId->v.len, text + jsmnf_guildId->v.pos);

      char *path[] = { "state", "time" };
      jsmnf_pair *time = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, time, "time") != COGLINK_PROCEED) return;

      char Time[16];
      snprintf(Time, sizeof(Time), "%.*s", (int)time->v.len, text + time->v.pos);

      path[1] = "position";
      jsmnf_pair *position = jsmnf_find_path(pairs, text, path, 2);

      char Position[16];
      if (position) snprintf(Position, sizeof(Position), "%.*s", (int)position->v.len, text + position->v.pos);
      else snprintf(Position, sizeof(Position), "NULL");

      path[1] = "connected";
      jsmnf_pair *connected = jsmnf_find_path(pairs, text, path, 2);
      if (__coglink_checkParse(lavaInfo, connected, "connected") != COGLINK_PROCEED) return;

      char Connected[TRUE_FALSE_LENGTH];
      snprintf(Connected, sizeof(Connected), "%.*s", (int)connected->v.len, text + connected->v.pos);

      path[1] = "ping";
      jsmnf_pair *ping = jsmnf_find_path(pairs, text, path, 2);
      
      char Ping[8];
      if (ping) snprintf(Ping, sizeof(Ping), "%.*s", (int)ping->v.len, text + ping->v.pos);

      if (lavaInfo->events->onPlayerUpdate) lavaInfo->events->onPlayerUpdate(atoi(Time), atoi(Position), Connected, atoi(Ping), strtoull(guildId, NULL, 10));
      break;
    }
    default: {
      if (lavaInfo->events->onUnknownOp) lavaInfo->events->onUnknownOp(Op, text);
      break;
    }
  }
}

/*
  EVENTS
*/

void coglink_wsLoop(struct lavaInfo *lavaInfo) {
  ws_easy_run(lavaInfo->ws, 5, &lavaInfo->tstamp);
}

void coglink_joinVoiceChannel(const struct lavaInfo *lavaInfo, struct discord *client, u64snowflake voiceChannelId, u64snowflake guildId) {
  char joinVCPayload[512];
  snprintf(joinVCPayload, sizeof(joinVCPayload), "{\"op\":4,\"d\":{\"guild_id\":%"PRIu64",\"channel_id\":\"%"PRIu64"\",\"self_mute\":false,\"self_deaf\":true}}", guildId, voiceChannelId);

  if (!ws_send_text(client->gw.ws, NULL, joinVCPayload, strlen(joinVCPayload))) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadErrorsDebugging) log_fatal("[coglink:libcurl] Something went wrong while sending a payload with op 4 to Discord.");
    return;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadSuccessDebugging) log_debug("[coglink:libcurl] Successfully sent the payload with op 4 to Discord.");
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
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_loader loader;
      jsmnf_pair pairs[128];

      jsmnf_init(&loader);
      r = jsmnf_load(&loader, data, tokens, parser.toknext, pairs, 128);

      if (r < 0) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", 8);
      if (__coglink_checkParse(lavaInfo, VGI, "guild_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char guildId[GUILD_ID_LENGTH];
      snprintf(guildId, sizeof(guildId), "%.*s", (int)VGI->v.len, data + VGI->v.pos);

      jsmnf_pair *VUI = jsmnf_find(pairs, data, "user_id", 7);
      if (__coglink_checkParse(lavaInfo, VUI, "user_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char userId[USER_ID_LENGTH];
      snprintf(userId, USER_ID_LENGTH, "%.*s", (int)VUI->v.len, data + VUI->v.pos);

      if (0 == strcmp(userId, lavaInfo->node.botId)) {
        jsmnf_pair *SSI = jsmnf_find(pairs, data, "session_id", 10);
        if (__coglink_checkParse(lavaInfo, SSI, "session_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

        char *sessionId = malloc(SESSION_ID_LENGTH);
        snprintf(sessionId, SESSION_ID_LENGTH, "%.*s", (int)SSI->v.len, data + SSI->v.pos);

        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging)  log_debug("[coglink:memory] Allocated %d bytes for sessionId to be saved in the hashtable.", SESSION_ID_LENGTH);
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_debug("[coglink:hashtable] Parsed voice state update json, results:\n> guild_id: %s\n> user_id: %s\n> session_id: %s", guildId, userId, sessionId);

        if (0 != strcmp(sessionId, "null")) {
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
      int r = jsmn_parse(&parser, data, size, tokens, sizeof(tokens));

      if (r < 0) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[jsmn-find] Failed to parse JSON.");
        return DISCORD_EVENT_IGNORE;
      } else {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");
      }

      jsmnf_loader loader;
      jsmnf_pair pairs[128];

      jsmnf_init(&loader);
      r = jsmnf_load(&loader, data, tokens, parser.toknext, pairs, 128);

      if (r < 0) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
        return DISCORD_EVENT_IGNORE;
      } else {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
      }

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", 8);
      if (__coglink_checkParse(lavaInfo, VGI, "guild_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char guildId[GUILD_ID_LENGTH];
      snprintf(guildId, sizeof(guildId), "%.*s", (int)VGI->v.len, data + VGI->v.pos);

      if (!hashtable) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->chashErrorsDebugging) log_error("[coglink:jsmn-find] The hashtable is not initialized.");
        return DISCORD_EVENT_IGNORE;
      } else {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->chashSuccessDebugging) log_debug("[coglink:jsmn-find] The hashtable is initialized.");
      }

      int exists = chash_contains(hashtable, guildId, exists, STRING_TABLE);
      if (0 == exists) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->chashErrorsDebugging) log_error("[coglink:jsmn-find] The hashtable does not contain any data related to the guildId.");
        return DISCORD_EVENT_IGNORE;
      } else {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->chashSuccessDebugging) log_debug("[coglink:jsmn-find] The hashtable contains the sessionId related to the guildId.");
      }

      char *sessionID = chash_lookup(hashtable, guildId, sessionID, STRING_TABLE);

      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Successfully found the sessionID in the hashtable.");
  
      char payload[512];
      snprintf(payload, sizeof(payload), "{\"op\":\"voiceUpdate\",\"guildId\":\"%s\",\"sessionId\":\"%s\",\"event\":%.*s}", guildId, sessionID, (int)size, data);

     __coglink_sendPayload(lavaInfo, payload, "voiceUpdate");
    } return DISCORD_EVENT_IGNORE;
    default:
      return DISCORD_EVENT_MAIN_THREAD;
  }
}

void coglink_freeNodeInfo(struct lavaInfo *lavaInfo) {
  if (lavaInfo) lavaInfo = NULL;
}

void coglink_disconnectNode(struct lavaInfo *lavaInfo) {
  ws_close(lavaInfo->ws, 1000, "Requested to be closed", sizeof("Requested to be closed"));
}

void coglink_connectNodeCleanup(struct lavaInfo *lavaInfo) {
  if (hashtable) chash_free(hashtable, STRING_TABLE);
  ws_end(lavaInfo->ws);
  ws_cleanup(lavaInfo->ws);
  curl_multi_cleanup(lavaInfo->mhandle);
  curl_global_cleanup();
  ws_end(lavaInfo->ws);
  ws_cleanup(lavaInfo->ws);
  coglink_freeNodeInfo(lavaInfo);
}

void coglink_setEvents(struct lavaInfo *lavaInfo, struct lavaEvents *lavaEvents) {
  lavaInfo->events = lavaEvents;
}

int coglink_connectNode(struct lavaInfo *lavaInfo, struct discord *client, struct lavaNode *node) {
  struct ws_callbacks callbacks = {
    .on_text = &onTextEvent,
    .on_connect = &onConnectEvent,
    .on_close = &onCloseEvent,
    .data = (void *)lavaInfo
  };

  curl_global_init(CURL_GLOBAL_ALL);

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
  ws_add_header(ws, "Client-Name", "Coglink");

  lavaInfo->mhandle = mhandle;
  lavaInfo->ws = ws;
  uint64_t tstamp = 0;
  lavaInfo->tstamp = tstamp;
  lavaInfo->node = node;

  return COGLINK_SUCCESS;
}