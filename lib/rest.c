#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>
#include <concord/types.h>

#include "lavalink.h"
#include "utils.h"
#include "jsonb.h"

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

int coglink_join_voice_channel(struct coglink_client *c_client, struct discord *client, u64snowflake guild_id, u64snowflake channel_id) {
  (void) c_client; /* Standard */

  char payload[((sizeof("{"
    "\"op\":4,"
    "\"d\":{"
      "\"guild_id\":\"\","
      "\"channel_id\":\"\","
      "\"self_mute\":false,"
      "\"self_deaf\":true"
    "}") - 1) + ((19 * 2) + 1) + 1)];

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

    return COGLINK_FAILED;
  }

  return COGLINK_SUCCESS;
}

int coglink_leave_voice_channel(struct coglink_client *c_client, struct discord *client, u64snowflake guild_id) {
  (void) c_client; /* Standard */

  char payload[((sizeof("{"
    "\"op\":4,"
    "\"d\":{"
      "\"guild_id\":\"\","
      "\"channel_id\":null,"
      "\"self_mute\":false,"
      "\"self_deaf\":true"
    "}") - 1) + (19 + 1) + 1)];

  size_t payload_size = snprintf(payload, sizeof(payload),
    "{"
      "\"op\":4,"
      "\"d\":{"
        "\"guild_id\":%"PRIu64","
        "\"channel_id\":null,"
        "\"self_mute\":false,"
        "\"self_deaf\":true"
      "}"
    "}", guild_id);

  if (!ws_send_text(client->gw.ws, NULL, payload, payload_size)) {
    FATAL("[coglink:cws] Something went wrong while sending a payload with op 4 to Discord.");

    return COGLINK_FAILED;
  }

  return COGLINK_SUCCESS;
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

struct coglink_player_queue *coglink_get_player_queue(struct coglink_client *c_client, struct coglink_player *player) {
  (void) c_client; /* Standard */

  return player->queue;
}

int coglink_add_track_to_queue(struct coglink_client *c_client, struct coglink_player *player, char *track) {
  struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);

  /* realloc will automatically allocate if it's invalid */
  queue->array = realloc(queue->array, sizeof(char *) * (queue->size + 1));
  queue->size++;

  queue->array[queue->size - 1] = strdup(track);

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

int coglink_load_tracks(struct coglink_client *c_client, struct coglink_node *node, char *identifier, struct coglink_load_tracks *response) {
  size_t endpoint_size = (sizeof("/loadtracks?identifier=") - 1) + strlen(identifier) + 1;
  char *endpoint = malloc(endpoint_size * sizeof(char));
  snprintf(endpoint, endpoint_size * sizeof(char), "/loadtracks?identifier=%s", identifier);

  struct coglink_response res = { 0 };

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .body = NULL,
    .body_length = 0,
    .get_response = true
  }, &res);

  int status = coglink_parse_load_tracks(response, res.body, res.size);

  free(endpoint);
  free(res.body);
  
  return status;
}

int coglink_decode_track(struct coglink_client *c_client, struct coglink_node *node, char *track, struct coglink_track *response) {
  (void) c_client; /* Standard */

  size_t endpoint_size = (sizeof("/decodetrack?encodedTrack=") - 1) + strlen(track) + 1;
  char *endpoint = malloc(endpoint_size * sizeof(char));
  snprintf(endpoint, endpoint_size * sizeof(char), "/decodetrack?encodedTrack=%s", track);

  struct coglink_response res = { 0 };

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .get_response = true
  }, &res);

  jsmn_parser parser;
  jsmntok_t tokens[64];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res.body, res.size, tokens, sizeof(tokens));

  if (r < 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[64];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res.body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    FATAL("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_FAILED;
  }

  coglink_parse_track(response, pairs, res.body);

  free(res.body);
  free(endpoint);

  return COGLINK_SUCCESS;
}

int coglink_decode_tracks(struct coglink_client *c_client, struct coglink_node *node, struct coglink_decode_tracks_params *params, struct coglink_tracks *response) {
  (void) c_client; /* Standard */

  char *endpoint = "/decodetracks";
  struct pjsonb jsonber;
  pjsonb_init(&jsonber);

  pjsonb_enter_array(&jsonber, "tracks");

  for (size_t i = 0; i < params->size; i++) {
    pjsonb_set_string(&jsonber, NULL, params->array[i]);
  }

  pjsonb_leave_array(&jsonber);

  pjsonb_end(&jsonber);

  struct coglink_response res = { 0 };

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "POST",
    .body = jsonber.string,
    .body_length = jsonber.position,
    .get_response = true
  }, &res);

  pjsonb_free(&jsonber);

  jsmn_parser parser;
  jsmntok_t *toks = NULL;
  unsigned num_tokens = 0;

  jsmn_init(&parser);
  int r = jsmn_parse_auto(&parser, res.body, res.size, &toks, &num_tokens);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to parse JSON.");

    return COGLINK_FAILED;
  }

  jsmnf_loader loader;
  jsmnf_pair *pairs = NULL;
  unsigned num_pairs = 0;

  jsmnf_init(&loader);
  r = jsmnf_load_auto(&loader, res.body, toks, num_tokens, &pairs, &num_pairs);
  if (r <= 0) {
    ERROR("[coglink:jsmn-find] Failed to load jsmn-find.");

    return COGLINK_FAILED;
  }

  response->array = malloc(sizeof(struct coglink_track) * pairs->size);
  response->size = pairs->size;

  for (int i = 0; i < pairs->size; i++) {
    response->array[i] = malloc(sizeof(struct coglink_track));
    response->array[i]->info = malloc(sizeof(struct coglink_track_info));

    coglink_parse_track(response->array[i], pairs, res.body);
  }

  free(res.body);
  free(toks);
  free(pairs);

  return COGLINK_SUCCESS;
}

int coglink_update_player(struct coglink_client *c_client, struct coglink_player *player, struct coglink_update_player_params *params, struct coglink_update_player *response) {
  struct coglink_node *node = &c_client->nodes->array[player->node];

  if (node->session_id == NULL) return COGLINK_NODE_OFFLINE;

  size_t endpoint_size = (sizeof("/sessions//players/") - 1) + COGLINK_SESSION_ID_LENGTH + 19 + 1;
  char *endpoint = malloc(endpoint_size * sizeof(char));
  snprintf(endpoint, endpoint_size * sizeof(char), "/sessions/%s/players/%" PRIu64 "", node->session_id, player->guild_id);

  struct pjsonb jsonber;
  pjsonb_init(&jsonber);

  if (params->track) {
    pjsonb_enter_object(&jsonber, "track");

    if (params->track->encoded) pjsonb_set_string(&jsonber, "encoded", params->track->encoded);
    if (params->track->identifier) pjsonb_set_string(&jsonber, "identifier", params->track->identifier);
    if (params->track->userData) pjsonb_set_string(&jsonber, "userData", params->track->userData);

    pjsonb_leave_object(&jsonber);
  }

  if (params->position) {
    pjsonb_set_int(&jsonber, "position", params->position);
  }

  if (params->endTime) {
    pjsonb_set_int(&jsonber, "endTime", params->endTime);
  }

  if (params->volume) {
    pjsonb_set_int(&jsonber, "volume", params->volume);
  }

  if (params->paused) {
    pjsonb_set_bool(&jsonber, "paused", params->paused);
  }

  if (params->filters) {
    pjsonb_enter_object(&jsonber, "filters");

    if (params->filters->volume) {
      pjsonb_set_int(&jsonber, "volume", params->filters->volume);
    }

    if (params->filters->equalizer) {
      pjsonb_enter_array(&jsonber, "equalizer");

      for (size_t i = 0; i < 15; i++) {
        pjsonb_enter_object(&jsonber, NULL);

        pjsonb_set_int(&jsonber, "band", params->filters->equalizer[i].band);
        pjsonb_set_int(&jsonber, "gain", params->filters->equalizer[i].gain);

        pjsonb_leave_object(&jsonber);
      }

      pjsonb_leave_array(&jsonber);
    }

    if (params->filters->karaoke) {
      pjsonb_enter_object(&jsonber, "karaoke");

      pjsonb_set_int(&jsonber, "level", params->filters->karaoke->level);
      pjsonb_set_int(&jsonber, "monoLevel", params->filters->karaoke->monoLevel);
      pjsonb_set_int(&jsonber, "filterBand", params->filters->karaoke->filterBand);
      pjsonb_set_int(&jsonber, "filterWidth", params->filters->karaoke->filterWidth);

      pjsonb_leave_object(&jsonber);
    }

    if (params->filters->timescale) {
      pjsonb_enter_object(&jsonber, "timescale");

      pjsonb_set_int(&jsonber, "speed", params->filters->timescale->speed);
      pjsonb_set_int(&jsonber, "pitch", params->filters->timescale->pitch);
      pjsonb_set_int(&jsonber, "rate", params->filters->timescale->rate);

      pjsonb_leave_object(&jsonber);
    }

    if (params->filters->tremolo) {
      pjsonb_enter_object(&jsonber, "tremolo");

      pjsonb_set_int(&jsonber, "frequency", params->filters->tremolo->frequency);
      pjsonb_set_int(&jsonber, "depth", params->filters->tremolo->depth);

      pjsonb_leave_object(&jsonber);
    }

    if (params->filters->vibrato) {
      pjsonb_enter_object(&jsonber, "vibrato");

      pjsonb_set_int(&jsonber, "frequency", params->filters->vibrato->frequency);
      pjsonb_set_int(&jsonber, "depth", params->filters->vibrato->depth);

      pjsonb_leave_object(&jsonber);
    }

    if (params->filters->rotation) {
      pjsonb_enter_object(&jsonber, "rotation");

      pjsonb_set_int(&jsonber, "frequency", params->filters->rotation->frequency);
      pjsonb_set_int(&jsonber, "depth", params->filters->rotation->depth);

      pjsonb_leave_object(&jsonber);
    }

    if (params->filters->distortion) {
      pjsonb_enter_object(&jsonber, "distortion");

      pjsonb_set_int(&jsonber, "sinOffset", params->filters->distortion->sinOffset);
      pjsonb_set_int(&jsonber, "sinScale", params->filters->distortion->sinScale);
      pjsonb_set_int(&jsonber, "cosOffset", params->filters->distortion->cosOffset);
      pjsonb_set_int(&jsonber, "cosScale", params->filters->distortion->cosScale);
      pjsonb_set_int(&jsonber, "tanOffset", params->filters->distortion->tanOffset);
      pjsonb_set_int(&jsonber, "tanScale", params->filters->distortion->tanScale);
      pjsonb_set_int(&jsonber, "offset", params->filters->distortion->offset);
      pjsonb_set_int(&jsonber, "scale", params->filters->distortion->scale);

      pjsonb_leave_object(&jsonber);
    }

    if (params->filters->channelMix) {
      pjsonb_enter_object(&jsonber, "channelMix");

      pjsonb_set_int(&jsonber, "leftToLeft", params->filters->channelMix->leftToLeft);
      pjsonb_set_int(&jsonber, "leftToRight", params->filters->channelMix->leftToRight);
      pjsonb_set_int(&jsonber, "rightToLeft", params->filters->channelMix->rightToLeft);
      pjsonb_set_int(&jsonber, "rightToRight", params->filters->channelMix->rightToRight);

      pjsonb_leave_object(&jsonber);
    }

    if (params->filters->lowPass) {
      pjsonb_enter_object(&jsonber, "lowPass");

      pjsonb_set_int(&jsonber, "smoothing", params->filters->lowPass->smoothing);

      pjsonb_leave_object(&jsonber);
    }

    pjsonb_leave_object(&jsonber);
  }

  pjsonb_end(&jsonber);

  struct coglink_response res = { 0 };

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "PATCH",
    .body = jsonber.string,
    .body_length = jsonber.position,
    .get_response = true
  }, &res);

  int status = coglink_parse_update_player(response, res.body, res.size);

  free(res.body);
  pjsonb_free(&jsonber);
  free(endpoint);

  return status;
}

void coglink_destroy_player(struct coglink_client *c_client, struct coglink_player *player) {
  struct coglink_node *node = &c_client->nodes->array[player->node];

  if (node->session_id == NULL) return;

  size_t endpoint_size = (sizeof("/sessions//players/") - 1) + COGLINK_SESSION_ID_LENGTH + 19 + 1;
  char *endpoint = malloc(endpoint_size * sizeof(char));
  snprintf(endpoint, endpoint_size * sizeof(char), "/sessions/%s/players/%" PRIu64 "", node->session_id, player->guild_id);

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

  struct coglink_response res = { 0 };

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .body = NULL,
    .body_length = 0,
    .get_response = true
  }, &res);

  int status = coglink_parse_node_info(info, res.body, res.size);

  free(res.body);

  return status;
}

int coglink_get_node_version(struct coglink_client *c_client, struct coglink_node *node, struct coglink_node_version *version) {
  (void) c_client; /* Standard */

  char endpoint[(sizeof("/version") - 1) + 1];
  snprintf(endpoint, sizeof(endpoint), "/version");

  struct coglink_response res = { 0 };

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .body = NULL,
    .body_length = 0,
    .get_response = true,
    .unversioned = true
  }, &res);

  int status = coglink_parse_version(version, res.body, res.size);

  free(res.body);

  return status;
}

int coglink_get_stats(struct coglink_client *c_client, struct coglink_node *node, struct coglink_stats *stats) {
  (void) c_client; /* Standard */

  char endpoint[(sizeof("/stats") - 1) + 1];
  snprintf(endpoint, sizeof(endpoint), "/stats");

  struct coglink_response res = { 0 };

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "GET",
    .body = NULL,
    .body_length = 0,
    .get_response = true
  }, &res);

  int status = coglink_parse_stats(stats, res.body, res.size);

  free(res.body);

  return status;
}

/* todo: support tunnelbroker endpoints (?) */