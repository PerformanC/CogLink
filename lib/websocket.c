#include <stdio.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>
#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include "tablec.h"
#include "utils.h"
#include "codecs.h"
#include "lavalink.h"
#include "events.h"
#include "rest.h"

#include "websocket.h"

struct tablec_ht coglink_hashtable;

int _IO_poller(struct io_poller *io, CURLM *multi, void *data) {
  (void) io; (void) multi;
  struct coglink_node *node = data;

  return !ws_multi_socket_run(node->ws, &node->tstamp) ? CCORD_DISCORD_CONNECTION : CCORD_OK;
}

void _ws_on_close(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t length) {
  (void)ws; (void)info; (void)length;

  struct _coglink_websocket_data *c_info = data;

  if (c_info->c_client->events->on_close == NULL) return;

  c_info->c_client->events->on_close(wscode, reason);
}

void _ws_on_text(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t length) {
  (void) ws; (void) info;
  struct _coglink_websocket_data *c_info = data;

  if (c_info->c_client->events->on_raw && c_info->c_client->events->on_raw(c_info->c_client, text, length) != COGLINK_PROCEED) return;

  int type;
  void *payload = coglink_parse_websocket_data(&type, text, length);

  switch (type) {
    case COGLINK_READY: {
      c_info->c_client->nodes->array[c_info->node_id].session_id = ((struct coglink_ready_payload *)payload)->session_id;

      if (c_info->c_client->events->on_ready) c_info->c_client->events->on_ready((struct coglink_ready_payload *)payload);

      break;
    }
    case COGLINK_PLAYER_UPDATE: {
      if (c_info->c_client->events->on_player_update) c_info->c_client->events->on_player_update((struct coglink_player_update_payload *)payload);

      break;
    }
    case COGLINK_STATS: {
      if (c_info->c_client->events->on_stats) c_info->c_client->events->on_stats((struct coglink_stats_payload *)payload);

      break;
    }
    case COGLINK_TRACK_START: {
      if (c_info->c_client->events->on_track_start) c_info->c_client->events->on_track_start((struct coglink_track_start_payload *)payload);

      break;
    }
    case COGLINK_TRACK_END: {
      struct coglink_track_end_payload *track_end = (struct coglink_track_end_payload *)payload;

      struct coglink_player *player = coglink_get_player(c_info->c_client, track_end->guildId);

      if (player == NULL) {
        DEBUG("[coglink] Player not found for guild %" PRIu64 "", track_end->guildId);

        return;
      }

      struct coglink_player_queue *queue = coglink_get_player_queue(c_info->c_client, player);

      if (queue->size != 0) {
        coglink_remove_track_from_queue(c_info->c_client, player, 0);

        if (queue->size != 0) {
          struct coglink_update_player_params data = {
            .track = &(struct coglink_update_player_track_params) {
              .encoded = queue->array[0]
            }
          };

          coglink_update_player(c_info->c_client, player, &data);
        }
      }

      if (c_info->c_client->events->on_track_end) c_info->c_client->events->on_track_end(track_end);

      break;
    }
    case COGLINK_TRACK_EXCEPTION: {
      if (c_info->c_client->events->on_track_excetion) c_info->c_client->events->on_track_excetion((struct coglink_track_exception_payload *)payload);

      break;
    }
    case COGLINK_TRACK_STUCK: {
      if (c_info->c_client->events->on_track_stuck) c_info->c_client->events->on_track_stuck((struct coglink_track_stuck_payload *)payload);

      break;
    }
    case COGLINK_WEBSOCKET_CLOSED: {
      if (c_info->c_client->events->on_websocket_closed) c_info->c_client->events->on_websocket_closed((struct coglink_websocket_closed_payload *)payload);

      break;
    }
    case COGLINK_PARSE_ERROR: {
      DEBUG("[coglink] Failed to parse WebSocket data: %s", text);

      break;
    }
  }
}

/* todo: replace for direct events (?) */
enum discord_event_scheduler _coglink_handle_scheduler(struct discord *client, const char data[], size_t length, enum discord_gateway_events event) {
  struct coglink_client *c_client = discord_get_data(client);

  switch (event) {
    case DISCORD_EV_VOICE_STATE_UPDATE: {
      struct coglink_voice_state *voice_state = coglink_parse_voice_state(data, length);

      if (voice_state->channel_id) {
        if (c_client->bot_id == voice_state->user_id) {
          struct coglink_player *player = coglink_get_player(c_client, voice_state->guild_id);

          if (player == NULL) return DISCORD_EVENT_MAIN_THREAD;

          player->voice_data = malloc(sizeof(struct coglink_voice_data));
          /* todo: is it necessary after being sent to the node? */
          player->voice_data->session_id = voice_state->session_id;
        } else {
          size_t i = 0;

          while (c_client->users->size != i) {
            if (c_client->users->array[i].id == 0) {
              c_client->users->array[i].channel_id = voice_state->channel_id;

              return DISCORD_EVENT_MAIN_THREAD;
            }

            i++;
          }

          c_client->users->array = realloc(c_client->users->array, (c_client->users->size + 1) * sizeof(struct coglink_user));
          c_client->users->array[c_client->users->size].id = voice_state->user_id;
          c_client->users->array[c_client->users->size].channel_id = voice_state->channel_id;
          c_client->users->size++;

          return DISCORD_EVENT_IGNORE;
        }
      } else {
        size_t i = 0;

        while (c_client->users->size > i) {
          if (c_client->users->array[i].id == voice_state->user_id) {
            c_client->users->array[i].id = 0;
            c_client->users->array[i].channel_id = 0;

            return DISCORD_EVENT_IGNORE;
          }

          i++;
        }

        return DISCORD_EVENT_IGNORE;
      }

      break;
    }
    case DISCORD_EV_VOICE_SERVER_UPDATE: {
      struct coglink_voice_server_update *voice_server_update = coglink_parse_voice_server_update(data, length);

      struct coglink_player *player = coglink_get_player(c_client, voice_server_update->guild_id);

      if (player == NULL) {
        DEBUG("[coglink] Player not found for guild %" PRIu64 "", voice_server_update->guild_id);

        coglink_free_voice_server_update(voice_server_update);

        return DISCORD_EVENT_IGNORE;
      }

      struct coglink_node *node = &c_client->nodes->array[player->node];

      char url_path[(sizeof("/sessions//players/") - 1) + 16 + 19 + 1]; 
      snprintf(url_path, sizeof(url_path), "/sessions/%s/players/%" PRIu64 "", node->session_id, voice_server_update->guild_id);

      /* todo: is that redudancy needed for readability? */
      size_t payload_size = (
        (sizeof("{"
          "\"voice\":{"
            "\"token\":\"\","
            "\"endpoint\":\"\","
            "\"sessionId\":\"\""
          "}"
        "}") - 1) + strlen(voice_server_update->token) + strlen(voice_server_update->endpoint) + strlen(player->voice_data->session_id) + 1);
      char *payload = malloc(payload_size * sizeof(char));
      snprintf(payload, payload_size,
        "{"
          "\"voice\":{"
            "\"token\":\"%s\","
            "\"endpoint\":\"%s\","
            "\"sessionId\":\"%s\""
          "}"
        "}", voice_server_update->token, voice_server_update->endpoint, player->voice_data->session_id);

      _coglink_perform_request(node, &(struct coglink_request_params) {
        .endpoint = url_path,
        .method = "PATCH",
        .body = payload,
        .body_length = payload_size - 1,
        .get_response = false
      }, NULL);

      free(payload);

      coglink_free_voice_server_update(voice_server_update);

      break;
    }
    default: {
      return DISCORD_EVENT_MAIN_THREAD;
    }
  }

  return DISCORD_EVENT_MAIN_THREAD;
}

int coglink_connect_nodes(struct coglink_client *c_client, struct discord *client, struct coglink_nodes *nodes) {
  /* Initialize TableC */
  tablec_init(&coglink_hashtable, 16);

  /* libcurl should be already initialized by Concord */
  
  c_client->nodes = nodes;
  c_client->players = malloc(sizeof(struct coglink_players));
  c_client->players->array = malloc(sizeof(struct coglink_player));
  c_client->players->size = 0;

  c_client->users = malloc(sizeof(struct coglink_users));
  c_client->users->array = malloc(sizeof(struct coglink_user));
  c_client->users->size = 0;

  char bot_id_str[32];
  snprintf(bot_id_str, sizeof(bot_id_str), "%" PRIu64, c_client->bot_id);

  for (size_t i = 0;i <= nodes->size - 1;i++) {
    struct coglink_node *node_info = &c_client->nodes->array[i];

    char hostname[128 + 19 + 4 + 1];
    snprintf(hostname, sizeof(hostname), "ws%s://%s%s%d/v4/websocket", node_info->ssl ? "s" : "", node_info->hostname, node_info->port ? ":" : "", node_info->port);

    node_info->mhandle = curl_multi_init();
    node_info->tstamp = 0;

    struct _coglink_websocket_data *c_info = malloc(sizeof(struct _coglink_websocket_data));
    c_info->c_client = c_client;
    c_info->node_id = i;

    struct ws_callbacks callbacks = {
      .data = c_info,
      // .on_connect = _ws_on_connect,
      .on_text = _ws_on_text,
      .on_close = _ws_on_close 
    };

    nodes->array[i].ws = ws_init(&callbacks, client->gw.mhandle, NULL);

    /* todo: use libcurl (official) websocket client */
    ws_set_url(node_info->ws, hostname, NULL);
    ws_start(node_info->ws);

    if (c_client->allow_resuming && c_client->nodes != NULL) ws_add_header(node_info->ws, "Session-Id", node_info->session_id);
    ws_add_header(node_info->ws, "Authorization", node_info->password);
    ws_add_header(node_info->ws, "Num-Shards", c_client->num_shards);
    ws_add_header(node_info->ws, "User-Id", bot_id_str);
    /* NodeLink/FrequenC Client-Name format */
    ws_add_header(node_info->ws, "Client-Name", "Coglink/3.0.0 (https://github.com/PerformanC/CogLink)");
    ws_add_header(node_info->ws, "Sec-WebSocket-Protocol", "13"); /* If not set, will be undefined */

    io_poller_curlm_add(client->io_poller, node_info->mhandle, _IO_poller, node_info);
  }

  discord_set_data(client, c_client);
  discord_set_event_scheduler(client, _coglink_handle_scheduler);

  return COGLINK_SUCCESS;
}
