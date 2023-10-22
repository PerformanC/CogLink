#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include <coglink/lavalink.h>
#include <coglink/lavalink-internal.h>
#include <coglink/definitions.h>
#include <coglink/player.h>
#include <coglink/tablec.h>

struct tablec_ht coglink_hashtable;

void _coglink_createPlayer(u64snowflake guildId, int node) {
  char key[32];
  snprintf(key, sizeof(key), "player:%" PRIu64 "", guildId);

  void *data;
  memcpy(&data, &node, sizeof(node));

  tablec_set(&coglink_hashtable, key, data);
}

int _coglink_findPlayerNode(u64snowflake guildId) {
  char key[32];
  snprintf(key, sizeof(key), "player:%" PRIu64 "", guildId);

  struct tablec_buckets value = tablec_get(&coglink_hashtable, key);

  int node;
  memcpy(&node, &value.value, sizeof(node));

  return node;
}

void onCloseEvent(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t length) {
  (void) ws;

  struct coglink_lavaInfo *lavaInfo = data;

  if (lavaInfo->plugins && lavaInfo->plugins->events->onLavalinkClose[0]) {
    if (lavaInfo->plugins->security->allowReadWebsocket) {
      for (int i = 0;i <= lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onLavalinkClose[i]) break;

        if (lavaInfo->plugins->events->onLavalinkClose[i](lavaInfo, info, wscode, reason, length) != COGLINK_PROCEED) return;
      }
    } else {
      struct coglink_lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->nodes = NULL;

      for (int i = 0;i <= lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onLavalinkClose[i]) break;

        if (lavaInfo->plugins->events->onLavalinkClose[i](lavaInfoPlugin, info, wscode, reason, length) != COGLINK_PROCEED) return;
      }
    }
  }

  if (lavaInfo->events->onClose) lavaInfo->events->onClose(wscode, reason);
}

void onTextEvent(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t length) {
  (void) ws; (void) info;
  struct coglink_lavaInfo *lavaInfo = data;

  int node = lavaInfo->nodeId;

  if (lavaInfo->plugins && lavaInfo->plugins->events->onLavalinkPacketReceived[0]) {
    if (lavaInfo->plugins->security->allowReadWebsocket) {
      for (int i = 0;i <= lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onLavalinkPacketReceived[i]) break;

        if (lavaInfo->plugins->events->onLavalinkPacketReceived[i](lavaInfo, text, length) != COGLINK_PROCEED) return;
      }
    } else {
      struct coglink_lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->nodes = NULL;

      for (int i = 0;i <= lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onLavalinkPacketReceived[i]) break;

        if (lavaInfo->plugins->events->onLavalinkPacketReceived[i](lavaInfoPlugin, text, length) != COGLINK_PROCEED) return;
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

  jsmnf_pair *op = jsmnf_find(pairs, text, "op", sizeof("op") - 1);
  if (_coglink_checkParse(lavaInfo, op, "op") != COGLINK_PROCEED) return;

  char Op[16];

  snprintf(Op, sizeof(Op), "%.*s", (int)op->v.len, text + op->v.pos);

  switch(Op[0]) {
    case 'r': { /* ready */
      jsmnf_pair *sessionId = jsmnf_find(pairs, text, "sessionId", sizeof("sessionId") - 1);
      if (_coglink_checkParse(lavaInfo, sessionId, "sessionId") != COGLINK_PROCEED) return;

      snprintf(lavaInfo->nodes[node].sessionId, COGLINK_LAVALINK_SESSIONID_LENGTH, "%.*s", (int)sessionId->v.len, text + sessionId->v.pos);

      if (lavaInfo->events->onConnect) lavaInfo->events->onConnect(lavaInfo->nodes[node].sessionId);
      break;
    }
    case 'e': {
      jsmnf_pair *type = jsmnf_find(pairs, text, "type", sizeof("type") - 1);
      if (_coglink_checkParse(lavaInfo, type, "type") != COGLINK_PROCEED) return;

      jsmnf_pair *jsmnf_guildId = jsmnf_find(pairs, text, "guildId", sizeof("guildId") - 1);
      if (_coglink_checkParse(lavaInfo, jsmnf_guildId, "guildId") != COGLINK_PROCEED) return;

      char Type[32], guildId[COGLINK_GUILD_ID_LENGTH];

      snprintf(Type, sizeof(Type), "%.*s", (int)type->v.len, text + type->v.pos);
      snprintf(guildId, sizeof(guildId), "%.*s", (int)jsmnf_guildId->v.len, text + jsmnf_guildId->v.pos);

      switch(Type[7]) {
        case 'a': { /* TrackStartEvent */
          if (!lavaInfo->events->onTrackStart) return;

          struct coglink_parsedTrack parsedTrack = _coglink_buildTrackStruct(lavaInfo, pairs, text);
          if (parsedTrack.encoded[0] == '\0') return;

          lavaInfo->events->onTrackStart(guildId, &parsedTrack);
          break;
        }
        case 'd': { /* TrackEndEvent */
          if (!lavaInfo->events->onTrackEnd) return;

          jsmnf_pair *reason = jsmnf_find(pairs, text, "reason", sizeof("reason") - 1);
          if (_coglink_checkParse(lavaInfo, reason, "reason") != COGLINK_PROCEED) return;

          struct coglink_parsedTrack parsedTrack = _coglink_buildTrackStruct(lavaInfo, pairs, text);
          if (parsedTrack.encoded[0] == '\0') return;

          char Reason[16];

          snprintf(Reason, sizeof(Reason), "%.*s", (int)reason->v.len, text + reason->v.pos);

          lavaInfo->events->onTrackEnd(guildId, &parsedTrack, Reason);
          break;
        }
        case 'c': { /* TrackExceptionEvent */
          if (!lavaInfo->events->onTrackException) return;

          struct coglink_parsedTrack parsedTrack = _coglink_buildTrackStruct(lavaInfo, pairs, text);
          if (parsedTrack.encoded[0] == '\0') return;

          char *path[] = { "exception", "message" };
          jsmnf_pair *message = jsmnf_find_path(pairs, text, path, 2);
          if (_coglink_checkParse(lavaInfo, message, "message") != COGLINK_PROCEED) return;

          path[1] = "severity";
          jsmnf_pair *severity = jsmnf_find_path(pairs, text, path, 2);
          if (_coglink_checkParse(lavaInfo, severity, "severity") != COGLINK_PROCEED) return;

          path[1] = "cause";
          jsmnf_pair *cause = jsmnf_find_path(pairs, text, path, 2);
          if (_coglink_checkParse(lavaInfo, cause, "cause") != COGLINK_PROCEED) return;

          char Message[128], Severity[16], Cause[256];

          snprintf(Message, sizeof(Message), "%.*s", (int)message->v.len, text + message->v.pos);
          snprintf(Severity, sizeof(Severity), "%.*s", (int)severity->v.len, text + severity->v.pos);
          snprintf(Cause, sizeof(Cause), "%.*s", (int)cause->v.len, text + cause->v.pos);

          lavaInfo->events->onTrackException(guildId, &parsedTrack, Message, Severity, Cause);
          break;
        }
        case 'u': { /* TrackStuckEvent */
          if (!lavaInfo->events->onTrackStuck) return;

          struct coglink_parsedTrack parsedTrack = _coglink_buildTrackStruct(lavaInfo, pairs, text);
          if (parsedTrack.encoded[0] == '\0') return;

          jsmnf_pair *thresholdMs = jsmnf_find(pairs, text, "thresholdMs", sizeof("thresholdMs") - 1);
          if (_coglink_checkParse(lavaInfo, thresholdMs, "thresholdMs") != COGLINK_PROCEED) return;

          char ThresholdMs[16];

          snprintf(ThresholdMs, sizeof(ThresholdMs), "%.*s", (int)thresholdMs->v.len, text + thresholdMs->v.pos);

          lavaInfo->events->onTrackStuck(guildId, ThresholdMs, &parsedTrack);
          break;
        }
        case 't': { /* WebSocketClosedEvent */
          if (!lavaInfo->events->onWebSocketClosed) return;

          jsmnf_pair *code = jsmnf_find(pairs, text, "code", sizeof("code") - 1);
          if (_coglink_checkParse(lavaInfo, code, "code") != COGLINK_PROCEED) return;

          jsmnf_pair *reason = jsmnf_find(pairs, text, "reason", sizeof("reason") - 1);
          if (_coglink_checkParse(lavaInfo, reason, "reason") != COGLINK_PROCEED) return;

          jsmnf_pair *byRemote = jsmnf_find(pairs, text, "byRemote", sizeof("byRemote") - 1);
          if (_coglink_checkParse(lavaInfo, byRemote, "byRemote") != COGLINK_PROCEED) return;

          char Code[16], Reason[128], ByRemote[COGLINK_TRUE_FALSE_LENGTH];

          snprintf(Code, sizeof(Code), "%.*s", (int)code->v.len, text + code->v.pos);
          snprintf(Reason, sizeof(Reason), "%.*s", (int)reason->v.len, text + reason->v.pos);
          snprintf(ByRemote, sizeof(ByRemote), "%.*s", (int)byRemote->v.len, text + byRemote->v.pos);

          lavaInfo->events->onWebSocketClosed(guildId, Code, Reason, (ByRemote[0] == 't' ? 1 : 0));
          break;
        }
        default: {
          if (lavaInfo->events->onUnknownEvent) lavaInfo->events->onUnknownEvent(guildId, Type, text);
          break;
        }
      }
      break;
    }
    case 's': { /* Stats */
      if (!lavaInfo->events->onStats) return;

      jsmnf_pair *players = jsmnf_find(pairs, text, "players", sizeof("players") - 1);
      if (_coglink_checkParse(lavaInfo, players, "players") != COGLINK_PROCEED) return;

      jsmnf_pair *playingPlayers = jsmnf_find(pairs, text, "playingPlayers", sizeof("playingPlayers") - 1);
      if (_coglink_checkParse(lavaInfo, playingPlayers, "playingPlayers") != COGLINK_PROCEED) return;

      jsmnf_pair *uptime = jsmnf_find(pairs, text, "uptime", sizeof("uptime") - 1);
      if (_coglink_checkParse(lavaInfo, uptime, "uptime") != COGLINK_PROCEED) return;

      char *path[] = { "memory", "free" };
      jsmnf_pair *lavaFree = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, lavaFree, "lavaFree") != COGLINK_PROCEED) return;

      path[1] = "used";
      jsmnf_pair *used = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, used, "used") != COGLINK_PROCEED) return;

      path[1] = "allocated";
      jsmnf_pair *allocated = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, allocated, "allocated") != COGLINK_PROCEED) return;

      path[1] = "reservable";
      jsmnf_pair *reservable = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, reservable, "reservable") != COGLINK_PROCEED) return;

      path[0] = "cpu";
      path[1] = "cores";
      jsmnf_pair *cores = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, cores, "cores") != COGLINK_PROCEED) return;

      path[1] = "systemLoad";
      jsmnf_pair *systemLoad = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, systemLoad, "systemLoad") != COGLINK_PROCEED) return;

      path[1] = "lavalinkLoad";
      jsmnf_pair *lavalinkLoad = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, lavalinkLoad, "lavalinkLoad") != COGLINK_PROCEED) return;

      struct coglink_lavalinkStats *lavalinkStatsStruct = &(struct coglink_lavalinkStats) {
        .players = "",
        .playingPlayers = "",
        .uptime = "",
        .memory = &(struct coglink_lavalinkStatsMemory) {
          .free = "",
          .used = "",
          .allocated = "",
          .reservable = ""
        },
        .cpu = &(struct coglink_lavalinkStatsCPU) {
          .cores = "",
          .systemLoad = "",
          .lavalinkLoad = ""
        },
        .frameStats = &(struct coglink_lavalinkStatsFrameStats) {
          .sent = "",
          .deficit = "",
          .nulled = ""
        }
      };

      path[0] = "frameStats";
      path[1] = "sent";
      jsmnf_pair *sent = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, sent, "sent") == COGLINK_PROCEED) {
        snprintf(lavalinkStatsStruct->frameStats->sent, sizeof(lavalinkStatsStruct->frameStats->sent), "%.*s", (int)sent->v.len, text + sent->v.pos);

        path[1] = "deficit";
        jsmnf_pair *deficit = jsmnf_find_path(pairs, text, path, 2);
        if (_coglink_checkParse(lavaInfo, deficit, "deficit") != COGLINK_PROCEED) return;

        snprintf(lavalinkStatsStruct->frameStats->deficit, sizeof(lavalinkStatsStruct->frameStats->deficit), "%.*s", (int)deficit->v.len, text + deficit->v.pos);

        path[1] = "nulled";
        jsmnf_pair *nulled = jsmnf_find_path(pairs, text, path, 2);
        if (_coglink_checkParse(lavaInfo, nulled, "nulled") != COGLINK_PROCEED) return;

        snprintf(lavalinkStatsStruct->frameStats->nulled, sizeof(lavalinkStatsStruct->frameStats->nulled), "%.*s", (int)nulled->v.len, text + nulled->v.pos);
      }

      snprintf(lavalinkStatsStruct->players, sizeof(lavalinkStatsStruct->players), "%.*s", (int)players->v.len, text + players->v.pos);
      snprintf(lavalinkStatsStruct->playingPlayers, sizeof(lavalinkStatsStruct->playingPlayers), "%.*s", (int)playingPlayers->v.len, text + playingPlayers->v.pos);
      snprintf(lavalinkStatsStruct->uptime, sizeof(lavalinkStatsStruct->uptime), "%.*s", (int)uptime->v.len, text + uptime->v.pos);
      snprintf(lavalinkStatsStruct->memory->free, sizeof(lavalinkStatsStruct->memory->free), "%.*s", (int)lavaFree->v.len, text + lavaFree->v.pos);
      snprintf(lavalinkStatsStruct->memory->used, sizeof(lavalinkStatsStruct->memory->used), "%.*s", (int)used->v.len, text + used->v.pos);
      snprintf(lavalinkStatsStruct->memory->allocated, sizeof(lavalinkStatsStruct->memory->allocated), "%.*s", (int)allocated->v.len, text + allocated->v.pos);
      snprintf(lavalinkStatsStruct->memory->reservable, sizeof(lavalinkStatsStruct->memory->reservable), "%.*s", (int)reservable->v.len, text + reservable->v.pos);
      snprintf(lavalinkStatsStruct->cpu->cores, sizeof(lavalinkStatsStruct->cpu->cores), "%.*s", (int)cores->v.len, text + cores->v.pos);
      snprintf(lavalinkStatsStruct->cpu->systemLoad, sizeof(lavalinkStatsStruct->cpu->systemLoad), "%.*s", (int)systemLoad->v.len, text + systemLoad->v.pos);
      snprintf(lavalinkStatsStruct->cpu->lavalinkLoad, sizeof(lavalinkStatsStruct->cpu->lavalinkLoad), "%.*s", (int)lavalinkLoad->v.len, text + lavalinkLoad->v.pos);

      struct _coglink_nodeStats nodeStats = {
        .cores = atoi(lavalinkStatsStruct->cpu->cores),
        .systemLoad = atof(lavalinkStatsStruct->cpu->systemLoad)
      };

      lavaInfo->nodes[node].stats = nodeStats;

      if (sent && (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging)) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> Players: %s\n> PlayingPlayers: %s\n> Uptime: %s\n> Free: %s\n> Used: %s\n> Allocated: %s\n> Reservable: %s\n> Cores: %s\n> SystemLoad: %s\n> LavalinkLoad: %s\n> Sent: %s\n> Nulled: %s\n> Deficit: %s", lavalinkStatsStruct->players, lavalinkStatsStruct->playingPlayers, lavalinkStatsStruct->uptime, lavalinkStatsStruct->memory->free, lavalinkStatsStruct->memory->used, lavalinkStatsStruct->memory->allocated, lavalinkStatsStruct->memory->reservable, lavalinkStatsStruct->cpu->cores, lavalinkStatsStruct->cpu->systemLoad, lavalinkStatsStruct->cpu->lavalinkLoad, lavalinkStatsStruct->frameStats->sent, lavalinkStatsStruct->frameStats->nulled, lavalinkStatsStruct->frameStats->deficit);
      else if (!sent && (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging)) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> Players: %s\n> PlayingPlayers: %s\n> Uptime: %s\n> Free: %s\n> Used: %s\n> Allocated: %s\n> Reservable: %s\n> Cores: %s\n> SystemLoad: %s\n> LavalinkLoad: %s", lavalinkStatsStruct->players, lavalinkStatsStruct->playingPlayers, lavalinkStatsStruct->uptime, lavalinkStatsStruct->memory->free, lavalinkStatsStruct->memory->used, lavalinkStatsStruct->memory->allocated, lavalinkStatsStruct->memory->reservable, lavalinkStatsStruct->cpu->cores, lavalinkStatsStruct->cpu->systemLoad, lavalinkStatsStruct->cpu->lavalinkLoad);

      lavaInfo->events->onStats(lavalinkStatsStruct);
      break;
    }
    case 'p': { /* PlayerUpdate */
      if (!lavaInfo->events->onPlayerUpdate) return;

      jsmnf_pair *jsmnf_guildId = jsmnf_find(pairs, text, "guildId", sizeof("guildId") - 1);
      if (_coglink_checkParse(lavaInfo, jsmnf_guildId, "guildId") != COGLINK_PROCEED) return;

      char *path[] = { "state", "time" };
      jsmnf_pair *time = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, time, "time") != COGLINK_PROCEED) return;

      path[1] = "position";
      jsmnf_pair *position = jsmnf_find_path(pairs, text, path, 2);

      path[1] = "connected";
      jsmnf_pair *connected = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, connected, "connected") != COGLINK_PROCEED) return;

      path[1] = "ping";
      jsmnf_pair *ping = jsmnf_find_path(pairs, text, path, 2);
      if (_coglink_checkParse(lavaInfo, ping, "ping") != COGLINK_PROCEED) return;

      char guildId[COGLINK_GUILD_ID_LENGTH], Time[16], Position[16], Connected[COGLINK_TRUE_FALSE_LENGTH], Ping[8];

      snprintf(guildId, sizeof(guildId), "%.*s", (int)jsmnf_guildId->v.len, text + jsmnf_guildId->v.pos);
      snprintf(Time, sizeof(Time), "%.*s", (int)time->v.len, text + time->v.pos);
      snprintf(Position, sizeof(Position), "%.*s", (int)position->v.len, text + position->v.pos);
      snprintf(Ping, sizeof(Ping), "%.*s", (int)ping->v.len, text + ping->v.pos);
      snprintf(Connected, sizeof(Connected), "%.*s", (int)connected->v.len, text + connected->v.pos);

      lavaInfo->events->onPlayerUpdate(guildId, Time, Position, (Connected[0] == 't' ? 1 : 0), Ping);
      break;
    }
    default: {
      if (lavaInfo->events->onUnknownOp) lavaInfo->events->onUnknownOp(Op, text);
      break;
    }
  }
}

enum discord_event_scheduler __coglink_handleScheduler(struct discord *client, const char data[], size_t length, enum discord_gateway_events event) {
  struct coglink_lavaInfo *lavaInfo = discord_get_data(client);

  if (lavaInfo->plugins && lavaInfo->plugins->events->onCoglinkScheduler[0]) {
    struct discord *clientPlugin = client;
    if (lavaInfo->plugins->security->allowReadBotToken) clientPlugin->token = NULL;
    if (lavaInfo->plugins->security->allowReadConcordWebsocket) clientPlugin->gw = (struct discord_gateway) { 0 };

    if (lavaInfo->plugins->security->allowReadWebsocket) {
      for (int i = 0;i <= lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onCoglinkScheduler[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onCoglinkScheduler[i](lavaInfo, clientPlugin, data, length, event);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct coglink_lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->nodes = NULL;

      clientPlugin->io_poller = NULL;

      for (int i = 0;i <= lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onCoglinkScheduler[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onCoglinkScheduler[i](lavaInfoPlugin, clientPlugin, data, length, event);
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

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", sizeof("guild_id") - 1);
      if (_coglink_checkParse(lavaInfo, VGI, "guild_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      jsmnf_pair *VUI = jsmnf_find(pairs, data, "user_id", sizeof("user_id") - 1);
      if (_coglink_checkParse(lavaInfo, VUI, "user_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char *userId = malloc(COGLINK_USER_ID_LENGTH);
      char *guildId = malloc(COGLINK_GUILD_ID_LENGTH);

      snprintf(userId, COGLINK_USER_ID_LENGTH, "%.*s", (int)VUI->v.len, data + VUI->v.pos);
      snprintf(guildId, COGLINK_GUILD_ID_LENGTH, "%.*s", (int)VGI->v.len, data + VGI->v.pos);

      if (0 == strcmp(userId, lavaInfo->botId)) {
        jsmnf_pair *SSI = jsmnf_find(pairs, data, "session_id", sizeof("session_id") - 1);
        if (_coglink_checkParse(lavaInfo, SSI, "session_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

        char *sessionId = malloc(COGLINK_SESSION_ID_LENGTH);
        snprintf(sessionId, COGLINK_SESSION_ID_LENGTH, "%.*s", (int)SSI->v.len, data + SSI->v.pos);

        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging)  log_debug("[coglink:memory] Allocated %d bytes for sessionId to be saved in the hashtable.", sizeof(sessionId));
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_debug("[coglink:tablec] Parsed voice state update json, results:\n> guild_id: %s\n> user_id: %s\n> session_id: %s", guildId, userId, sessionId);

        if (sessionId[0] != 'n') {
          tablec_set(&coglink_hashtable, guildId, sessionId);

          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->tablecSuccessDebugging) log_debug("[coglink:tablec] The user that got updated is the bot, saving the sessionId.");
        } else {
          tablec_del(&coglink_hashtable, guildId);

          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->tablecSuccessDebugging) log_debug("[coglink:tablec] The user that got updated is the bot, but the sessionId is NULL, removing the sessionId from the hashtable.");
        }
      } else if (lavaInfo->allowCachingVoiceChannelIds) {
        jsmnf_pair *VCI = jsmnf_find(pairs, data, "channel_id", sizeof("channel_id") - 1);
        if (_coglink_checkParse(lavaInfo, VCI, "channel_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

        char *channelId = malloc(COGLINK_VOICE_ID_LENGTH);
        snprintf(channelId, COGLINK_VOICE_ID_LENGTH, "%.*s", (int)VCI->v.len, data + VCI->v.pos);

        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging)  log_debug("[coglink:memory] Allocated %d bytes for voiceId to be saved in the hashtable.", sizeof(channelId));
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_debug("[coglink:tablec] Parsed voice state update json, results:\n> guild_id: %s\n> user_id: %s\n> channel_id: %s", guildId, userId, channelId);

        if (channelId[0] != 'n') {
          tablec_set(&coglink_hashtable, userId, channelId);

          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->tablecSuccessDebugging) log_debug("[coglink:tablec] Optional save members channel ID is enabled, saving the channelId.");
        } else {
          tablec_del(&coglink_hashtable, userId);

          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->tablecSuccessDebugging) log_debug("[coglink:tablec] Optional save members channel ID is enabled, but the channelId is NULL, removing the channelId from the hashtable.");
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

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", sizeof("guild_id") - 1);
      if (_coglink_checkParse(lavaInfo, VGI, "guild_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char guildId[COGLINK_GUILD_ID_LENGTH];
      snprintf(guildId, sizeof(guildId), "%.*s", (int)VGI->v.len, data + VGI->v.pos);

      struct tablec_buckets sessionId = tablec_get(&coglink_hashtable, guildId);
      if (!sessionId.value) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->tablecErrorsDebugging) log_error("[coglink:TableC] The hashtable does not contain any data related to the guildId.");
        return DISCORD_EVENT_IGNORE;
      }

      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceServerDebugging || lavaInfo->debugging->tablecSuccessDebugging) log_debug("[coglink:TableC] Successfully found the sessionID in the hashtable.");
  
      jsmnf_pair *token = jsmnf_find(pairs, data, "token", sizeof("token") - 1);
      if (_coglink_checkParse(lavaInfo, token, "token") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      jsmnf_pair *endpoint = jsmnf_find(pairs, data, "endpoint", sizeof("endpoint") - 1);
      if (_coglink_checkParse(lavaInfo, endpoint, "endpoint") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char key[32];
      snprintf(key, sizeof(key), "player:%s", guildId);

      struct tablec_buckets value = tablec_get(&coglink_hashtable, key);

      int node;
      memcpy(&node, &value.value, sizeof(node));

      char reqPath[128];
      int pathLen = snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%s", lavaInfo->nodes[node].sessionId, guildId);

      char payload[256];
      int payloadLen = snprintf(payload, sizeof(payload), "{\"voice\":{\"token\":\"%.*s\",\"endpoint\":\"%.*s\",\"sessionId\":\"%s\"}}", (int)token->v.len, data + token->v.pos, (int)endpoint->v.len, data + endpoint->v.pos, (char *)sessionId.value);

      _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL, 
                              &(struct __coglink_requestConfig) {
                                .requestType = __COGLINK_PATCH_REQ,
                                .additionalDebuggingSuccess = lavaInfo->debugging->handleSchedulerVoiceServerDebugging,
                                .additionalDebuggingError = lavaInfo->debugging->handleSchedulerVoiceServerDebugging,
                                .path = reqPath,
                                .pathLength = pathLen,
                                .body = payload,
                                .bodySize = payloadLen
                              });

      free(sessionId.key);
      free(sessionId.value);
    } return DISCORD_EVENT_IGNORE;
    case DISCORD_EV_GUILD_CREATE: {
      jsmn_parser parser;
      jsmntok_t tokens[5096];

      jsmn_init(&parser);
      int r = jsmn_parse(&parser, data, length, tokens, sizeof(tokens));

      if (r < 0) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_loader loader;
      jsmnf_pair pairs[5096];

      jsmnf_init(&loader);
      r = jsmnf_load(&loader, data, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

      if (r < 0) {
        if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_pair *voice_states = jsmnf_find(pairs, data, "voice_states", sizeof("voice_states") - 1);

      if (!voice_states) return DISCORD_EVENT_IGNORE;

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "id", sizeof("id") - 1);
      if (_coglink_checkParse(lavaInfo, VGI, "id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

      char *guildId = malloc(COGLINK_GUILD_ID_LENGTH);
      snprintf(guildId, COGLINK_GUILD_ID_LENGTH, "%.*s", (int)VGI->v.len, data + VGI->v.pos);

      jsmnf_pair *GVS = jsmnf_find(pairs, data, "voice_states", sizeof("voice_states") - 1);

      int i = GVS->size;
      while (i > 0) {
        char iStr[16];
        snprintf(iStr, sizeof(iStr), "%d", i - 1);

        char *path[] = { "voice_states", iStr, "user_id" };

        jsmnf_pair *VUI = jsmnf_find_path(pairs, data, path, 3);
        if (_coglink_checkParse(lavaInfo, VUI, "user_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

        char *userId = malloc(COGLINK_USER_ID_LENGTH);

        snprintf(userId, COGLINK_USER_ID_LENGTH, "%.*s", (int)VUI->v.len, data + VUI->v.pos);

        if (0 == strcmp(userId, lavaInfo->botId)) {
          path[2] = "session_id";
          jsmnf_pair *SSI = jsmnf_find_path(pairs, data, path, 3);
          if (_coglink_checkParse(lavaInfo, SSI, "session_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

          char *sessionId = malloc(COGLINK_SESSION_ID_LENGTH);
          snprintf(sessionId, COGLINK_SESSION_ID_LENGTH, "%.*s", (int)SSI->v.len, data + SSI->v.pos);

          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging)  log_debug("[coglink:memory] Allocated %d bytes for sessionId to be saved in the hashtable.", sizeof(sessionId));
          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_debug("[coglink:tablec] Parsed voice state update json, results:\n> guild_id: %s\n> user_id: %s\n> session_id: %s", guildId, userId, sessionId);

          if (sessionId[0] != 'n') {
            tablec_set(&coglink_hashtable, guildId, sessionId);

            if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->tablecSuccessDebugging) log_debug("[coglink:tablec] The user that got updated is the bot, saving the sessionId.");
          } else {
            tablec_del(&coglink_hashtable, guildId);

            if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->tablecSuccessDebugging) log_debug("[coglink:tablec] The user that got updated is the bot, but the sessionId is NULL, removing the sessionId from the hashtable.");
          }
        } else if (lavaInfo->allowCachingVoiceChannelIds) {
          path[2] = "channel_id";
          jsmnf_pair *VCI = jsmnf_find_path(pairs, data, path, 3);
          if (_coglink_checkParse(lavaInfo, VCI, "channel_id") != COGLINK_PROCEED) return DISCORD_EVENT_IGNORE;

          char *channelId = malloc(COGLINK_VOICE_ID_LENGTH);
          snprintf(channelId, COGLINK_VOICE_ID_LENGTH, "%.*s", (int)VCI->v.len, data + VCI->v.pos);

          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging)  log_debug("[coglink:memory] Allocated %d bytes for voiceId to be saved in the hashtable.", sizeof(channelId));
          if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging) log_debug("[coglink:tablec] Parsed voice state update json, results:\n> guild_id: %s\n> user_id: %s\n> channel_id: %s", guildId, userId, channelId);

          if (channelId[0] != 'n') {
            tablec_set(&coglink_hashtable, userId, channelId);

            if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->tablecSuccessDebugging) log_debug("[coglink:tablec] Optional save members channel ID is enabled, saving the channelId.");
          } else {
            tablec_del(&coglink_hashtable, userId);

            if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->handleSchedulerVoiceStateDebugging || lavaInfo->debugging->tablecSuccessDebugging) log_debug("[coglink:tablec] Optional save members channel ID is enabled, but the channelId is NULL, removing the channelId from the hashtable.");
          }
        }

        i--;
      }
    } return DISCORD_EVENT_IGNORE;
    default:
      return DISCORD_EVENT_MAIN_THREAD;
  }
}

int coglink_joinVoiceChannel(struct coglink_lavaInfo *lavaInfo, struct discord *client, u64snowflake voiceChannelId, u64snowflake guildId) {
  char joinVCPayload[512];
  int payloadLen = snprintf(joinVCPayload, sizeof(joinVCPayload), "{\"op\":4,\"d\":{\"guild_id\":%"PRIu64",\"channel_id\":\"%"PRIu64"\",\"self_mute\":false,\"self_deaf\":true}}", guildId, voiceChannelId);

  if (!ws_send_text(client->gw.ws, NULL, joinVCPayload, payloadLen)) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadErrorsDebugging) log_fatal("[coglink:libcurl] Something went wrong while sending a payload with op 4 to Discord.");
    return COGLINK_ERROR;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadSuccessDebugging) log_debug("[coglink:libcurl] Successfully sent the payload with op 4 to Discord.");

  return COGLINK_SUCCESS;
}

int coglink_joinUserVoiceChannel(struct coglink_lavaInfo *lavaInfo, struct discord *client, u64snowflake userId, u64snowflake guildId) {
  char userIdStr[COGLINK_USER_ID_LENGTH];
  snprintf(userIdStr, sizeof(userIdStr), "%"PRIu64"", userId);

  struct tablec_buckets voiceId = tablec_get(&coglink_hashtable, userIdStr);

  if (!voiceId.value) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->joinUserVoiceChannelDebugging || lavaInfo->debugging->tablecErrorsDebugging) log_error("[coglink:tablec] The hashtable does not contain any data related to the userId.");
    return COGLINK_TABLEC_NOT_FOUND;
  }

  char joinVCPayload[512];
  int payloadLen = snprintf(joinVCPayload, sizeof(joinVCPayload), "{\"op\":4,\"d\":{\"guild_id\":%"PRIu64",\"channel_id\":\"%s\",\"self_mute\":false,\"self_deaf\":true}}", guildId, (char *)voiceId.value);

  if (!ws_send_text(client->gw.ws, NULL, joinVCPayload, payloadLen)) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadErrorsDebugging) log_fatal("[coglink:libcurl] Something went wrong while sending a payload with op 4 to Discord.");
    return COGLINK_ERROR;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadSuccessDebugging) log_debug("[coglink:libcurl] Successfully sent the payload with op 4 to Discord.");
  
  free(voiceId.key);
  free(voiceId.value);

  return COGLINK_SUCCESS;
}

void coglink_freeNodeInfo(struct coglink_lavaInfo *lavaInfo) {
  if (lavaInfo) lavaInfo = NULL;
}

void coglink_setEvents(struct coglink_lavaInfo *lavaInfo, struct coglink_lavalinkEvents *lavalinkEvents) {
  lavaInfo->events = lavalinkEvents;
}

void coglink_disconnectNode(struct coglink_lavaInfo *lavaInfo, int nodePos) {
  ws_close(lavaInfo->nodes[nodePos].ws, 1000, "Requested to be closed", 23);
  ws_cleanup(lavaInfo->nodes[nodePos].ws);
  curl_multi_cleanup(lavaInfo->nodes[nodePos].mhandle);
  free(&lavaInfo->nodes[nodePos]);
}

void coglink_connectNodeCleanup(struct coglink_lavaInfo *lavaInfo, struct discord *client) {
  tablec_cleanup(&coglink_hashtable);

  int i = -1;
  while (i++ <= lavaInfo->nodeCount) {
    ws_close(lavaInfo->nodes[i].ws, 1000, "Normal close", 13);
    io_poller_curlm_del(client->io_poller, lavaInfo->nodes[i].mhandle);
    ws_cleanup(lavaInfo->nodes[i].ws);
    curl_multi_cleanup(lavaInfo->nodes[i].mhandle);
    free(&lavaInfo->nodes[i]);
  }

  curl_global_cleanup();
  coglink_freeNodeInfo(lavaInfo);
}

int coglink_connectNode(struct coglink_lavaInfo *lavaInfo, struct discord *client, struct coglink_lavalinkNodes *nodesArr, struct coglink_nodeInfo nodesBuf[]) {
  tablec_init(&coglink_hashtable, 128);

  discord_set_data(client, lavaInfo);
  discord_set_event_scheduler(client, __coglink_handleScheduler);

  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] Something went wrong while initializing libcurl (global).");
    return COGLINK_ERROR;
  }

  int i = -1;
  while (i++ <= nodesArr->size - 2) {
    char hostname[128 + 21];

    if (nodesArr->nodes[i].ssl) snprintf(hostname, sizeof(hostname), "wss://%s/v4/websocket", nodesArr->nodes[i].hostname);
    else snprintf(hostname, sizeof(hostname), "ws://%s/v4/websocket", nodesArr->nodes[i].hostname);

    struct coglink_nodeInfo *nodeInfo = malloc(sizeof(struct coglink_nodeInfo));
    nodeInfo->node = nodesArr->nodes[i];
    nodeInfo->mhandle = curl_multi_init();
    nodeInfo->tstamp = (uint64_t)0;

    lavaInfo->nodeId = i;

    struct ws_callbacks callbacks = {
      .on_text = onTextEvent,
      .on_close = onCloseEvent,
      .data = lavaInfo
    };

    nodeInfo->ws = ws_init(&callbacks, client->gw.mhandle, NULL);

    ws_set_url(nodeInfo->ws, hostname, NULL);
    ws_start(nodeInfo->ws);

    if (lavaInfo->allowResuming && lavaInfo->nodes != NULL) ws_add_header(nodeInfo->ws, "Session-Id", lavaInfo->nodes[i].sessionId);
    ws_add_header(nodeInfo->ws, "Authorization", nodesArr->nodes[i].password);
    ws_add_header(nodeInfo->ws, "Num-Shards", lavaInfo->shards);
    ws_add_header(nodeInfo->ws, "User-Id", lavaInfo->botId);
    ws_add_header(nodeInfo->ws, "Client-Name", "Coglink");
    ws_add_header(nodeInfo->ws, "Sec-WebSocket-Protocol", "13");

    io_poller_curlm_add(client->io_poller, nodeInfo->mhandle, _coglink_IOPoller, nodeInfo);
    io_poller_curlm_enable_perform(client->io_poller, nodeInfo->mhandle);

    nodesBuf[i] = *nodeInfo;
  }

  lavaInfo->nodes = nodesBuf;
  lavaInfo->nodeCount = nodesArr->size - 2;

  return COGLINK_SUCCESS;
}