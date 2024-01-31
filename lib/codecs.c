/* TODO: Use cog-chef (https://github.com/Cogmasters/cog-chef)*/

#include <stdlib.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include "utils.h"

#include "codecs.h"

#define _coglink_parse_track(pairs, json)                                                                                       \
  struct coglink_track *track_info = malloc(sizeof(struct coglink_partial_track));                                                   \
  track_info->info = malloc(sizeof(struct coglink_partial_track));                                                                   \
                                                                                                                                \
  char *path[] = { "encoded", NULL };                                                                                           \
  FIND_FIELD_PATH(encoded, "encoded", 1);                                                                                       \
  snprintf(track_info->encoded, sizeof(track_info->encoded), "%.*s", (int)encoded->v.len, json + encoded->v.pos);                         \
                                                                                                                                \
  path[0] = "info";                                                                                                             \
  path[1] = "identifier";                                                                                                       \
  FIND_FIELD_PATH(identifier, "identifier", 2);                                                                                 \
  snprintf(track_info->info->identifier, sizeof(track_info->info->identifier), "%.*s", (int)identifier->v.len, json + identifier->v.pos); \
                                                                                                                                \
  path[1] = "isSeekable";                                                                                                       \
  FIND_FIELD_PATH(isSeekable, "isSeekable", 2);                                                                                 \
  if (json[isSeekable->v.pos] == 't') track_info->info->isSeekable = true;                                                           \
  else track_info->info->isSeekable = false;                                                                                         \
                                                                                                                                \
  path[1] = "author";                                                                                                           \
  FIND_FIELD_PATH(author, "author", 2);                                                                                         \
  snprintf(track_info->info->author, sizeof(track_info->info->author), "%.*s", (int)author->v.len, json + author->v.pos);                 \
                                                                                                                                \
  path[1] = "length";                                                                                                           \
  FIND_FIELD_PATH(length, "length", 2);                                                                                         \
  PAIR_TO_SIZET(length, lengthStr, track_info->info->length, 16);                                                                    \
                                                                                                                                \
  path[1] = "isStream";                                                                                                         \
  FIND_FIELD_PATH(isStream, "isStream", 2);                                                                                     \
  if (json[isStream->v.pos] == 't') track_info->info->isStream = true;                                                               \
  else track_info->info->isStream = false;                                                                                           \
                                                                                                                                \
  path[1] = "position";                                                                                                         \
  FIND_FIELD_PATH(position, "position", 2);                                                                                     \
  PAIR_TO_SIZET(position, positionStr, track_info->info->position, 16);                                                              \
                                                                                                                                \
  path[1] = "title";                                                                                                            \
  FIND_FIELD_PATH(title, "title", 2);                                                                                           \
  snprintf(track_info->info->title, sizeof(track_info->info->title), "%.*s", (int)title->v.len, json + title->v.pos);                     \
                                                                                                                                \
  path[1] = "uri";                                                                                                              \
  FIND_FIELD_PATH(uri, "uri", 2);                                                                                               \
  snprintf(track_info->info->uri, sizeof(track_info->info->uri), "%.*s", (int)uri->v.len, json + uri->v.pos);                             \
                                                                                                                                \
  path[1] = "isrc";                                                                                                             \
  FIND_FIELD_PATH(isrc, "isrc", 2);                                                                                             \
  snprintf(track_info->info->isrc, sizeof(track_info->info->isrc), "%.*s", (int)isrc->v.len, json + isrc->v.pos);                         \
                                                                                                                                \
  path[1] = "artworkUrl";                                                                                                       \
  FIND_FIELD_PATH(artworkUrl, "artworkUrl", 2);                                                                                 \
  snprintf(track_info->info->artworkUrl, sizeof(track_info->info->artworkUrl), "%.*s", (int)artworkUrl->v.len, json + artworkUrl->v.pos); \
                                                                                                                                \
  path[1] = "sourceName";                                                                                                       \
  FIND_FIELD_PATH(sourceName, "sourceName", 2);                                                                                 \
  snprintf(track_info->info->sourceName, sizeof(track_info->info->sourceName), "%.*s", (int)sourceName->v.len, json + sourceName->v.pos);
  
void *coglink_parse_websocket_data(int *event_type, const char *json, size_t length) {
  jsmn_parser parser;
  jsmntok_t tokens[64];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    *event_type = COGLINK_PARSE_ERROR;

    return NULL;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[64];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    *event_type = COGLINK_PARSE_ERROR;

    return NULL;
  }

  jsmnf_pair *op = jsmnf_find(pairs, json, "op", sizeof("op") - 1);
  if (!op) {
    *event_type = COGLINK_PARSE_ERROR;

    return NULL;
  }

  const char *Op = json + op->v.pos;

  switch(Op[0]) {
    case 'r': { /* ready */
      jsmnf_pair *sessionId = jsmnf_find(pairs, json, "sessionId", sizeof("sessionId") - 1);
      if (!sessionId) {
        *event_type = COGLINK_PARSE_ERROR;

        return NULL;
      }

      jsmnf_pair *resumed = jsmnf_find(pairs, json, "resumed", sizeof("resumed") - 1);

      struct coglink_ready_payload *ready = malloc(sizeof(struct coglink_ready_payload));

      ready->session_id = malloc(sessionId->v.len + 1);
      snprintf(ready->session_id, sessionId->v.len + 1, "%.*s", (int)sessionId->v.len, json + sessionId->v.pos);

      if (json[resumed->v.pos] == 't') ready->resumed = true;
      else ready->resumed = false;

      *event_type = COGLINK_READY;

      return ready;
    }
    case 'e': {
      FIND_FIELD(type, "type");
      FIND_FIELD(guildId, "guildId");

      switch(json[type->v.pos + 7]) {
        case 'a': { /* TrackStartEvent */
          struct coglink_track_start_payload *parsedTrack = malloc(sizeof(struct coglink_track_start_payload));

          PAIR_TO_SIZET(guildId, guildIdStr, parsedTrack->guildId, 18);

          _coglink_parse_track(pairs, json); /* Defines track_info */

          parsedTrack->track = track_info;

          *event_type = COGLINK_TRACK_START;

          return parsedTrack;
        }
        case 'd': { /* TrackEndEvent */
          FIND_FIELD(reason, "reason");

          struct coglink_track_end_payload *parsedTrack = malloc(sizeof(struct coglink_track_end_payload));

          PAIR_TO_SIZET(guildId, guildIdStr, parsedTrack->guildId, 18);

          _coglink_parse_track(pairs, json); /* Defines track_info */

          parsedTrack->track = track_info;

          snprintf(parsedTrack->reason, sizeof(parsedTrack->reason), "%.*s", (int)reason->v.len, json + reason->v.pos);

          *event_type = COGLINK_TRACK_END;

          return parsedTrack;
        }
        // case 'c': { /* TrackExceptionEvent */
        //   struct coglink_parsedTrack parsedTrack = _coglink_buildTrackStruct(c_info, pairs, json);
        //   if (parsedTrack.encoded[0] == '\0') return;

        //   char *path[] = { "exception", "message" };
        //   FIND_FIELD_PATH("message", sizeof("message") - 1, 2);

        //   path[1] = "severity";
        //   FIND_FIELD_PATH("severity", sizeof("severity") - 1, 2);

        //   path[1] = "cause";
        //   FIND_FIELD_PATH("cause", sizeof("cause") - 1, 2);

        //   char Message[128], Severity[16], Cause[256];
        //   snprintf(Message, sizeof(Message), "%.*s", (int)message->v.len, json + message->v.pos);
        //   snprintf(Severity, sizeof(Severity), "%.*s", (int)severity->v.len, json + severity->v.pos);
        //   snprintf(Cause, sizeof(Cause), "%.*s", (int)cause->v.len, json + cause->v.pos);

        //   c_info->c_info->events->onTrackException(guildId, &parsedTrack, Message, Severity, Cause);
        //   break;
        // }
        // case 'u': { /* TrackStuckEvent */
        //   struct coglink_parsedTrack parsedTrack = _coglink_buildTrackStruct(c_info, pairs, json);
        //   if (parsedTrack.encoded[0] == '\0') return;

        //   jsmnf_pair *thresholdMs = FIND_FIELD("thresholdMs", sizeof("thresholdMs") - 1);

        //   size_t ThresholdMs;
        //   PAIR_TO_SIZET(thresholdMs, "thresholdMs", "ThresholdMs", 16);

        //   c_info->c_info->events->onTrackStuck(guildId, ThresholdMs, &parsedTrack);
        //   break;
        // }
        // case 't': { /* WebSocketClosedEvent */
        //   if (!c_info->c_info->events->onWebSocketClosed) return;

        //   jsmnf_pair *code = FIND_FIELD("code", sizeof("code") - 1);
        //   jsmnf_pair *reason = FIND_FIELD("reason", sizeof("reason") - 1);
        //   jsmnf_pair *byRemote = FIND_FIELD("byRemote", sizeof("byRemote") - 1);

        //   char Code[16], Reason[128];
        //   bool ByRemote;

        //   snprintf(Code, sizeof(Code), "%.*s", (int)code->v.len, json + code->v.pos);
        //   snprintf(Reason, sizeof(Reason), "%.*s", (int)reason->v.len, json + reason->v.pos);

        //   if (json[byRemote->v.pos] == 't') ByRemote = true;
        //   else ByRemote = false;
  
        //   c_info->c_info->events->onWebSocketClosed(guildId, Code, Reason, ByRemote);
        //   break;
        // }
        // default: {
        //   if (c_info->c_info->events->onUnknownEvent) c_info->c_info->events->onUnknownEvent(guildId, Type, json);

        //   break;
        // }
      }

      break;
    }
    case 's': { /* Stats */
      FIND_FIELD(players, "players");
      FIND_FIELD(playingPlayers, "playingPlayers");
      FIND_FIELD(uptime, "uptime");
      FIND_FIELD(memory, "memory");

      char *path[] = { "memory", "free" };
      FIND_FIELD_PATH(lavaFree, "free", 2);

      path[1] = "used";
      FIND_FIELD_PATH(used, "used", 2);

      path[1] = "allocated";
      FIND_FIELD_PATH(allocated, "allocated", 2);

      path[1] = "reservable";
      FIND_FIELD_PATH(reservable, "reservable", 2);

      path[0] = "cpu";
      path[1] = "cores";
      FIND_FIELD_PATH(cores, "cores", 2);

      path[1] = "systemLoad";
      FIND_FIELD_PATH(systemLoad, "systemLoad", 2);

      path[1] = "lavalinkLoad";
      FIND_FIELD_PATH(lavalinkLoad, "lavalinkLoad", 2);

      path[0] = "frameStats";
      path[1] = "sent";
      FIND_FIELD_PATH(sent, "sent", 2);

      path[1] = "deficit";
      FIND_FIELD_PATH(deficit, "deficit", 2);

      path[1] = "nulled";
      FIND_FIELD_PATH(nulled, "nulled", 2);

      struct coglink_stats_payload *stats = malloc(sizeof(struct coglink_stats_payload));
      stats->memory = malloc(sizeof(struct coglink_stats_memory_payload));
      stats->cpu = malloc(sizeof(struct coglink_stats_cpu_payload));
      stats->frameStats = malloc(sizeof(struct coglink_stats_frame_stats_payload));

      PAIR_TO_SIZET(players, playersStr, stats->players, 8);
      PAIR_TO_SIZET(playingPlayers, playingPlayersStr, stats->playingPlayers, 16);
      PAIR_TO_SIZET(uptime, uptimeStr, stats->uptime, 8);
      PAIR_TO_SIZET(lavaFree, freeStr, stats->memory->free, 8);
      PAIR_TO_SIZET(used, usedStr, stats->memory->used, 8);
      PAIR_TO_SIZET(allocated, allocatedStr, stats->memory->allocated, 8);
      PAIR_TO_SIZET(reservable, reservableStr, stats->memory->reservable, 8);
      PAIR_TO_SIZET(cores, coresStr, stats->cpu->cores, 8);
      PAIR_TO_SIZET(systemLoad, systemLoadStr, stats->cpu->systemLoad, 8);
      PAIR_TO_SIZET(lavalinkLoad, lavalinkLoadStr, stats->cpu->lavalinkLoad, 8);
      PAIR_TO_SIZET(sent, sentStr, stats->frameStats->sent, 8);
      PAIR_TO_SIZET(deficit, deficitStr, stats->frameStats->deficit, 8);
      PAIR_TO_SIZET(nulled, nulledStr, stats->frameStats->nulled, 8);

      DEBUG("[coglink:jsmn-find] Parsed error search json, results:\n> Players: %s\n> PlayingPlayers: %s\n> Uptime: %s\n> Free: %s\n> Used: %s\n> Allocated: %s\n> Reservable: %s\n> Cores: %s\n> SystemLoad: %s\n> LavalinkLoad: %s\n> Sent: %s\n> Nulled: %s\n> Deficit: %s", playersStr, playingPlayersStr, uptimeStr, freeStr, usedStr, allocatedStr, reservableStr, coresStr, systemLoadStr, lavalinkLoadStr, sentStr, nulledStr, deficitStr);

      *event_type = COGLINK_STATS;

      return stats;
    }
    case 'p': { /* PlayerUpdate */
      FIND_FIELD(guildId, "guildId");

      char *path[] = { "state", "time" };
      FIND_FIELD_PATH(time, "time", 2);

      path[1] = "position";
      FIND_FIELD_PATH(position, "position", 2);

      path[1] = "connected";
      FIND_FIELD_PATH(connected, "connected", 2);

      path[1] = "ping";
      FIND_FIELD_PATH(ping, "ping", 2);

      struct coglink_player_update_payload *playerUpdate = malloc(sizeof(struct coglink_player_update_payload));
      playerUpdate->state = malloc(sizeof(struct coglink_player_state_payload));

      PAIR_TO_SIZET(guildId, guildIdStr, playerUpdate->guildId, 18);
      PAIR_TO_SIZET(time, timeStr, playerUpdate->state->time, 16);
      PAIR_TO_SIZET(position, positionStr, playerUpdate->state->position, 16);
      PAIR_TO_SIZET(ping, pingStr, playerUpdate->state->ping, 8);
      if (json[connected->v.pos] == 't') playerUpdate->state->connected = true;
      else playerUpdate->state->connected = false;

      *event_type = COGLINK_PLAYER_UPDATE;

      return playerUpdate;
    }
    // default: {
    //   if (c_info->c_info->events->onUnknownOp) c_info->c_info->events->onUnknownOp(Op, json);
    //   break;
    // }
  }

  return NULL;
}

struct coglink_load_tracks_response *coglink_parse_load_tracks_response(const char *json, size_t length) {
  jsmn_parser parser;
  jsmntok_t tokens[64];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return NULL;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[64];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return NULL;
  }

  jsmnf_pair *loadType = jsmnf_find(pairs, json, "loadType", sizeof("loadType") - 1);
  if (!loadType) return NULL;

  switch (json[loadType->v.pos + 1]) {
    // case 'r': { /* tRack */
    //   struct coglink_tracks *tracks = malloc(sizeof(struct coglink_tracks));
    //   tracks->array = malloc(sizeof(struct coglink_partial_track) * 1);
    //   tracks->size = 1;

    //   _coglink_parse_track(pairs, json); /* Defines track_info */

    //   tracks->array[0] = *track_info->info;

    //   struct coglink_load_tracks_response *response = malloc(sizeof(struct coglink_load_tracks_response));
    //   response->type = COGLINK_LOAD_TYPE_TRACK;
    //   response->tracks = tracks;

    //   return response;
    // }
    // case 'l': { /* pLaylist */
    //   struct coglink_tracks *tracks = malloc(sizeof(struct coglink_tracks));
    //   tracks->array = malloc(sizeof(struct coglink_partial_track) * 1);
    //   tracks->size = 1;

    //   _coglink_parse_track(pairs, json); /* Defines track_info */

    //   tracks->array[0] = *track_info->info;

    //   struct coglink_load_tracks_response *response = malloc(sizeof(struct coglink_load_tracks_response));
    //   response->type = COGLINK_LOAD_TYPE_PLAYLIST;
    //   response->tracks = tracks;

    //   return response;
    // }
    case 'e': { /* sEarch */
      struct coglink_tracks *tracks = malloc(sizeof(struct coglink_tracks));
      tracks->array = malloc(sizeof(struct coglink_partial_track) * 1);
      tracks->size = 1;

      _coglink_parse_track(pairs, json); /* Defines track_info */

      tracks->array[0] = *track_info->info;

      struct coglink_load_tracks_response *response = malloc(sizeof(struct coglink_load_tracks_response));
      response->type = COGLINK_LOAD_TYPE_SEARCH;
      response->tracks = tracks;

      return response;
    }
    // case 'm': { /* eMpty */
    //   struct coglink_load_tracks_response *response = malloc(sizeof(struct coglink_load_tracks_response));
    //   response->type = COGLINK_LOAD_TYPE_EMPTY;
    //   response->tracks = NULL;

    //   return response;
    // }
    // case 'r': { /* eRror */
    //   struct coglink_load_tracks_response *response = malloc(sizeof(struct coglink_load_tracks_response));
    //   response->type = COGLINK_LOAD_TYPE_ERROR;
    //   response->tracks = NULL;

    //   return response;
    // }
  }

  return NULL;
}

struct coglink_voice_state *coglink_parse_voice_state(const char *json, size_t length) {
  jsmn_parser parser;
  jsmntok_t tokens[128];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return NULL;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[128];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return NULL;
  }

  jsmnf_pair *guild_id = jsmnf_find(pairs, json, "guild_id", sizeof("guild_id") - 1);
  if (!guild_id) return NULL;

  jsmnf_pair *channel_id = jsmnf_find(pairs, json, "channel_id", sizeof("channel_id") - 1);

  jsmnf_pair *user_id = jsmnf_find(pairs, json, "user_id", sizeof("user_id") - 1);
  if (!user_id) return NULL;

  jsmnf_pair *session_id = jsmnf_find(pairs, json, "session_id", sizeof("session_id") - 1);
  if (!session_id) return NULL;

  struct coglink_voice_state *voiceState = malloc(sizeof(struct coglink_voice_state));

  PAIR_TO_SIZET(guild_id, guild_id_str, voiceState->guild_id, 18);
  PAIR_TO_SIZET(channel_id, channel_id_str, voiceState->channel_id, 18);
  if (channel_id) {
    PAIR_TO_SIZET(user_id, user_id_str, voiceState->user_id, 18);
  } else {
    voiceState->user_id = 0;
  }
  voiceState->session_id = malloc(session_id->v.len + 1);
  snprintf(voiceState->session_id, session_id->v.len + 1, "%.*s", (int)session_id->v.len, json + session_id->v.pos);

  return voiceState;
}

struct coglink_voice_server_update *coglink_parse_voice_server_update(const char *json, size_t length) {
  jsmn_parser parser;
  jsmntok_t tokens[128];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return NULL;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[128];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return NULL;
  }

  jsmnf_pair *token = jsmnf_find(pairs, json, "token", sizeof("token") - 1);
  if (!token) return NULL;

  jsmnf_pair *endpoint = jsmnf_find(pairs, json, "endpoint", sizeof("endpoint") - 1);
  if (!endpoint) return NULL;

  jsmnf_pair *guild_id = jsmnf_find(pairs, json, "guild_id", sizeof("guild_id") - 1);
  if (!guild_id) return NULL;

  struct coglink_voice_server_update *voiceServerUpdate = malloc(sizeof(struct coglink_voice_server_update));

  voiceServerUpdate->token = malloc(token->v.len + 1);
  snprintf(voiceServerUpdate->token, token->v.len + 1, "%.*s", (int)token->v.len, json + token->v.pos);

  voiceServerUpdate->endpoint = malloc(endpoint->v.len + 1);
  snprintf(voiceServerUpdate->endpoint, endpoint->v.len + 1, "%.*s", (int)endpoint->v.len, json + endpoint->v.pos);

  PAIR_TO_SIZET(guild_id, guild_id_str, voiceServerUpdate->guild_id, 18);

  return voiceServerUpdate;
}