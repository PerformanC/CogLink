#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>
#include <concord/types.h>

#include "lavalink.h"
#include "utils.h"

#include "rest.h"

/* todo: option for more algorithms */
int _coglink_select_node(struct coglink_client *c_client) {
  size_t i = 0;
  int bestStatsNode = -1, bestStats = -1;

  do {
    if (!c_client->nodes->array[i].stats) {
      if (bestStatsNode == -1 && c_client->nodes->array[i].session_id) bestStatsNode = i;

      continue;
    }

    int systemLoad = c_client->nodes->array[i].stats->cpu->systemLoad;
    int cores = c_client->nodes->array[i].stats->cpu->cores;

    int stats = (systemLoad / cores) * 100;

    if (stats < bestStats) {
      bestStats = stats;
      bestStatsNode = i;
    }
  } while (++i < c_client->nodes->size);

  return bestStatsNode;
}

struct coglink_user *coglink_get_user(struct coglink_client *c_client, u64snowflake user_id) {
  for (size_t i = 0; i < c_client->users->size; i++) {
    struct coglink_user *user = &c_client->users->array[i];

    if (user->id == user_id) return user;
  }

  return NULL;
}

int coglink_join_voice_channel(struct discord *client, u64snowflake guild_id, u64snowflake channel_id) {
  char payload[((sizeof("{"
    "\"op\":4,"
    "\"d\":{"
      "\"guild_id\":\"\","
      "\"channel_id\":\"\","
      "\"self_mute\":false,"
      "\"self_deaf\":true"
    "}") - 1) + (19 * 2) + 1)];

  size_t payload_size = snprintf(payload, sizeof(payload),
    "{"
      "\"op\":4,"
      "\"d\":{"
        "\"guild_id\":%"PRIu64","
        "\"channel_id\":\"%"PRIu64"\","
        "\"self_mute\":false,"
        "\"self_deaf\":true"
      "}"
    "}", guild_id, channel_id);

  if (!ws_send_text(client->gw.ws, NULL, payload, payload_size)) {
    FATAL("[coglink:cws] Something went wrong while sending a payload with op 4 to Discord.");

    return -1;
  }

  return 0;
}

struct coglink_player *coglink_create_player(struct coglink_client *c_client, u64snowflake guild_id) {
  int selected_node = _coglink_select_node(c_client);

  if (selected_node == -1) return NULL;

  size_t i = 0;

  do {
    struct coglink_player *player = &c_client->players->array[i];

    if (player->guild_id == guild_id) return player;

    if (player->guild_id == 0) {
      player->guild_id = guild_id;
      player->node = selected_node;
      player->queue = malloc(sizeof(struct coglink_player_queue));
      player->queue->size = 0;
      player->queue->array = NULL;

      c_client->players->size++;

      return player;
    }
  } while (++i < c_client->players->size);

  c_client->players->array = realloc(c_client->players->array, sizeof(struct coglink_player) * (c_client->players->size + 1));
  c_client->players->size++;

  /* todo: (?) use goto to reduce duplicated code */
  struct coglink_player *player = &c_client->players->array[c_client->players->size - 1];
  player->guild_id = guild_id;
  player->node = selected_node;
  player->queue = malloc(sizeof(struct coglink_player_queue));
  player->queue->size = 0;
  player->queue->array = NULL;

  c_client->players->size++;

  return player;
}

struct coglink_player *coglink_get_player(struct coglink_client *c_client, u64snowflake guild_id) {
  for (size_t i = 0; i < c_client->players->size; i++) {
    struct coglink_player *player = &c_client->players->array[i];

    if (player->guild_id == guild_id) return player;
  }

  return NULL;
}

/* Experimental */
struct coglink_player_queue *coglink_get_player_queue(struct coglink_client *c_client, struct coglink_player *player) {
  (void) c_client; /* Standard */
  return player->queue;
}

int coglink_add_track_to_queue(struct coglink_client *c_client, struct coglink_player *player, char *track) {
  struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);

  /* realloc will automatically allocate if it's invalid */
  queue->array = realloc(queue->array, sizeof(char *) * (queue->size + 1));
  queue->size++;

  /* copy */
  queue->array[queue->size - 1] = malloc(strlen(track) + 1);
  strcpy(queue->array[queue->size - 1], track);

  return COGLINK_SUCCESS;
}

int coglink_remove_track_from_queue(struct coglink_client *c_client, struct coglink_player *player, size_t position) {
  struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);

  if (position >= queue->size) {
    FATAL("[coglink] Attempted to remove a track from an invalid position.");
  }

  free(queue->array[position]);
  queue->array[position] = '\0';

  for (size_t j = position; j < queue->size - 1; j++) {
    queue->array[j] = queue->array[j + 1];
  }

  queue->array = realloc(queue->array, sizeof(char *) * (queue->size - 1));
  queue->size--;

  return COGLINK_SUCCESS;
}

int coglink_remove_player(struct coglink_client *c_client, struct coglink_player *player) {
  player->guild_id = 0;
  player->node = 0;
  player->queue->size = 0;
  player->queue->array = NULL;

  c_client->players->size--;

  return COGLINK_SUCCESS;
}

int coglink_load_tracks(struct coglink_client *c_client, struct coglink_player *player, char *identifier, struct coglink_load_tracks_response *response) {
  struct coglink_node *node = &c_client->nodes->array[player->node];

  if (node->session_id == NULL) return COGLINK_NODE_OFFLINE;

  size_t endpoint_size = (sizeof("/loadtracks?identifier=") - 1) + strlen(identifier) + 1;
  char *endpoint = malloc(endpoint_size);
  snprintf(endpoint, endpoint_size, "/loadtracks?identifier=%s", identifier);

  struct coglink_response *res = malloc(sizeof(struct coglink_response));

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .body = NULL,
    .body_length = 0,
    .get_response = true
  }, res);

  coglink_parse_load_tracks_response(response, res->body, res->size);

  free(endpoint);
  free(res->body);
  free(res);
  
  return COGLINK_SUCCESS;
}

/* todo: decentralize from a player */
struct coglink_track *coglink_decode_track(struct coglink_client *c_client, struct coglink_player *player, char *track) {
  struct coglink_node *node = &c_client->nodes->array[player->node];

  if (node->session_id == NULL) return NULL;

  size_t endpoint_size = (sizeof("/decodetrack?encodedTrack=") - 1) + strlen(track) + 1;
  char *endpoint = malloc(endpoint_size);
  snprintf(endpoint, endpoint_size, "/decodetrack?encodedTrack=%s", track);

  struct coglink_response *res = malloc(sizeof(struct coglink_response));

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .get_response = true
  }, res);

  jsmn_parser parser;
  jsmntok_t tokens[64];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return NULL;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[64];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return NULL;
  }

  coglink_parse_track(pairs, res->body);

  free(endpoint);

  return track_info;
}

struct coglink_tracks *coglink_decode_tracks(struct coglink_client *c_client, struct coglink_player *player, struct coglink_decode_tracks_params *params) {
  struct coglink_node *node = &c_client->nodes->array[player->node];

  if (node->session_id == NULL) return NULL;

  char *endpoint = "/decodetracks";
  char *body = malloc(1 + 1);
  snprintf(body, 2, "[");
  size_t body_length = 0;

  for (size_t i = 0; i < params->size; i++) {
    size_t track_size = strlen(params->array[i]) + 1;
    body = realloc(body, body_length + track_size + 1);
    snprintf(body + body_length, track_size + 1, "\"%s\",", params->array[i]);
    body_length += track_size;
  }

  body[body_length - 1] = ']';

  struct coglink_response *res = malloc(sizeof(struct coglink_response));

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "POST",
    .body = body,
    .body_length = body_length,
    .get_response = true
  }, res);

  jsmn_parser parser;
  jsmntok_t *toks = NULL;
  unsigned num_tokens = 0;

  jsmn_init(&parser);
  int r = jsmn_parse_auto(&parser, res->body, res->size, &toks, &num_tokens);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return NULL;
  }

  jsmnf_loader loader;
  jsmnf_pair *pairs = NULL;
  unsigned num_pairs = 0;

  jsmnf_init(&loader);
  r = jsmnf_load_auto(&loader, res->body, toks, num_tokens, &pairs, &num_pairs);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to load jsmn-find.");

    return NULL;
  }

  struct coglink_tracks *tracks_info = malloc(sizeof(struct coglink_tracks *));
  tracks_info->array = malloc(sizeof(struct coglink_track) * pairs->size);
  tracks_info->size = pairs->size;

  for (int i = 0; i < pairs->size; i++) {
    coglink_parse_track(pairs, res->body);

    tracks_info->array[i] = track_info;
  }

  free(body);
  free(res->body);
  free(res);
  free(toks);
  free(pairs);

  return tracks_info;
}

void coglink_free_decode_tracks(struct coglink_tracks *tracks) {
  for (size_t i = 0; i < tracks->size; i++) {
    free(tracks->array[i]->encoded);
    free(tracks->array[i]);
  }

  free(tracks->array);
  free(tracks);
}

/* todo: use json-build by müller */
int coglink_update_player(struct coglink_client *c_client, struct coglink_player *player, struct coglink_update_player_params *params) {
  struct coglink_node *node = &c_client->nodes->array[player->node];

  if (node->session_id == NULL) return COGLINK_NODE_OFFLINE;

  size_t payload_size = (sizeof("{") - 1) + 1;
  bool should_add_comma = false;
  char *payload = malloc(payload_size);
  payload[payload_size - 2] = '{';
  payload[payload_size - 1] = '\0';

  size_t endpoint_size = (sizeof("/sessions//players/") - 1) + COGLINK_SESSION_ID_LENGTH + 19 + 1;
  char *endpoint = malloc(endpoint_size);
  snprintf(endpoint, endpoint_size, "/sessions/%s/players/%" PRIu64 "", node->session_id, player->guild_id);

  if (params->track) {
    if (params->track->encoded) {
      size_t track_size = (sizeof("\"track\":{\"encoded\":\"\"") - 1) + strlen(params->track->encoded) + 1;

      payload = realloc(payload, payload_size + track_size);
      snprintf(payload + payload_size - 1, track_size, "\"track\":{\"encoded\":\"%s\"", params->track->encoded);
      payload_size += track_size - 1;

      if (params->track->identifier || params->track->userData) {
        payload = realloc(payload, payload_size + 1);
        payload[payload_size - 1] = ',';
        payload[payload_size] = '\0';
        payload_size++;
      }
    }

    if (params->track->identifier) {
      size_t identifier_size = ((params->track->encoded ? 1 : 0) + (sizeof("\"identifier\":\"\"") - 1) + strlen(params->track->identifier) + 1);

      payload = realloc(payload, payload_size + identifier_size);
      snprintf(payload + payload_size - 1, identifier_size, "%s\"identifier\":\"%s\"", (params->track->encoded ? "," : ""), params->track->identifier);
      payload_size += identifier_size - 1;

      if (params->track->userData) {
        payload = realloc(payload, payload_size + 1);
        payload[payload_size - 1] = ',';
        payload[payload_size] = '\0';
        payload_size++;
      }
    }

    if (params->track->userData) {
      size_t userData_size = ((params->track->encoded || params->track->identifier ? 1 : 0) + (sizeof("\"userData\":\"\"") - 1) + strlen(params->track->userData) + 1);

      payload = realloc(payload, payload_size + userData_size);
      snprintf(payload + payload_size - 1, userData_size, "%s\"userData\":\"%s\"", (params->track->encoded || params->track->identifier ? "," : ""), params->track->userData);
      payload_size += userData_size - 1;
    }

    payload = realloc(payload, payload_size + 1);
    payload[payload_size - 1] = '}';
    payload[payload_size] = '\0';
    payload_size++;

    should_add_comma = true;
  }

  if (params->position) {
    size_t position_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"position\":") - 1) + 10 + 1;

    payload = realloc(payload, payload_size + position_size);
    snprintf(payload + payload_size - 1, position_size, "%s\"position\":%d", (should_add_comma ? "," : ""), params->position);
    payload_size += position_size - 1;

    should_add_comma = true;
  }

  if (params->endTime) {
    size_t endTime_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"endTime\":") - 1) + 10 + 1;

    payload = realloc(payload, payload_size + endTime_size);
    snprintf(payload + payload_size - 1, endTime_size, "%s\"endTime\":%d", (should_add_comma ? "," : ""), params->endTime);
    payload_size += endTime_size - 1;

    should_add_comma = true;
  }

  if (params->volume) {
    size_t volume_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"volume\":") - 1) + 10 + 1;

    payload = realloc(payload, payload_size + volume_size);
    snprintf(payload + payload_size - 1, volume_size, "%s\"volume\":%d", (should_add_comma ? "," : ""), params->volume);
    payload_size += volume_size - 1;

    should_add_comma = true;
  }

  if (params->paused) {
    size_t paused_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"paused\":") - 1) + 5 + 1;

    payload = realloc(payload, payload_size + paused_size);
    snprintf(payload + payload_size - 1, paused_size, "%s\"paused\":%s", (should_add_comma ? "," : ""), (params->paused ? "true" : "false"));
    payload_size += paused_size - 1;

    should_add_comma = true;
  }

  if (params->filters) {
    size_t filters_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"filters\":") - 1) + 1 + 1;

    payload = realloc(payload, payload_size + filters_size);
    snprintf(payload + payload_size - 1, filters_size, "%s\"filters\":{", (should_add_comma ? "," : ""));
    payload_size += filters_size - 1;

    if (params->filters->volume) {
      size_t volume_size = (sizeof("\"volume\":") - 1) + 10 + 1;

      payload = realloc(payload, payload_size + volume_size);
      snprintf(payload + payload_size - 1, volume_size, "\"volume\":%f", params->filters->volume);
      payload_size += volume_size - 1;

      should_add_comma = true;
    }

    if (params->filters->equalizer) {
      size_t equalizer_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"equalizer\":[]") - 1) + (15 * (sizeof("{\"band\":-x.x,\"gain\":-x.x}") - 1)) + 1;

      payload = realloc(payload, payload_size + equalizer_size);
      snprintf(payload + payload_size - 1, equalizer_size, "%s\"equalizer\":[", (should_add_comma ? "," : ""));
      payload_size += equalizer_size - 1;

      for (size_t i = 0; i < 15; i++) {
        size_t equalizer_params_size = (sizeof("{\"band\":-x.x,\"gain\":-x.x}") - 1) + 10 + 1;

        payload = realloc(payload, payload_size + equalizer_params_size);
        snprintf(payload + payload_size - 1, equalizer_params_size, "{\"band\":%d,\"gain\":%f}", params->filters->equalizer[i].band, params->filters->equalizer[i].gain);
        payload_size += equalizer_params_size - 1;

        if (i < 15 - 1) {
          payload = realloc(payload, payload_size + 1);
          payload[payload_size - 1] = ',';
          payload[payload_size] = '\0';
          payload_size++;
        }
      }

      payload = realloc(payload, payload_size + 1);
      payload[payload_size - 1] = ']';
      payload[payload_size] = '\0';

      should_add_comma = true;
    }

    if (params->filters->karaoke) {
      size_t karaoke_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"karaoke\":{}") - 1) + (sizeof("{\"level\":-x.x,\"monoLevel\":-x.x,\"filterBand\":-x.x,\"filterWidth\":-x.x}") - 1) + 1;

      payload = realloc(payload, payload_size + karaoke_size);
      snprintf(payload + payload_size - 1, karaoke_size, "%s\"karaoke\":{\"level\":%f,\"monoLevel\":%f,\"filterBand\":%f,\"filterWidth\":%f}", (should_add_comma ? "," : ""), params->filters->karaoke->level, params->filters->karaoke->monoLevel, params->filters->karaoke->filterBand, params->filters->karaoke->filterWidth);
      payload_size += karaoke_size - 1;

      should_add_comma = true;
    }

    if (params->filters->timescale) {
      size_t timescale_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"timescale\":{}") - 1) + (sizeof("{\"speed\":-x.x,\"pitch\":-x.x,\"rate\":-x.x}") - 1) + 1;

      payload = realloc(payload, payload_size + timescale_size);
      snprintf(payload + payload_size - 1, timescale_size, "%s\"timescale\":{\"speed\":%f,\"pitch\":%f,\"rate\":%f}", (should_add_comma ? "," : ""), params->filters->timescale->speed, params->filters->timescale->pitch, params->filters->timescale->rate);
      payload_size += timescale_size - 1;

      should_add_comma = true;
    }

    if (params->filters->tremolo) {
      size_t tremolo_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"tremolo\":{}") - 1) + (sizeof("{\"frequency\":-x.x,\"depth\":-x.x}") - 1) + 1;

      payload = realloc(payload, payload_size + tremolo_size);
      snprintf(payload + payload_size - 1, tremolo_size, "%s\"tremolo\":{\"frequency\":%f,\"depth\":%f}", (should_add_comma ? "," : ""), params->filters->tremolo->frequency, params->filters->tremolo->depth);
      payload_size += tremolo_size - 1;

      should_add_comma = true;
    }

    if (params->filters->vibrato) {
      size_t vibrato_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"vibrato\":{}") - 1) + (sizeof("{\"frequency\":-x.x,\"depth\":-x.x}") - 1) + 1;

      payload = realloc(payload, payload_size + vibrato_size);
      snprintf(payload + payload_size - 1, vibrato_size, "%s\"vibrato\":{\"frequency\":%f,\"depth\":%f}", (should_add_comma ? "," : ""), params->filters->vibrato->frequency, params->filters->vibrato->depth);
      payload_size += vibrato_size - 1;

      should_add_comma = true;
    }

    if (params->filters->rotation) {
      size_t rotation_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"rotation\":{}") - 1) + (sizeof("{\"frequency\":-x.x,\"depth\":-x.x}") - 1) + 1;

      payload = realloc(payload, payload_size + rotation_size);
      snprintf(payload + payload_size - 1, rotation_size, "%s\"rotation\":{\"frequency\":%f,\"depth\":%f}", (should_add_comma ? "," : ""), params->filters->rotation->frequency, params->filters->rotation->depth);
      payload_size += rotation_size - 1;

      should_add_comma = true;
    }

    if (params->filters->distortion) {
      size_t distortion_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"distortion\":{}") - 1) + (sizeof("{\"sinOffset\":-x.x,\"sinScale\":-x.x,\"cosOffset\":-x.x,\"cosScale\":-x.x,\"tanOffset\":-x.x,\"tanScale\":-x.x,\"offset\":-x.x,\"scale\":-x.x}") - 1) + 1;

      payload = realloc(payload, payload_size + distortion_size);
      snprintf(payload + payload_size - 1, distortion_size, "%s\"distortion\":{\"sinOffset\":%f,\"sinScale\":%f,\"cosOffset\":%f,\"cosScale\":%f,\"tanOffset\":%f,\"tanScale\":%f,\"offset\":%f,\"scale\":%f}", (should_add_comma ? "," : ""), params->filters->distortion->sinOffset, params->filters->distortion->sinScale, params->filters->distortion->cosOffset, params->filters->distortion->cosScale, params->filters->distortion->tanOffset, params->filters->distortion->tanScale, params->filters->distortion->offset, params->filters->distortion->scale);
      payload_size += distortion_size - 1;

      should_add_comma = true;
    }

    if (params->filters->channelMix) {
      size_t channelMix_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"channelMix\":{}") - 1) + (sizeof("{\"leftToLeft\":-x.x,\"leftToRight\":-x.x,\"rightToLeft\":-x.x,\"rightToRight\":-x.x}") - 1) + 1;

      payload = realloc(payload, payload_size + channelMix_size);
      snprintf(payload + payload_size - 1, channelMix_size, "%s\"channelMix\":{\"leftToLeft\":%f,\"leftToRight\":%f,\"rightToLeft\":%f,\"rightToRight\":%f}", (should_add_comma ? "," : ""), params->filters->channelMix->leftToLeft, params->filters->channelMix->leftToRight, params->filters->channelMix->rightToLeft, params->filters->channelMix->rightToRight);
      payload_size += channelMix_size - 1;

      should_add_comma = true;
    }

    if (params->filters->lowPass) {
      size_t lowPass_size = (should_add_comma ? (sizeof(",") - 1) : 0) + (sizeof("\"lowPass\":{}") - 1) + (sizeof("{\"smoothing\":-x.x}") - 1) + 1;

      payload = realloc(payload, payload_size + lowPass_size);
      snprintf(payload + payload_size - 1, lowPass_size, "%s\"lowPass\":{\"smoothing\":%f}", (should_add_comma ? "," : ""), params->filters->lowPass->smoothing);
      payload_size += lowPass_size - 1;

      should_add_comma = true;
    }

    payload = realloc(payload, payload_size + 1);
    payload[payload_size - 1] = '}';
    payload[payload_size] = '\0';
    payload_size++;

    should_add_comma = true;
  }

  payload = realloc(payload, payload_size + 1);
  payload[payload_size - 1] = '}';
  payload[payload_size] = '\0';
  payload_size++;

  /* todo: implement JSON parsing */
  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "PATCH",
    .body = payload,
    .body_length = payload_size - 1,
    .get_response = false
  }, NULL);

  free(payload);
  free(endpoint);

  return COGLINK_SUCCESS;
}

void coglink_destroy_player(struct coglink_client *c_client, struct coglink_player *player) {
  struct coglink_node *node = &c_client->nodes->array[player->node];

  if (node->session_id == NULL) return;

  size_t endpoint_size = (sizeof("/sessions//players/") - 1) + COGLINK_SESSION_ID_LENGTH + 19 + 1;
  char *endpoint = malloc(endpoint_size);
  snprintf(endpoint, endpoint_size, "/sessions/%s/players/%" PRIu64 "", node->session_id, player->guild_id);

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "DELETE",
    .body = NULL,
    .body_length = 0,
    .get_response = false
  }, NULL);

  free(endpoint);
}

/* todo: implement update session (?) */

int coglink_get_node_info(struct coglink_client *c_client, struct coglink_node *node, struct coglink_node_info *info) {
  (void) c_client; /* Standard */

  if (node->session_id == NULL) return COGLINK_NODE_OFFLINE;

  char endpoint[(sizeof("/info") - 1) + 1];
  snprintf(endpoint, sizeof(endpoint), "/info");

  struct coglink_response *res = malloc(sizeof(struct coglink_response));

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .body = NULL,
    .body_length = 0,
    .get_response = true
  }, res);

  coglink_parse_node_info(info, res->body, res->size);

  free(res->body);
  free(res);

  return COGLINK_SUCCESS;
}

char *coglink_get_node_version(struct coglink_client *c_client, struct coglink_node *node) {
  (void) c_client; /* Standard */

  char endpoint[(sizeof("/version") - 1) + 1];
  snprintf(endpoint, sizeof(endpoint), "/version");

  struct coglink_response *res = malloc(sizeof(struct coglink_response));

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .body = NULL,
    .body_length = 0,
    .get_response = true,
    .unversioned = true
  }, res);

  char *version = malloc(res->size + 1);
  strcpy(version, res->body);

  free(res->body);
  free(res);

  return version;
}

void coglink_free_node_version(char *version) {
  free(version);
}

int coglink_get_stats(struct coglink_client *c_client, struct coglink_node *node, struct coglink_stats_payload *stats) {
  (void) c_client; /* Standard */

  char endpoint[(sizeof("/stats") - 1) + 1];
  snprintf(endpoint, sizeof(endpoint), "/stats");

  struct coglink_response *res = malloc(sizeof(struct coglink_response));

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .body = NULL,
    .body_length = 0,
    .get_response = true
  }, res);

  coglink_parse_stats(stats, res->body, res->size);

  free(res->body);
  free(res);

  return COGLINK_SUCCESS;
}

/* todo: support tunnelbroker endpoints (?) */