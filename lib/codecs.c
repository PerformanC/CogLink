/* TODO: Use cog-chef (https://github.com/Cogmasters/cog-chef)*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include "utils.h"

#include "codecs.h"

int coglink_parse_websocket_data(const char *json, size_t json_length, void **response, int *event_type) {
  *event_type = COGLINK_PARSE_ERROR;

  jsmn_parser parser;
  jsmntok_t tokens[64];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, json_length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_PARSE_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[64];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_PARSE_FAILED;
  }

  FIND_FIELD(pairs, json, op, "op");

  const char *Op = json + op->v.pos;

  switch(Op[0]) {
    case 'r': { /* Ready */
      FIND_FIELD(pairs, json, sessionId, "sessionId");
      FIND_FIELD(pairs, json, resumed, "resumed");

      struct coglink_ready *ready = malloc(sizeof(struct coglink_ready));

      PAIR_TO_D_STRING(json, sessionId, ready->session_id);
      ready->resumed = json[resumed->v.pos] == 't';

      *response = ready;
      *event_type = COGLINK_READY;

      return COGLINK_SUCCESS;
    }
    case 'e': { /* Event */
      FIND_FIELD(pairs, json, type, "type");
      FIND_FIELD(pairs, json, guildId, "guildId");

      switch(json[type->v.pos + 7]) {
        case 'a': { /* TrackStartEvent */
          FIND_FIELD(pairs, json, event_track, "track");

          struct coglink_track_start *parsedTrack = malloc(sizeof(struct coglink_track_start));

          PAIR_TO_SIZET(json, guildId, guildIdStr, parsedTrack->guildId, 18);

          struct coglink_track *track = malloc(sizeof(struct coglink_track));
          track->info = malloc(sizeof(struct coglink_track_info));

          coglink_parse_track(track, event_track, json);

          parsedTrack->track = track;

          *response = parsedTrack;
          *event_type = COGLINK_TRACK_START;

          return COGLINK_SUCCESS;
        }
        case 'd': { /* TrackEndEvent */
          FIND_FIELD(pairs, json, reason, "reason");
          FIND_FIELD(pairs, json, event_track, "track");

          struct coglink_track_end *parsedTrack = malloc(sizeof(struct coglink_track_end));

          PAIR_TO_SIZET(json, guildId, guildIdStr, parsedTrack->guildId, 18);

          struct coglink_track *track = malloc(sizeof(struct coglink_track));
          track->info = malloc(sizeof(struct coglink_track_info));

          coglink_parse_track(track, event_track, json);

          parsedTrack->track = track;

          switch (json[reason->v.pos]) {
            case 'f': {
              parsedTrack->reason = COGLINK_TRACK_END_REASON_FINISHED;

              break;
            }
            case 'l': {
              parsedTrack->reason = COGLINK_TRACK_END_REASON_LOAD_FAILED;

              break;
            }
            case 's': {
              parsedTrack->reason = COGLINK_TRACK_END_REASON_STOPPED;

              break;
            }
            case 'p': {
              parsedTrack->reason = COGLINK_TRACK_END_REASON_REPLACED;

              break;
            }
            case 'c': {
              parsedTrack->reason = COGLINK_TRACK_END_REASON_CLEANUP;

              break;
            }
          }

          *response = parsedTrack;
          *event_type = COGLINK_TRACK_END;

          return COGLINK_SUCCESS;
        }
        case 'c': { /* TrackExceptionEvent */
          FIND_FIELD(pairs, json, message, "message");
          FIND_FIELD(pairs, json, severity, "severity");
          FIND_FIELD(pairs, json, cause, "cause");
          FIND_FIELD(pairs, json, event_track, "track");

          struct coglink_track_exception *parsedTrack = malloc(sizeof(struct coglink_track_end));

          PAIR_TO_SIZET(json, guildId, guildIdStr, parsedTrack->guildId, 18);

          struct coglink_track *track = malloc(sizeof(struct coglink_track));
          track->info = malloc(sizeof(struct coglink_track_info));

          coglink_parse_track(track, event_track, json);

          parsedTrack->track = track;
          parsedTrack->exception = malloc(sizeof(struct coglink_exception));
          PAIR_TO_D_STRING(json, message, parsedTrack->exception->message);

          switch (json[severity->v.pos]) {
            case 'c': {
              parsedTrack->exception->severity = COGLINK_EXCEPTION_SEVERITY_COMMON;

              break;
            }
            case 's': {
              parsedTrack->exception->severity = COGLINK_EXCEPTION_SEVERITY_SUSPICIOUS;

              break;
            }
            case 'f': {
              parsedTrack->exception->severity = COGLINK_EXCEPTION_SEVERITY_FAULT;

              break;
            }
          }

          PAIR_TO_D_STRING(json, cause, parsedTrack->exception->cause);

          *response = parsedTrack;
          *event_type = COGLINK_TRACK_EXCEPTION;

          return COGLINK_SUCCESS;
        }
        case 'u': { /* TrackStuckEvent */
          FIND_FIELD(pairs, json, thresholdMs, "thresholdMs");
          FIND_FIELD(pairs, json, event_track, "track");

          struct coglink_track_stuck *parsedTrack = malloc(sizeof(struct coglink_track_end));

          PAIR_TO_SIZET(json, guildId, guildIdStr, parsedTrack->guildId, 18);

          struct coglink_track *track = malloc(sizeof(struct coglink_track));
          track->info = malloc(sizeof(struct coglink_track_info));

          coglink_parse_track(track, event_track, json);

          parsedTrack->track = track;

          PAIR_TO_SIZET(json, thresholdMs, thresholdMsStr, parsedTrack->thresholdMs, 8);

          *response = parsedTrack;
          *event_type = COGLINK_TRACK_STUCK;

          return COGLINK_SUCCESS;
        }
        case 'e': { /* WebSocketClosedEvent */
          FIND_FIELD(pairs, json, code, "code");
          FIND_FIELD(pairs, json, reason, "reason");
          FIND_FIELD(pairs, json, byRemote, "byRemote");

          struct coglink_websocket_closed *c_info = malloc(sizeof(struct coglink_websocket_closed));

          PAIR_TO_SIZET(json, code, codeStr, c_info->code, 8);
          PAIR_TO_D_STRING(json, reason, c_info->reason);
          c_info->byRemote = json[byRemote->v.pos] == 't';

          *response = c_info;
          *event_type = COGLINK_WEBSOCKET_CLOSED;

          return COGLINK_SUCCESS;
        }
        default: {
          ERROR("[coglink:jsmn-find] Unknown event type: %.*s", (int)type->v.len, json + type->v.pos);

          return COGLINK_PARSE_ERROR;
        }
      }

      break;
    }
    case 's': { /* Stats */
      FIND_FIELD(pairs, json, players, "players");
      FIND_FIELD(pairs, json, playingPlayers, "playingPlayers");
      FIND_FIELD(pairs, json, uptime, "uptime");
      FIND_FIELD(pairs, json, memory, "memory");

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

      struct coglink_stats *stats = malloc(sizeof(struct coglink_stats));
      stats->memory = malloc(sizeof(struct coglink_stats_memory));
      stats->cpu = malloc(sizeof(struct coglink_stats_cpu));
      stats->frameStats = malloc(sizeof(struct coglink_stats_frame_stats));

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

      *response = stats;
      *event_type = COGLINK_STATS;

      return COGLINK_SUCCESS;
    }
    case 'p': { /* PlayerUpdate */
      FIND_FIELD(pairs, json, guildId, "guildId");

      char *path[] = { "state", "time" };
      FIND_FIELD_PATH(json, pairs, time, "time", 2);

      path[1] = "position";
      FIND_FIELD_PATH(json, pairs, position, "position", 2);

      path[1] = "connected";
      FIND_FIELD_PATH(json, pairs, connected, "connected", 2);

      path[1] = "ping";
      FIND_FIELD_PATH(json, pairs, ping, "ping", 2);

      struct coglink_player_update *playerUpdate = malloc(sizeof(struct coglink_player_update));
      playerUpdate->state = malloc(sizeof(struct coglink_player_state));

      PAIR_TO_SIZET(json, guildId, guildIdStr, playerUpdate->guildId, 18);
      PAIR_TO_SIZET(json, time, timeStr, playerUpdate->state->time, 16);
      PAIR_TO_SIZET(json, position, positionStr, playerUpdate->state->position, 16);
      PAIR_TO_SIZET(json, ping, pingStr, playerUpdate->state->ping, 8);
      if (json[connected->v.pos] == 't') playerUpdate->state->connected = true;
      else playerUpdate->state->connected = false;

      *response = playerUpdate;
      *event_type = COGLINK_PLAYER_UPDATE;

      return COGLINK_SUCCESS;
    }
    default: {
      ERROR("[coglink:jsmn-find] Unknown event type: %.*s", (int)op->v.len, json + op->v.pos);

      return COGLINK_PARSE_ERROR;
    }
  }

  return COGLINK_PARSE_ERROR;
}

void coglink_free_track(struct coglink_track *track) {
  free(track->encoded);
  free(track->info);
  free(track);
}

void coglink_free_tracks(struct coglink_tracks *tracks) {
  for (size_t i = 0; i < tracks->size; i++) {
    coglink_free_track(tracks->array[i]);
  }

  free(tracks->array);
  free(tracks);
}

int coglink_parse_load_tracks(struct coglink_load_tracks *response, const char *json, size_t json_length) {
  jsmn_parser parser;
  jsmntok_t *toks = NULL;
  unsigned num_tokens = 0;

  jsmn_init(&parser);
  int r = jsmn_parse_auto(&parser, json, json_length, &toks, &num_tokens);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_PARSE_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair *pairs = NULL;
  unsigned num_pairs = 0;

  jsmnf_init(&loader);
  r = jsmnf_load_auto(&loader, json, toks, num_tokens, &pairs, &num_pairs);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_PARSE_FAILED;
  }

  FIND_FIELD(pairs, json, loadType, "loadType");

  response->type = COGLINK_LOAD_TYPE_EMPTY;
  response->data = NULL;

  switch (json[loadType->v.pos + 3]) {
    case 'c': { /* traCk */
      jsmnf_pair *data = jsmnf_find(pairs, json, "data", sizeof("data") - 1);

      struct coglink_track *track_info = malloc(sizeof(struct coglink_track));
      track_info->info = malloc(sizeof(struct coglink_track_info));

      coglink_parse_track(track_info, data, json);

      response->type = COGLINK_LOAD_TYPE_TRACK;
      response->data = track_info;

      goto cleanup;
    }
    case 'y': { /* plaYlist */
      char *playlist_path[] = { "data", "tracks", NULL };

      jsmnf_pair *tracks = jsmnf_find_path(pairs, json, playlist_path, 2);

      struct coglink_load_tracks_playlist *data = malloc(sizeof(struct coglink_load_tracks_playlist));
      data->info = malloc(sizeof(struct coglink_playlist_info));
      data->tracks = malloc(sizeof(struct coglink_tracks));
      data->tracks->array = malloc(tracks->size * sizeof(struct coglink_track));
      data->tracks->size = tracks->size;

      for (int i = 0; i < tracks->size; i++) {
        char i_str[11];
        snprintf(i_str, sizeof(i_str), "%d", i);

        char *track_path[] = { "data", "tracks", i_str };
        jsmnf_pair *track_pair = jsmnf_find_path(pairs, json, track_path, 3);

        struct coglink_track *track_info = malloc(sizeof(struct coglink_track));
        track_info->info = malloc(sizeof(struct coglink_track_info));

        coglink_parse_track(track_info, track_pair, json);

        data->tracks->array[i] = track_info;
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
      FIND_FIELD(pairs, json, tracks, "data");

      struct coglink_load_tracks_search *data = malloc(sizeof(struct coglink_load_tracks_search));
      data->array = malloc(tracks->size * sizeof(struct coglink_track));
      data->size = tracks->size;

      for (int i = 0; i < tracks->size; i++) {
        char i_str[11];
        snprintf(i_str, sizeof(i_str), "%d", i);

        char *track_path[] = { "data", i_str };
        jsmnf_pair *track_pair = jsmnf_find_path(pairs, json, track_path, 2);

        struct coglink_track *track_info = malloc(sizeof(struct coglink_track));
        track_info->info = malloc(sizeof(struct coglink_track_info));

        coglink_parse_track(track_info, track_pair, json);

        data->array[i] = track_info;
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

      struct coglink_load_tracks_error *data = malloc(sizeof(struct coglink_load_tracks_error));
      response->type = COGLINK_LOAD_TYPE_ERROR;

      PAIR_TO_D_STRING(json, message, data->message);

      switch (json[severity->v.pos]) {
        case 'c': {
          data->severity = COGLINK_LOAD_TRACKS_ERROR_SEVERITY_COMMON;

          break;
        }
        case 's': {
          data->severity = COGLINK_LOAD_TRACKS_ERROR_SEVERITY_SUSPICIOUS;

          break;
        }
        case 'f': {
          data->severity = COGLINK_LOAD_TRACKS_ERROR_SEVERITY_FAULT;

          break;
        }
      }

      PAIR_TO_D_STRING(json, cause, data->cause);

      response->data = data;

      goto cleanup;
    }
  }

  cleanup: {
    free(toks);
    free(pairs);
  }

  return COGLINK_SUCCESS;
}

void coglink_free_load_tracks(struct coglink_load_tracks *response) {
  switch (response->type) {
    case COGLINK_LOAD_TYPE_TRACK: {
      struct coglink_load_tracks_track *data = response->data;

      FREE_NULLABLE(data->encoded);
      free(data->info);
      free(data);

      break;
    }
    case COGLINK_LOAD_TYPE_PLAYLIST: {
      struct coglink_load_tracks_playlist *data = response->data;

      free(data->info);

      for (size_t i = 0; i < data->tracks->size; i++) {
        coglink_free_track(data->tracks->array[i]);
      }
      
      free(data->tracks);
      free(data);

      break;
    }
    case COGLINK_LOAD_TYPE_SEARCH: {
      struct coglink_load_tracks_search *data = response->data;

      for (size_t i = 0; i < data->size; i++) {
        FREE_NULLABLE(data->array[i]->encoded);
        FREE_NULLABLE(data->array[i]->info);
        FREE_NULLABLE(data->array[i]);
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
      struct coglink_load_tracks_error *data = response->data;

      FREE_NULLABLE(data->message);
      FREE_NULLABLE(data->cause);
      free(data);

      break;
    }
  }
}

int coglink_parse_voice_state(const char *json, size_t json_length, struct coglink_voice_state *response) {
  jsmn_parser parser;
  jsmntok_t tokens[128];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, json_length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_PARSE_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[128];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_PARSE_FAILED;
  }

  FIND_FIELD(pairs, json, guild_id, "guild_id");
  jsmnf_pair *channel_id = jsmnf_find(pairs, json, "channel_id", sizeof("channel_id") - 1);
  FIND_FIELD(pairs, json, user_id, "user_id");
  FIND_FIELD(pairs, json, session_id, "session_id");

  PAIR_TO_SIZET(json, guild_id, guild_id_str, response->guild_id, 18);
  if (channel_id) {
    PAIR_TO_SIZET(json, channel_id, channel_id_str, response->channel_id, 18);
  } else {
    response->channel_id = 0;
  }
  PAIR_TO_SIZET(json, user_id, user_id_str, response->user_id, 18);
  PAIR_TO_D_STRING(json, session_id, response->session_id);

  return COGLINK_SUCCESS;
}

void coglink_free_voice_state(struct coglink_voice_state *voice_state) {
  (void)voice_state;

  /* Nothing to free */
}

int coglink_parse_voice_server_update(const char *json, size_t json_length, struct coglink_voice_server_update *response) {
  jsmn_parser parser;
  jsmntok_t tokens[128];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, json_length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_PARSE_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[128];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_PARSE_FAILED;
  }

  FIND_FIELD(pairs, json, token, "token");
  FIND_FIELD(pairs, json, endpoint, "endpoint");
  FIND_FIELD(pairs, json, guild_id, "guild_id");

  PAIR_TO_D_STRING(json, token, response->token);
  PAIR_TO_D_STRING(json, endpoint, response->endpoint);
  PAIR_TO_SIZET(json, guild_id, guild_id_str, response->guild_id, 18);

  return COGLINK_SUCCESS;
}

void coglink_free_voice_server_update(struct coglink_voice_server_update *voice_server_update) {
  FREE_NULLABLE(voice_server_update->token);
  FREE_NULLABLE(voice_server_update->endpoint);
}

int coglink_parse_guild_create(const char *json, size_t json_length, struct coglink_guild_create *response) {
  jsmn_parser parser;
  jsmntok_t *toks = NULL;
  unsigned num_tokens = 0;

  jsmn_init(&parser);
  int r = jsmn_parse_auto(&parser, json, json_length, &toks, &num_tokens);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_PARSE_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair *pairs = NULL;
  unsigned num_pairs = 0;

  jsmnf_init(&loader);
  r = jsmnf_load_auto(&loader, json, toks, num_tokens, &pairs, &num_pairs);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_PARSE_FAILED;
  }

  FIND_FIELD(pairs, json, id, "id");
  
  u64snowflake guild_id = 0;
  PAIR_TO_SIZET(json, id, idStr, guild_id, 18 + 1);

  response->guild_id = guild_id;
  response->pairs = pairs;

  free(toks);

  return COGLINK_SUCCESS;
}

void coglink_free_guild_create(struct coglink_guild_create *guild_create) {
  free(guild_create->pairs);
}

int coglink_parse_single_user_guild_create(jsmnf_pair *pairs, const char *json, char *i_str, u64snowflake bot_id, struct coglink_single_user_guild_create *response) {
  char *path[] = { "voice_states", i_str, "user_id" };
  FIND_FIELD_PATH(json, pairs, user_id_token, "user_id", 3);

  u64snowflake user_id = 0;
  PAIR_TO_SIZET(json, user_id_token, user_id_str, user_id, 18 + 1);

  if (user_id == bot_id) {
    path[2] = "session_id";
    FIND_FIELD_PATH(json, pairs, session_id, "session_id", 3);

    PAIR_TO_D_STRING(json, session_id, response->session_id);

    response->type = 1;
    response->user_id = user_id;

    return COGLINK_SUCCESS;
  } else {
    path[2] = "channel_id";
    FIND_FIELD_PATH(json, pairs, channel_id, "channel_id", 3);

    PAIR_TO_SIZET(json, channel_id, channel_id_str, response->vc_id, 18 + 1);

    response->user_id = user_id;
    response->type = 2;

    return COGLINK_SUCCESS;
  }
}

int coglink_parse_update_player(struct coglink_update_player *response, const char *json, size_t json_length) {
  jsmn_parser parser;
  jsmntok_t tokens[128];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, json_length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_PARSE_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[128];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_PARSE_FAILED;
  }

  FIND_FIELD(pairs, json, track, "track");
  FIND_FIELD(pairs, json, volume, "volume");
  FIND_FIELD(pairs, json, paused, "paused");
  FIND_FIELD(pairs, json, state, "state");
  FIND_FIELD(pairs, json, filters, "filters");

  response->track = malloc(sizeof(struct coglink_track));
  response->track->info = malloc(sizeof(struct coglink_track_info));

  coglink_parse_track(response->track, track, json);
  
  PAIR_TO_SIZET(json, volume, volumeStr, response->volume, 8);
  response->paused = json[paused->v.pos] == 't';

  FIND_FIELD(pairs, json, state_time, "time");
  FIND_FIELD(pairs, json, state_position, "position");
  FIND_FIELD(pairs, json, state_connected, "connected");
  FIND_FIELD(pairs, json, state_ping, "ping");

  response->state = malloc(sizeof(struct coglink_update_player_state));
  PAIR_TO_SIZET(json, state_time, state_timeStr, response->state->time, 16);
  PAIR_TO_SIZET(json, state_position, state_positionStr, response->state->position, 16);
  response->state->connected = json[state_connected->v.pos] == 't';
  PAIR_TO_SIZET(json, state_ping, state_pingStr, response->state->ping, 8);

  response->filters = malloc(sizeof(struct coglink_update_player_filters_params));

  FIND_FIELD(pairs, json, filters_volume, "volume");
  if (filters_volume) {
    response->filters->volume = atof(json + filters_volume->v.pos);
  }

  FIND_FIELD(filters, json, filters_equalizer, "equalizer");
  if (filters_equalizer) {
    FIND_FIELD(filters_equalizer, json, band, "band");
    FIND_FIELD(filters_equalizer, json, gain, "gain");

    response->filters->equalizer = malloc(sizeof(struct coglink_update_player_filters_equalizer_params));
    PAIR_TO_SIZET(json, band, bandStr, response->filters->equalizer->band, 8);
    response->filters->equalizer->gain = atof(json + gain->v.pos);
  }

  FIND_FIELD(filters, json, filters_karaoke, "karaoke");
  if (filters_karaoke) {
    FIND_FIELD(filters_karaoke, json, level, "level");
    FIND_FIELD(filters_karaoke, json, monoLevel, "monoLevel");
    FIND_FIELD(filters_karaoke, json, filterBand, "filterBand");
    FIND_FIELD(filters_karaoke, json, filterWidth, "filterWidth");

    response->filters->karaoke = malloc(sizeof(struct coglink_update_player_filters_karaoke_params));
    response->filters->karaoke->level = atof(json + level->v.pos);
    response->filters->karaoke->monoLevel = atof(json + monoLevel->v.pos);
    response->filters->karaoke->filterBand = atof(json + filterBand->v.pos);
    response->filters->karaoke->filterWidth = atof(json + filterWidth->v.pos);
  }

  FIND_FIELD(filters, json, filters_timescale, "timescale");
  if (filters_timescale) {
    FIND_FIELD(filters_timescale, json, speed, "speed");
    FIND_FIELD(filters_timescale, json, pitch, "pitch");
    FIND_FIELD(filters_timescale, json, rate, "rate");

    response->filters->timescale = malloc(sizeof(struct coglink_update_player_filters_timescale_params));
    response->filters->timescale->speed = atof(json + speed->v.pos);
    response->filters->timescale->pitch = atof(json + pitch->v.pos);
    response->filters->timescale->rate = atof(json + rate->v.pos);
  }

  FIND_FIELD(filters, json, filters_tremolo, "tremolo");
  if (filters_tremolo) {
    FIND_FIELD(filters_tremolo, json, frequency, "frequency");
    FIND_FIELD(filters_tremolo, json, depth, "depth");

    response->filters->tremolo = malloc(sizeof(struct coglink_update_player_filters_tremolo_params));
    response->filters->tremolo->frequency = atof(json + frequency->v.pos);
    response->filters->tremolo->depth = atof(json + depth->v.pos);
  }

  FIND_FIELD(filters, json, filters_vibrato, "vibrato");
  if (filters_vibrato) {
    FIND_FIELD(filters_vibrato, json, frequency, "frequency");
    FIND_FIELD(filters_vibrato, json, depth, "depth");

    response->filters->vibrato = malloc(sizeof(struct coglink_update_player_filters_vibrato_params));
    response->filters->vibrato->frequency = atof(json + frequency->v.pos);
    response->filters->vibrato->depth = atof(json + depth->v.pos);
  }

  FIND_FIELD(filters, json, filters_rotation, "rotation");
  if (filters_rotation) {
    FIND_FIELD(filters_rotation, json, frequency, "frequency");
    FIND_FIELD(filters_rotation, json, depth, "depth");

    response->filters->rotation = malloc(sizeof(struct coglink_update_player_filters_rotation_params));
    response->filters->rotation->frequency = atof(json + frequency->v.pos);
    response->filters->rotation->depth = atof(json + depth->v.pos);
  }

  FIND_FIELD(filters, json, filters_distortion, "distortion");
  if (filters_distortion) {
    FIND_FIELD(filters_distortion, json, sinOffset, "sinOffset");
    FIND_FIELD(filters_distortion, json, sinScale, "sinScale");
    FIND_FIELD(filters_distortion, json, cosOffset, "cosOffset");
    FIND_FIELD(filters_distortion, json, cosScale, "cosScale");
    FIND_FIELD(filters_distortion, json, tanOffset, "tanOffset");
    FIND_FIELD(filters_distortion, json, tanScale, "tanScale");
    FIND_FIELD(filters_distortion, json, offset, "offset");
    FIND_FIELD(filters_distortion, json, scale, "scale");

    response->filters->distortion = malloc(sizeof(struct coglink_update_player_filters_distortion_params));
    response->filters->distortion->sinOffset = atof(json + sinOffset->v.pos);
    response->filters->distortion->sinScale = atof(json + sinScale->v.pos);
    response->filters->distortion->cosOffset = atof(json + cosOffset->v.pos);
    response->filters->distortion->cosScale = atof(json + cosScale->v.pos);
    response->filters->distortion->tanOffset = atof(json + tanOffset->v.pos);
    response->filters->distortion->tanScale = atof(json + tanScale->v.pos);
    response->filters->distortion->offset = atof(json + offset->v.pos);
    response->filters->distortion->scale = atof(json + scale->v.pos);
  }

  FIND_FIELD(filters, json, filters_channelMix, "channelMix");
  if (filters_channelMix) {
    FIND_FIELD(filters_channelMix, json, leftToLeft, "leftToLeft");
    FIND_FIELD(filters_channelMix, json, leftToRight, "leftToRight");
    FIND_FIELD(filters_channelMix, json, rightToLeft, "rightToLeft");
    FIND_FIELD(filters_channelMix, json, rightToRight, "rightToRight");

    response->filters->channelMix = malloc(sizeof(struct coglink_update_player_filters_channelMix_params));
    response->filters->channelMix->leftToLeft = atof(json + leftToLeft->v.pos);
    response->filters->channelMix->leftToRight = atof(json + leftToRight->v.pos);
    response->filters->channelMix->rightToLeft = atof(json + rightToLeft->v.pos);
    response->filters->channelMix->rightToRight = atof(json + rightToRight->v.pos);
  }

  FIND_FIELD(filters, json, filters_lowPass, "lowPass");
  if (filters_lowPass) {
    FIND_FIELD(filters_lowPass, json, smoothing, "smoothing");

    response->filters->lowPass = malloc(sizeof(struct coglink_update_player_filters_lowPass_params));
    response->filters->lowPass->smoothing = atof(json + smoothing->v.pos);
  }

  return COGLINK_SUCCESS;
}

void coglink_free_update_player(struct coglink_update_player *response) {
  coglink_free_track(response->track);
  FREE_NULLABLE(response->track);
  FREE_NULLABLE(response->state);
  FREE_NULLABLE(response->filters);
  FREE_NULLABLE(response);
}

int coglink_parse_node_info(struct coglink_node_info *response, const char *json, size_t json_length) {
  jsmn_parser parser;
  jsmntok_t tokens[128];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, json_length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_PARSE_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[128];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_PARSE_FAILED;
  }

  FIND_FIELD(pairs, json, version, "version");
  FIND_FIELD(pairs, json, buildTime, "buildTime");
  FIND_FIELD(pairs, json, git, "git");
  FIND_FIELD(pairs, json, jvm, "jvm");
  FIND_FIELD(pairs, json, lavaplayer, "lavaplayer");
  FIND_FIELD(pairs, json, sourceManagers, "sourceManagers");
  FIND_FIELD(pairs, json, filters, "filters");

  char *path[] = { "version", "semver" };
  FIND_FIELD_PATH(json, pairs, semver, "semver", 2);

  path[1] = "major";
  FIND_FIELD_PATH(json, pairs, major, "major", 2);

  path[1] = "minor";
  FIND_FIELD_PATH(json, pairs, minor, "minor", 2);

  path[1] = "patch";
  FIND_FIELD_PATH(json, pairs, patch, "patch", 2);

  path[1] = "preRelease";
  FIND_FIELD_PATH(json, pairs, preRelease, "preRelease", 2);

  path[1] = "build";
  FIND_FIELD_PATH(json, pairs, build, "build", 2);

  response->version = malloc(sizeof(struct coglink_node_info_version));
  PAIR_TO_D_STRING(json, semver, response->version->semver);
  PAIR_TO_SIZET(json, major, majorStr, response->version->major, 8);
  PAIR_TO_SIZET(json, minor, minorStr, response->version->minor, 8);
  PAIR_TO_SIZET(json, patch, patchStr, response->version->patch, 8);
  PAIR_TO_D_STRING(json, preRelease, response->version->preRelease);
  PAIR_TO_D_STRING(json, build, response->version->build);

  PAIR_TO_SIZET(json, buildTime, buildTimeStr, response->buildTime, 8);

  path[0] = "git";
  path[1] = "branch";
  FIND_FIELD_PATH(json, pairs, branch, "branch", 2);

  path[1] = "commit";
  FIND_FIELD_PATH(json, pairs, commit, "commit", 2);

  path[1] = "commitTime";
  FIND_FIELD_PATH(json, pairs, commitTime, "commitTime", 2);

  response->git = malloc(sizeof(struct coglink_node_info_git));
  PAIR_TO_D_STRING(json, branch, response->git->branch);
  PAIR_TO_D_STRING(json, commit, response->git->commit);

  PAIR_TO_D_STRING(json, jvm, response->jvm);
  PAIR_TO_D_STRING(json, lavaplayer, response->lavaplayer);

  response->sourceManagers = malloc(sizeof(struct coglink_node_info_sourceManagers));
  response->sourceManagers->size = sourceManagers->size;
  response->sourceManagers->array = malloc(sizeof(char *) * sourceManagers->size);
  for (int i = 0; i < (int)sourceManagers->size; i++) {
    char i_str[11];
    snprintf(i_str, sizeof(i_str), "%d", i);

    char *path[] = { "sourceManagers", i_str };
    FIND_FIELD_PATH(json, pairs, sourceManager, "sourceManagers", 2);

    PAIR_TO_D_STRING(json, sourceManager, response->sourceManagers->array[i]);
  }

  response->filters = malloc(sizeof(struct coglink_node_info_filters));
  response->filters->size = filters->size;
  response->filters->array = malloc(sizeof(char *) * filters->size);
  for (int i = 0; i < filters->size; i++) {
    char i_str[11];
    snprintf(i_str, sizeof(i_str), "%d", i);

    char *path[] = { "filters", i_str };
    FIND_FIELD_PATH(json, pairs, filter, "filters", 2);

    PAIR_TO_D_STRING(json, filter, response->filters->array[i]);
  }

  return COGLINK_SUCCESS;
}

void coglink_free_node_info(struct coglink_node_info *response) {
  FREE_NULLABLE(response->version->semver);
  FREE_NULLABLE(response->version->preRelease);
  FREE_NULLABLE(response->version->build);
  FREE_NULLABLE(response->version);
  FREE_NULLABLE(response->git->branch);
  FREE_NULLABLE(response->git->commit);
  FREE_NULLABLE(response->git);
  FREE_NULLABLE(response->jvm);
  FREE_NULLABLE(response->lavaplayer);

  for (size_t i = 0; i < response->sourceManagers->size; i++) {
    FREE_NULLABLE(response->sourceManagers->array[i]);
  }
  FREE_NULLABLE(response->sourceManagers->array);
  FREE_NULLABLE(response->sourceManagers);

  for (size_t i = 0; i < response->filters->size; i++) {
    FREE_NULLABLE(response->filters->array[i]);
  }
  FREE_NULLABLE(response->filters->array);
  FREE_NULLABLE(response->filters);
  FREE_NULLABLE(response);
}

int coglink_parse_stats(struct coglink_stats *response, const char *json, size_t json_length) {
  jsmn_parser parser;
  jsmntok_t tokens[128];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, json_length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_PARSE_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[128];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_PARSE_FAILED;
  }

  FIND_FIELD(pairs, json, players, "players");
  FIND_FIELD(pairs, json, playingPlayers, "playingPlayers");
  FIND_FIELD(pairs, json, uptime, "uptime");
  FIND_FIELD(pairs, json, memory, "memory");

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

  /* frameStats is always null, so we don't need to parse it */

  PAIR_TO_SIZET(json, players, playersStr, response->players, 8);

  PAIR_TO_SIZET(json, playingPlayers, playingPlayersStr, response->playingPlayers, 16);

  PAIR_TO_SIZET(json, uptime, uptimeStr, response->uptime, 8);

  response->memory = malloc(sizeof(struct coglink_stats_memory));
  PAIR_TO_SIZET(json, lavaFree, freeStr, response->memory->free, 8);
  PAIR_TO_SIZET(json, used, usedStr, response->memory->used, 8);
  PAIR_TO_SIZET(json, allocated, allocatedStr, response->memory->allocated, 8);
  PAIR_TO_SIZET(json, reservable, reservableStr, response->memory->reservable, 8);

  response->cpu = malloc(sizeof(struct coglink_stats_cpu));
  PAIR_TO_SIZET(json, cores, coresStr, response->cpu->cores, 8);
  PAIR_TO_SIZET(json, systemLoad, systemLoadStr, response->cpu->systemLoad, 8);
  PAIR_TO_SIZET(json, lavalinkLoad, lavalinkLoadStr, response->cpu->lavalinkLoad, 8);

  return COGLINK_SUCCESS;
}

void coglink_free_stats(struct coglink_stats *response) {
  FREE_NULLABLE(response->memory);
  FREE_NULLABLE(response->cpu);
  FREE_NULLABLE(response);
}

int coglink_parse_update_session(struct coglink_update_session *response, const char *json, size_t json_length) {
  jsmn_parser parser;
  jsmntok_t tokens[8];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json, json_length, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_PARSE_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[8];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, json, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_PARSE_FAILED;
  }

  FIND_FIELD(pairs, json, resuming, "resuming");
  FIND_FIELD(pairs, json, timeout, "timeout");

  response->resuming = json[resuming->v.pos] == 't';
  PAIR_TO_SIZET(json, timeout, timeoutStr, response->timeout, 8);

  return COGLINK_SUCCESS;
}

void coglink_free_update_session(struct coglink_update_session *response) {
  FREE_NULLABLE(response);
}

int coglink_parse_version(struct coglink_node_version *response, const char *version, size_t version_length) {
  int i = 0;
  int dot_count = 0;
  int last_info = 0;
  /* 0 = major, 1 = minor, 2 = patch, 3 = pre-release */
  int state = 0;

  while (--version_length) {
    if (version[i] == '.') {
      if (dot_count == 0) {
        response->major = atoi(version + last_info);
      } else if (dot_count == 1) {
        response->minor = atoi(version + last_info);
      } else if (dot_count == 2) {
        response->patch = atoi(version + last_info);
      } else {
        return COGLINK_PARSE_FAILED;
      }

      last_info = i + 1;
      dot_count++;
    } else if (version[i] == '-') {
      if (dot_count == 2) {
        response->patch = atoi(version + last_info);
      } else {
        return COGLINK_PARSE_FAILED;
      }

      last_info = i + 1;
      state = 3;
    } else if (version[i] == '+') {
      if (state == 3) {
        response->preRelease = malloc(version_length + 1);
        memcpy(response->preRelease, version + last_info, version_length);
        response->preRelease[version_length] = '\0';
      } else {
        return COGLINK_PARSE_FAILED;
      }

      break;
    }

    i++;
  }

  return COGLINK_SUCCESS;
}

void coglink_free_node_version(struct coglink_node_version *version) {
  FREE_NULLABLE(version->preRelease);
}