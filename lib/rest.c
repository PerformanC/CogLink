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
  int bestStatsNode = 0, bestStats = -1;

  do {
    if (!c_client->nodes->array[i].stats) continue;

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

  queue->array[queue->size - 1] = track;

  return COGLINK_SUCCESS;
}

int coglink_remove_track_from_queue(struct coglink_client *c_client, struct coglink_player *player, char *track) {
  struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);

  for (size_t i = 0; i < queue->size; i++) {
    if (strcmp(queue->array[i], track) == 0) {
      queue->array[i] = '\0';

      for (size_t j = i; j < queue->size - 1; j++) {
        queue->array[j] = queue->array[j + 1];
      }

      queue->array = realloc(queue->array, sizeof(char *) * (queue->size - 1));

      return COGLINK_SUCCESS;
    }
  }

  return COGLINK_NOT_FOUND;
}

int coglink_remove_player(struct coglink_client *c_client, struct coglink_player *player) {
  player->guild_id = 0;
  player->node = 0;
  player->queue->size = 0;
  player->queue->array = NULL;

  c_client->players->size--;

  return COGLINK_SUCCESS;
}

int coglink_load_tracks(struct coglink_client *c_client, struct coglink_player *player, char *identifier) {
  struct coglink_node *node = &c_client->nodes->array[player->node];

  if (node->session_id == NULL) return COGLINK_NODE_READY;

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

  printf("%s\n", res->body);

  free(endpoint);
  
  return COGLINK_SUCCESS;
}

int coglink_play_track(struct coglink_client *c_client, struct coglink_player *player, char *track) {
  struct coglink_node *node = &c_client->nodes->array[player->node];

  if (node->session_id == NULL) return COGLINK_NODE_READY;

  size_t payload_size = (sizeof("{\"track\":{\"encoded\":\"\"}}") - 1) + strlen(track) + 1;
  char *payload = malloc(payload_size);
  snprintf(payload, payload_size, "{\"track\":{\"encoded\":\"%s\"}}", track);

  size_t endpoint_size = (sizeof("/sessions//guilds/") - 1) + COGLINK_SESSION_ID_LENGTH + 19 + 1;
  char *endpoint = malloc(endpoint_size);
  snprintf(endpoint, endpoint_size, "/sessions/%s/guilds/%" PRIu64 "", node->session_id, player->guild_id);

  _coglink_perform_request(node, &(struct coglink_request_params) {
    .endpoint = endpoint,
    .method = "POST",
    .body = payload,
    .body_length = payload_size,
    .get_response = false
  }, NULL);

  free(payload);
  free(endpoint);

  return COGLINK_SUCCESS;
}