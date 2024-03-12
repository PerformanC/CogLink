/* TODO: Use cog-chef (https://github.com/Cogmasters/cog-chef)*/

#include <stdlib.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include "utils.h"

#include "codecs.h"

#define _coglink_parse_track(pairs, json)                                                                                       \
  struct coglink_track *track_info = malloc(sizeof(struct coglink_partial_track));                                              \
  track_info->info = malloc(sizeof(struct coglink_partial_track));                                                              \
                                                                                                                                \
  char *path[] = { "encoded", NULL };                                                                                           \
  FIND_FIELD_PATH(pairs, encoded, "encoded", 1);                                                                                       \
  snprintf(track_info->encoded, sizeof(track_info->encoded), "%.*s", (int)encoded->v.len, json + encoded->v.pos);               \
                                                                                                                                \
  path[0] = "info";                                                                                                             \
  path[1] = "identifier";                                                                                                       \
  FIND_FIELD_PATH(pairs, identifier, "identifier", 2);                                                                                 \
  snprintf(track_info->info->identifier, sizeof(track_info->info->identifier), "%.*s", (int)identifier->v.len, json + identifier->v.pos); \
                                                                                                                                \
  path[1] = "isSeekable";                                                                                                       \
  FIND_FIELD_PATH(pairs, isSeekable, "isSeekable", 2);                                                                                 \
  if (json[isSeekable->v.pos] == 't') track_info->info->isSeekable = true;                                                           \
  else track_info->info->isSeekable = false;                                                                                         \
                                                                                                                                \
  path[1] = "author";                                                                                                           \
  FIND_FIELD_PATH(pairs, author, "author", 2);                                                                                         \
  snprintf(track_info->info->author, sizeof(track_info->info->author), "%.*s", (int)author->v.len, json + author->v.pos);                 \
                                                                                                                                \
  path[1] = "length";                                                                                                           \
  FIND_FIELD_PATH(pairs, length, "length", 2);                                                                                         \
  PAIR_TO_SIZET(length, lengthStr, track_info->info->length, 16);                                                                    \
                                                                                                                                \
  path[1] = "isStream";                                                                                                         \
  FIND_FIELD_PATH(pairs, isStream, "isStream", 2);                                                                                     \
  if (json[isStream->v.pos] == 't') track_info->info->isStream = true;                                                               \
  else track_info->info->isStream = false;                                                                                           \
                                                                                                                                \
  path[1] = "position";                                                                                                         \
  FIND_FIELD_PATH(pairs, position, "position", 2);                                                                                     \
  PAIR_TO_SIZET(position, positionStr, track_info->info->position, 16);                                                              \
                                                                                                                                \
  path[1] = "title";                                                                                                            \
  FIND_FIELD_PATH(pairs, title, "title", 2);                                                                                           \
  snprintf(track_info->info->title, sizeof(track_info->info->title), "%.*s", (int)title->v.len, json + title->v.pos);                     \
                                                                                                                                \
  path[1] = "uri";                                                                                                              \
  FIND_FIELD_PATH(pairs, uri, "uri", 2);                                                                                               \
  snprintf(track_info->info->uri, sizeof(track_info->info->uri), "%.*s", (int)uri->v.len, json + uri->v.pos);                             \
                                                                                                                                \
  path[1] = "isrc";                                                                                                             \
  FIND_FIELD_PATH(pairs, isrc, "isrc", 2);                                                                                             \
  snprintf(track_info->info->isrc, sizeof(track_info->info->isrc), "%.*s", (int)isrc->v.len, json + isrc->v.pos);                         \
                                                                                                                                \
  path[1] = "artworkUrl";                                                                                                       \
  FIND_FIELD_PATH(pairs, artworkUrl, "artworkUrl", 2);                                                                                 \
  snprintf(track_info->info->artworkUrl, sizeof(track_info->info->artworkUrl), "%.*s", (int)artworkUrl->v.len, json + artworkUrl->v.pos); \
                                                                                                                                \
  path[1] = "sourceName";                                                                                                       \
  FIND_FIELD_PATH(pairs, sourceName, "sourceName", 2);                                                                                 \
  snprintf(track_info->info->sourceName, sizeof(track_info->info->sourceName), "%.*s", (int)sourceName->v.len, json + sourceName->v.pos);

void *coglink_parse_websocket_data(int *event_type, const char *json, size_t length) {
  *event_type = COGLINK_PARSE_ERROR;

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

  jsmnf_pair *op = jsmnf_find(pairs, json, "op", sizeof("op") - 1);
  if (!op) {
    ERROR("[coglink:jsmn-find] No op field found.");

    return NULL;
  }

  const char *Op = json + op->v.pos;

  switch(Op[0]) {
    case 'r': { /* Ready */
      jsmnf_pair *sessionId = jsmnf_find(pairs, json, "sessionId", sizeof("sessionId") - 1);
      if (!sessionId) {
        ERROR("[coglink:jsmn-find] No sessionId field found.");

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
    case 'e': { /* Event */
      FIND_FIELD(type, "type");
      FIND_FIELD(guildId, "guildId");

      switch(json[type->v.pos + 7]) {
        case 'a': { /* TrackStartEvent */
          struct coglink_track_start_payload *parsedTrack = malloc(sizeof(struct coglink_track_start_payload));

          PAIR_TO_SIZET(guildId, guildIdStr, parsedTrack->guildId, 18);

          jsmnf_pair *event_track_info = jsmnf_find(pairs, json, "track", sizeof("track") - 1);
          _coglink_parse_track(event_track_info, json); /* Defines track_info */

          parsedTrack->track = track_info;

          *event_type = COGLINK_TRACK_START;

          return parsedTrack;
        }
        case 'd': { /* TrackEndEvent */
          FIND_FIELD(reason, "reason");

          struct coglink_track_end_payload *parsedTrack = malloc(sizeof(struct coglink_track_end_payload));

          PAIR_TO_SIZET(guildId, guildIdStr, parsedTrack->guildId, 18);

          jsmnf_pair *event_track_info = jsmnf_find(pairs, json, "track", sizeof("track") - 1);
          _coglink_parse_track(event_track_info, json); /* Defines track_info */

          parsedTrack->track = track_info;

          snprintf(parsedTrack->reason, sizeof(parsedTrack->reason), "%.*s", (int)reason->v.len, json + reason->v.pos);

          *event_type = COGLINK_TRACK_END;

          return parsedTrack;
        }
        case 'c': { /* TrackExceptionEvent */
          FIND_FIELD(message, "message");
          FIND_FIELD(severity, "severity");
          FIND_FIELD(cause, "cause");

          struct coglink_track_exception_payload *parsedTrack = malloc(sizeof(struct coglink_track_end_payload));

          PAIR_TO_SIZET(guildId, guildIdStr, parsedTrack->guildId, 18);

          jsmnf_pair *event_track_info = jsmnf_find(pairs, json, "track", sizeof("track") - 1);
          _coglink_parse_track(event_track_info, json); /* Defines track_info */

          parsedTrack->track = track_info;
          parsedTrack->exception = malloc(sizeof(struct coglink_exception_payload));
          parsedTrack->exception->message = malloc(message->v.len + 1);
          snprintf(parsedTrack->exception->message, message->v.len + 1, "%.*s", (int)message->v.len, json + message->v.pos);
          parsedTrack->exception->severity = malloc(severity->v.len + 1);
          snprintf(parsedTrack->exception->severity, severity->v.len + 1, "%.*s", (int)severity->v.len, json + severity->v.pos);
          parsedTrack->exception->cause = malloc(cause->v.len + 1);
          snprintf(parsedTrack->exception->cause, cause->v.len + 1, "%.*s", (int)cause->v.len, json + cause->v.pos);

          *event_type = COGLINK_TRACK_EXCEPTION;

          return parsedTrack;
        }
        case 'u': { /* TrackStuckEvent */
          FIND_FIELD(reason, "thresholdMs");

          struct coglink_track_stuck_payload *parsedTrack = malloc(sizeof(struct coglink_track_end_payload));

          PAIR_TO_SIZET(guildId, guildIdStr, parsedTrack->guildId, 18);

          jsmnf_pair *event_track_info = jsmnf_find(pairs, json, "track", sizeof("track") - 1);
          _coglink_parse_track(event_track_info, json); /* Defines track_info */

          parsedTrack->track = track_info;

          PAIR_TO_SIZET(reason, thresholdMsStr, parsedTrack->thresholdMs, 8);

          *event_type = COGLINK_TRACK_STUCK;

          return parsedTrack;
        }
        case 't': { /* WebSocketClosedEvent */
          FIND_FIELD(code, "code");
          FIND_FIELD(reason, "reason");
          FIND_FIELD(byRemote, "byRemote");

          struct coglink_websocket_closed_payload *c_info = malloc(sizeof(struct coglink_websocket_closed_payload));

          PAIR_TO_SIZET(code, codeStr, c_info->code, 8);
          snprintf(c_info->reason, sizeof(c_info->reason), "%.*s", (int)reason->v.len, json + reason->v.pos);
          if (json[byRemote->v.pos] == 't') c_info->byRemote = true;
          else c_info->byRemote = false;

          *event_type = COGLINK_WEBSOCKET_CLOSED;

          return c_info;
        }
        default: {
          ERROR("[coglink:jsmn-find] Unknown event type: %.*s", (int)type->v.len, json + type->v.pos);

          *event_type = COGLINK_PARSE_ERROR;

          break;
        }
      }

      break;
    }
    case 's': { /* Stats */
      FIND_FIELD(players, "players");
      FIND_FIELD(playingPlayers, "playingPlayers");
      FIND_FIELD(uptime, "uptime");
      FIND_FIELD(memory, "memory");

      char *path[] = { "memory", "free" };
      FIND_FIELD_PATH(pairs, lavaFree, "free", 2);

      path[1] = "used";
      FIND_FIELD_PATH(pairs, used, "used", 2);

      path[1] = "allocated";
      FIND_FIELD_PATH(pairs, allocated, "allocated", 2);

      path[1] = "reservable";
      FIND_FIELD_PATH(pairs, reservable, "reservable", 2);

      path[0] = "cpu";
      path[1] = "cores";
      FIND_FIELD_PATH(pairs, cores, "cores", 2);

      path[1] = "systemLoad";
      FIND_FIELD_PATH(pairs, systemLoad, "systemLoad", 2);

      path[1] = "lavalinkLoad";
      FIND_FIELD_PATH(pairs, lavalinkLoad, "lavalinkLoad", 2);

      path[0] = "frameStats";
      path[1] = "sent";
      FIND_FIELD_PATH(pairs, sent, "sent", 2);

      path[1] = "deficit";
      FIND_FIELD_PATH(pairs, deficit, "deficit", 2);

      path[1] = "nulled";
      FIND_FIELD_PATH(pairs, nulled, "nulled", 2);

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
      FIND_FIELD_PATH(pairs, time, "time", 2);

      path[1] = "position";
      FIND_FIELD_PATH(pairs, position, "position", 2);

      path[1] = "connected";
      FIND_FIELD_PATH(pairs, connected, "connected", 2);

      path[1] = "ping";
      FIND_FIELD_PATH(pairs, ping, "ping", 2);

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
    default: {
      ERROR("[coglink:jsmn-find] Unknown event type: %.*s", (int)op->v.len, json + op->v.pos);

      *event_type = COGLINK_PARSE_ERROR;

      break;
    }
  }

  return NULL;
}

/* todo: move to int */
void *coglink_parse_load_tracks_response(struct coglink_load_tracks_response *response, const char *json, size_t length) {
  jsmn_parser parser;
  jsmntok_t *toks = NULL;
  unsigned num_tokens = 0;

  jsmn_init(&parser);
  int r = jsmn_parse_auto(&parser, json, length, &toks, &num_tokens);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return NULL;
  }

  jsmnf_loader loader;
  jsmnf_pair *pairs = NULL;
  unsigned num_pairs = 0;

  jsmnf_init(&loader);
  r = jsmnf_load_auto(&loader, json, toks, num_tokens, &pairs, &num_pairs);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to load jsmn-find.");

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
      jsmnf_pair *tracks = jsmnf_find(pairs, json, "data", sizeof("data") - 1);

      struct coglink_load_tracks_search_response *data = malloc(sizeof(struct coglink_load_tracks_search_response));
      data->array = malloc(sizeof(struct coglink_track) * tracks->size);
      data->size = tracks->size;

      for (int i = 0; i < tracks->size; i++) {
        char i_str[11];
        snprintf(i_str, sizeof(i_str), "%d", i);

        char *track_path[] = { "data", i_str };
        jsmnf_pair *track_pair = jsmnf_find_path(pairs, json, track_path, 2);

        _coglink_parse_track(track_pair, json); /* Defines track_info */

        data->array[i] = *track_info;
      }

      response->type = COGLINK_LOAD_TYPE_SEARCH;
      response->data = data;

      goto cleanup;
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

  cleanup: {
    free(toks);
    free(pairs);
  }

  return NULL;
}

void coglink_free_load_tracks_response(struct coglink_load_tracks_response *response) {
  switch (response->type) {
    case COGLINK_LOAD_TYPE_SEARCH: {
      struct coglink_load_tracks_search_response *data = response->data;

      for (size_t i = 0; i < data->size; i++) {
        free(data->array[i].info);
      }

      free(data->array);
      free(data);

      break;
    }
  }
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