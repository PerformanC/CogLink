/* TODO: Use cog-chef (https://github.com/Cogmasters/cog-chef)*/

#include <stdlib.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include "utils.h"

#include "codecs.h"

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
      FIND_FIELD(json, type, "type");
      FIND_FIELD(json, guildId, "guildId");

      switch(json[type->v.pos + 7]) {
        case 'a': { /* TrackStartEvent */
          struct coglink_track_start_payload *parsedTrack = malloc(sizeof(struct coglink_track_start_payload));

          PAIR_TO_SIZET(json, guildId, guildIdStr, parsedTrack->guildId, 18);

          jsmnf_pair *event_track_info = jsmnf_find(pairs, json, "track", sizeof("track") - 1);
          coglink_parse_track(event_track_info, json); /* Defines track_info */

          parsedTrack->track = track_info;

          *event_type = COGLINK_TRACK_START;

          return parsedTrack;
        }
        case 'd': { /* TrackEndEvent */
          FIND_FIELD(json, reason, "reason");

          struct coglink_track_end_payload *parsedTrack = malloc(sizeof(struct coglink_track_end_payload));

          PAIR_TO_SIZET(json, guildId, guildIdStr, parsedTrack->guildId, 18);

          jsmnf_pair *event_track_info = jsmnf_find(pairs, json, "track", sizeof("track") - 1);
          coglink_parse_track(event_track_info, json); /* Defines track_info */

          parsedTrack->track = track_info;

          snprintf(parsedTrack->reason, sizeof(parsedTrack->reason), "%.*s", (int)reason->v.len, json + reason->v.pos);

          *event_type = COGLINK_TRACK_END;

          return parsedTrack;
        }
        case 'c': { /* TrackExceptionEvent */
          FIND_FIELD(json, message, "message");
          FIND_FIELD(json, severity, "severity");
          FIND_FIELD(json, cause, "cause");

          struct coglink_track_exception_payload *parsedTrack = malloc(sizeof(struct coglink_track_end_payload));

          PAIR_TO_SIZET(json, guildId, guildIdStr, parsedTrack->guildId, 18);

          jsmnf_pair *event_track_info = jsmnf_find(pairs, json, "track", sizeof("track") - 1);
          coglink_parse_track(event_track_info, json); /* Defines track_info */

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
          FIND_FIELD(json, reason, "thresholdMs");

          struct coglink_track_stuck_payload *parsedTrack = malloc(sizeof(struct coglink_track_end_payload));

          PAIR_TO_SIZET(json, guildId, guildIdStr, parsedTrack->guildId, 18);

          jsmnf_pair *event_track_info = jsmnf_find(pairs, json, "track", sizeof("track") - 1);
          coglink_parse_track(event_track_info, json); /* Defines track_info */

          parsedTrack->track = track_info;

          PAIR_TO_SIZET(json, reason, thresholdMsStr, parsedTrack->thresholdMs, 8);

          *event_type = COGLINK_TRACK_STUCK;

          return parsedTrack;
        }
        case 't': { /* WebSocketClosedEvent */
          FIND_FIELD(json, code, "code");
          FIND_FIELD(json, reason, "reason");
          FIND_FIELD(json, byRemote, "byRemote");

          struct coglink_websocket_closed_payload *c_info = malloc(sizeof(struct coglink_websocket_closed_payload));

          PAIR_TO_SIZET(json, code, codeStr, c_info->code, 8);
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
      FIND_FIELD(json, players, "players");
      FIND_FIELD(json, playingPlayers, "playingPlayers");
      FIND_FIELD(json, uptime, "uptime");
      FIND_FIELD(json, memory, "memory");

      char *path[] = { "memory", "free" };
      FIND_FIELD_PATH(json, pairs, lavaFree, "free", 2);

      path[1] = "used";
      FIND_FIELD_PATH(json, pairs, used, "used", 2);

      path[1] = "allocated";
      FIND_FIELD_PATH(json, pairs, allocated, "allocated", 2);

      path[1] = "reservable";
      FIND_FIELD_PATH(json, pairs, reservable, "reservable", 2);

      path[0] = "cpu";
      path[1] = "cores";
      FIND_FIELD_PATH(json, pairs, cores, "cores", 2);

      path[1] = "systemLoad";
      FIND_FIELD_PATH(json, pairs, systemLoad, "systemLoad", 2);

      path[1] = "lavalinkLoad";
      FIND_FIELD_PATH(json, pairs, lavalinkLoad, "lavalinkLoad", 2);

      path[0] = "frameStats";
      path[1] = "sent";
      FIND_FIELD_PATH(json, pairs, sent, "sent", 2);

      path[1] = "deficit";
      FIND_FIELD_PATH(json, pairs, deficit, "deficit", 2);

      path[1] = "nulled";
      FIND_FIELD_PATH(json, pairs, nulled, "nulled", 2);

      struct coglink_stats_payload *stats = malloc(sizeof(struct coglink_stats_payload));
      stats->memory = malloc(sizeof(struct coglink_stats_memory_payload));
      stats->cpu = malloc(sizeof(struct coglink_stats_cpu_payload));
      stats->frameStats = malloc(sizeof(struct coglink_stats_frame_stats_payload));

      PAIR_TO_SIZET(json, players, playersStr, stats->players, 8);
      PAIR_TO_SIZET(json, playingPlayers, playingPlayersStr, stats->playingPlayers, 16);
      PAIR_TO_SIZET(json, uptime, uptimeStr, stats->uptime, 8);
      PAIR_TO_SIZET(json, lavaFree, freeStr, stats->memory->free, 8);
      PAIR_TO_SIZET(json, used, usedStr, stats->memory->used, 8);
      PAIR_TO_SIZET(json, allocated, allocatedStr, stats->memory->allocated, 8);
      PAIR_TO_SIZET(json, reservable, reservableStr, stats->memory->reservable, 8);
      PAIR_TO_SIZET(json, cores, coresStr, stats->cpu->cores, 8);
      PAIR_TO_SIZET(json, systemLoad, systemLoadStr, stats->cpu->systemLoad, 8);
      PAIR_TO_SIZET(json, lavalinkLoad, lavalinkLoadStr, stats->cpu->lavalinkLoad, 8);
      PAIR_TO_SIZET(json, sent, sentStr, stats->frameStats->sent, 8);
      PAIR_TO_SIZET(json, deficit, deficitStr, stats->frameStats->deficit, 8);
      PAIR_TO_SIZET(json, nulled, nulledStr, stats->frameStats->nulled, 8);

      DEBUG("[coglink:jsmn-find] Parsed error search json, results:\n> Players: %s\n> PlayingPlayers: %s\n> Uptime: %s\n> Free: %s\n> Used: %s\n> Allocated: %s\n> Reservable: %s\n> Cores: %s\n> SystemLoad: %s\n> LavalinkLoad: %s\n> Sent: %s\n> Nulled: %s\n> Deficit: %s", playersStr, playingPlayersStr, uptimeStr, freeStr, usedStr, allocatedStr, reservableStr, coresStr, systemLoadStr, lavalinkLoadStr, sentStr, nulledStr, deficitStr);

      *event_type = COGLINK_STATS;

      return stats;
    }
    case 'p': { /* PlayerUpdate */
      FIND_FIELD(json, guildId, "guildId");

      char *path[] = { "state", "time" };
      FIND_FIELD_PATH(json, pairs, time, "time", 2);

      path[1] = "position";
      FIND_FIELD_PATH(json, pairs, position, "position", 2);

      path[1] = "connected";
      FIND_FIELD_PATH(json, pairs, connected, "connected", 2);

      path[1] = "ping";
      FIND_FIELD_PATH(json, pairs, ping, "ping", 2);

      struct coglink_player_update_payload *playerUpdate = malloc(sizeof(struct coglink_player_update_payload));
      playerUpdate->state = malloc(sizeof(struct coglink_player_state_payload));

      PAIR_TO_SIZET(json, guildId, guildIdStr, playerUpdate->guildId, 18);
      PAIR_TO_SIZET(json, time, timeStr, playerUpdate->state->time, 16);
      PAIR_TO_SIZET(json, position, positionStr, playerUpdate->state->position, 16);
      PAIR_TO_SIZET(json, ping, pingStr, playerUpdate->state->ping, 8);
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

  response->type = COGLINK_LOAD_TYPE_EMPTY;
  response->data = NULL;

  switch (json[loadType->v.pos + 3]) {
    case 'c': { /* traCk */
      jsmnf_pair *data = jsmnf_find(pairs, json, "data", sizeof("data") - 1);

      coglink_parse_track(data, json); /* Defines track_info */

      response->type = COGLINK_LOAD_TYPE_TRACK;
      response->data = track_info;

      goto cleanup;
    }
    case 'y': { /* plaYlist */
      char *playlist_path[] = { "data", "tracks", NULL };

      jsmnf_pair *tracks = jsmnf_find_path(pairs, json, playlist_path, 2);

      struct coglink_load_tracks_playlist_response *data = malloc(sizeof(struct coglink_load_tracks_playlist_response));
      data->info = malloc(sizeof(struct coglink_playlist_info));
      data->tracks = malloc(sizeof(struct coglink_tracks));
      data->tracks->array = malloc(sizeof(struct coglink_track) * tracks->size);
      data->tracks->size = tracks->size;

      for (int i = 0; i < tracks->size; i++) {
        char i_str[11];
        snprintf(i_str, sizeof(i_str), "%d", i);

        char *track_path[] = { "data", "tracks", i_str };
        jsmnf_pair *track_pair = jsmnf_find_path(pairs, json, track_path, 3);

        coglink_parse_track(track_pair, json); /* Defines track_info */

        data->tracks->array[i] = *track_info;
      }

      playlist_path[1] = "info";
      playlist_path[2] = "name";

      jsmnf_pair *playlist_name = jsmnf_find_path(pairs, json, playlist_path, 3);
      snprintf(data->info->name, sizeof(data->info->name), "%.*s", (int)playlist_name->v.len, json + playlist_name->v.pos);

      playlist_path[2] = "selectedTrack";
      jsmnf_pair *selected_track = jsmnf_find_path(pairs, json, playlist_path, 3);
      PAIR_TO_SIZET(json, selected_track, selectedTrackStr, data->info->selectedTrack, 16);

      response->type = COGLINK_LOAD_TYPE_PLAYLIST;
      response->data = data;

      goto cleanup;
    }
    case 'r': { /* seaRch */
      jsmnf_pair *tracks = jsmnf_find(pairs, json, "data", sizeof("data") - 1);

      struct coglink_load_tracks_search_response *data = malloc(sizeof(struct coglink_load_tracks_search_response));
      data->array = malloc(sizeof(struct coglink_track) * tracks->size);
      data->size = tracks->size;

      for (int i = 0; i < tracks->size; i++) {
        char i_str[11];
        snprintf(i_str, sizeof(i_str), "%d", i);

        char *track_path[] = { "data", i_str };
        jsmnf_pair *track_pair = jsmnf_find_path(pairs, json, track_path, 2);

        coglink_parse_track(track_pair, json); /* Defines track_info */

        data->array[i] = *track_info;
      }

      response->type = COGLINK_LOAD_TYPE_SEARCH;
      response->data = data;

      goto cleanup;
    }
    case 't': { /* empTy */
      /* Nothing to do */

      goto cleanup;
    }
    case 'o': { /* errOr */
      char *path[] = { "exception", "message" };

      FIND_FIELD_PATH(json, pairs, message, "message", 2);

      path[1] = "severity";
      FIND_FIELD_PATH(json, pairs, severity, "severity", 2);

      path[1] = "cause";
      FIND_FIELD_PATH(json, pairs, cause, "cause", 2);

      struct coglink_load_tracks_error_response *data = malloc(sizeof(struct coglink_load_tracks_error_response));
      response->type = COGLINK_LOAD_TYPE_ERROR;

      data->message = malloc(message->v.len + 1);
      snprintf(data->message, message->v.len + 1, "%.*s", (int)message->v.len, json + message->v.pos);

      data->severity = malloc(severity->v.len + 1);
      snprintf(data->severity, severity->v.len + 1, "%.*s", (int)severity->v.len, json + severity->v.pos);

      data->cause = malloc(cause->v.len + 1);
      snprintf(data->cause, cause->v.len + 1, "%.*s", (int)cause->v.len, json + cause->v.pos);

      response->data = data;

      goto cleanup;
    }
  }

  cleanup: {
    free(toks);
    free(pairs);
  }

  return NULL;
}

void coglink_free_load_tracks_response(struct coglink_load_tracks_response *response) {
  switch (response->type) {
    case COGLINK_LOAD_TYPE_TRACK: {
      struct coglink_load_tracks_track_response *data = response->data;

      free(data->info);
      free(data);

      break;
    }
    case COGLINK_LOAD_TYPE_PLAYLIST: {
      struct coglink_load_tracks_playlist_response *data = response->data;

      free(data->info);

      for (size_t i = 0; i < data->tracks->size; i++) {
        free(data->tracks->array[i].info);
      }
      
      free(data->tracks);
      free(data);

      break;
    }
    case COGLINK_LOAD_TYPE_SEARCH: {
      struct coglink_load_tracks_search_response *data = response->data;

      for (size_t i = 0; i < data->size; i++) {
        free(data->array[i].info);
      }

      free(data->array);
      free(data);

      break;
    }
    case COGLINK_LOAD_TYPE_EMPTY: {
      /* Nothing to free */

      break;
    }
    case COGLINK_LOAD_TYPE_ERROR: {
      struct coglink_load_tracks_error_response *data = response->data;

      free(data->message);
      free(data->severity);
      free(data->cause);
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

  PAIR_TO_SIZET(json, guild_id, guild_id_str, voiceState->guild_id, 18);
  PAIR_TO_SIZET(json, channel_id, channel_id_str, voiceState->channel_id, 18);
  if (channel_id) {
    PAIR_TO_SIZET(json, user_id, user_id_str, voiceState->user_id, 18);
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

  PAIR_TO_SIZET(json, guild_id, guild_id_str, voiceServerUpdate->guild_id, 18);

  return voiceServerUpdate;
}