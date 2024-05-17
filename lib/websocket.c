#include <stdio.h>
#include <string.h>
#include <pthread.h>

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

struct _coglink_websocket_data {
  struct coglink_client *c_client;
  size_t node_id;
};

struct tablec_ht coglink_hashtable;

int _IO_poller(struct io_poller *io, CURLM *multi, void *data) {
  (void) io; (void) multi;
  struct coglink_node *node = data;

  return !ws_multi_socket_run(node->ws, &node->tstamp) ? CCORD_DISCORD_CONNECTION : CCORD_OK;
}

void _ws_on_connect(void *data, struct websockets *ws, struct ws_info *info, const char *protocols) {
  (void)ws; (void)info; (void)protocols;

  struct _coglink_websocket_data *c_info = data;

  if (c_info->c_client->events->on_connect == NULL) return;

  c_info->c_client->events->on_connect(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id]);
}

void _ws_on_close(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t length) {
  (void)ws; (void)info; (void)length;

  struct _coglink_websocket_data *c_info = data;

  if (c_info->c_client->events->on_close == NULL) return;

  c_info->c_client->events->on_close(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], wscode, reason);
}

void _ws_on_text(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t length) {
  (void) ws; (void) info;
  struct _coglink_websocket_data *c_info = data;

  if (c_info->c_client->events->on_raw && c_info->c_client->events->on_raw(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], text, length) != COGLINK_PROCEED) return;

  int type;
  void *payload;
  int status = coglink_parse_websocket_data(text, length, &payload, &type);

  if (status < 0) {
    DEBUG("[coglink] Failed to parse WebSocket data: %s", text);

    return;
  }

  switch (type) {
    case COGLINK_READY: {
      struct coglink_ready *ready = payload;

      c_info->c_client->nodes->array[c_info->node_id].session_id = ready->session_id;

      if (c_info->c_client->events->on_ready) c_info->c_client->events->on_ready(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], ready);

      break;
    }
    case COGLINK_PLAYER_UPDATE: {
      struct coglink_player_update *player_update = payload;

      if (c_info->c_client->events->on_player_update) c_info->c_client->events->on_player_update(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], player_update);

      free(player_update->state);

      break;
    }
    case COGLINK_STATS: {
      struct coglink_stats *stats = payload;

      if (c_info->c_client->events->on_stats) c_info->c_client->events->on_stats(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], stats);

      free(stats->memory);
      free(stats->cpu);
      free(stats->frameStats);

      break;
    }
    case COGLINK_TRACK_START: {
      struct coglink_track_start *track_start = payload;

      if (c_info->c_client->events->on_track_start) c_info->c_client->events->on_track_start(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], track_start);

      coglink_free_track(track_start->track);

      break;
    }
    case COGLINK_TRACK_END: {
      struct coglink_track_end *track_end = payload;

      struct coglink_player *player = coglink_get_player(c_info->c_client, track_end->guildId);

      if (player == NULL) {
        DEBUG("[coglink] Player not found for guild %" PRIu64 "", track_end->guildId);

        coglink_free_track(track_end->track);
        free(track_end);

        return;
      }

      struct coglink_player_queue *queue = coglink_get_player_queue(c_info->c_client, player);

      if (queue->size != 0 && (track_end->reason == COGLINK_TRACK_END_REASON_FINISHED || track_end->reason == COGLINK_TRACK_END_REASON_LOAD_FAILED)) {
        coglink_remove_track_from_queue(c_info->c_client, player, 0);

        if (queue->size != 0) {
          struct coglink_update_player_params data = {
            .track = &(struct coglink_update_player_track_params) {
              .encoded = queue->array[0]
            }
          };

          coglink_update_player(c_info->c_client, player, &data, NULL);
        }
      }

      if (c_info->c_client->events->on_track_end) c_info->c_client->events->on_track_end(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], track_end);

      coglink_free_track(track_end->track);

      break;
    }
    case COGLINK_TRACK_EXCEPTION: {
      struct coglink_track_exception *track_exception = payload;

      if (c_info->c_client->events->on_track_excetion) c_info->c_client->events->on_track_excetion(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], track_exception);

      coglink_free_track(track_exception->track);
      free(track_exception->exception->cause);
      free(track_exception->exception->message);
      free(track_exception->exception);

      break;
    }
    case COGLINK_TRACK_STUCK: {
      struct coglink_track_stuck *track_stuck = (struct coglink_track_stuck *)payload;

      if (c_info->c_client->events->on_track_stuck) c_info->c_client->events->on_track_stuck(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], track_stuck);

      coglink_free_track(track_stuck->track);

      break;
    }
    case COGLINK_WEBSOCKET_CLOSED: {
      struct coglink_websocket_closed *websocket_closed = payload;

      if (c_info->c_client->events->on_websocket_closed) c_info->c_client->events->on_websocket_closed(c_info->c_client, &c_info->c_client->nodes->array[c_info->node_id], websocket_closed);

      free(websocket_closed->reason);

      break;
    }
  }

  free(payload);
}

enum discord_event_scheduler _coglink_handle_scheduler(struct discord *client, const char data[], size_t length, enum discord_gateway_events event) {
  struct coglink_client *c_client = discord_get_data(client);

  printf("Event: %d\n", event);

  switch (event) {
    case DISCORD_EV_VOICE_STATE_UPDATE: {
      struct coglink_voice_state voice_state = { 0 };
      
      if (coglink_parse_voice_state(data, length, &voice_state) == COGLINK_FAILED) return DISCORD_EVENT_MAIN_THREAD;

      if (voice_state.channel_id) {
        if (c_client->bot_id == voice_state.user_id) {
          struct coglink_player *player = coglink_get_player(c_client, voice_state.guild_id);

          if (player == NULL) break;

          if (player->voice_data != NULL) {
            free(player->voice_data->session_id);
            free(player->voice_data);
          }

          player->voice_data = malloc(sizeof(struct coglink_player_voice_data));
          player->voice_data->session_id = voice_state.session_id;
        } else {
          size_t i = 0;

          while (c_client->users->size != i) {
            if (c_client->users->array[i].id == 0) {
              c_client->users->array[i].channel_id = voice_state.channel_id;

              break;
            }

            i++;
          }

          c_client->users->array = realloc(c_client->users->array, (c_client->users->size + 1) * sizeof(struct coglink_user));
          c_client->users->array[c_client->users->size].id = voice_state.user_id;
          c_client->users->array[c_client->users->size].channel_id = voice_state.channel_id;
          c_client->users->size++;
        }
      } else {
        size_t i = 0;

        while (c_client->users->size > i) {
          if (c_client->users->array[i].id == voice_state.user_id) {
            c_client->users->array[i].id = 0;
            c_client->users->array[i].channel_id = 0;

            return DISCORD_EVENT_MAIN_THREAD;
          }

          i++;
        }
      }

      break;
    }
    case DISCORD_EV_VOICE_SERVER_UPDATE: {
      struct coglink_voice_server_update voice_server_update = { 0 };
      
      if (coglink_parse_voice_server_update(data, length, &voice_server_update) == COGLINK_FAILED) return DISCORD_EVENT_MAIN_THREAD;

      struct coglink_player *player = coglink_get_player(c_client, voice_server_update.guild_id);

      if (player == NULL) {
        DEBUG("[coglink] Player not found for guild %" PRIu64 "", voice_server_update.guild_id);

        coglink_free_voice_server_update(&voice_server_update);

        return DISCORD_EVENT_IGNORE;
      }

      struct coglink_node *node = coglink_get_player_node(c_client, player);

      char url_path[(sizeof("/sessions//players/") - 1) + 16 + 19 + 1]; 
      snprintf(url_path, sizeof(url_path), "/sessions/%s/players/%" PRIu64 "", node->session_id, voice_server_update.guild_id);

      size_t payload_size = (51 + strlen(voice_server_update.token) + strlen(voice_server_update.endpoint) + strlen(player->voice_data->session_id) + 1);
      char *payload = malloc(payload_size * sizeof(char));
      snprintf(payload, payload_size * sizeof(char),
        "{"
          "\"voice\":{"
            "\"token\":\"%s\","
            "\"endpoint\":\"%s\","
            "\"sessionId\":\"%s\""
          "}"
        "}", voice_server_update.token, voice_server_update.endpoint, player->voice_data->session_id);

      _coglink_perform_request(node, &(struct coglink_request_params) {
        .endpoint = url_path,
        .method = "PATCH",
        .body = payload,
        .body_length = payload_size - 1,
        .get_response = false
      }, NULL);

      free(payload);

      free(player->voice_data->session_id);
      free(player->voice_data);
      player->voice_data = NULL;

      coglink_free_voice_server_update(&voice_server_update);

      break;
    }
    case DISCORD_EV_GUILD_CREATE: {
      struct coglink_guild_create guild_create;

      if (coglink_parse_guild_create(data, length, &guild_create) == COGLINK_FAILED) return DISCORD_EVENT_MAIN_THREAD;

      FIND_FIELD(guild_create.pairs, data, voice_states, "voice_states");

      for (int i = 0; i < voice_states->size; i++) {
        char i_str[16];
        snprintf(i_str, sizeof(i_str), "%d", i);

        struct coglink_single_user_guild_create single_guild_create;
        int status = coglink_parse_single_user_guild_create(guild_create.pairs, data, i_str, c_client->bot_id, &single_guild_create);

        if (status == COGLINK_FAILED) continue;

        if (single_guild_create.type == 1) {
          struct coglink_player *player = coglink_get_player(c_client, guild_create.guild_id);

          if (player == NULL) continue;

          if (player->voice_data != NULL) {
            free(player->voice_data->session_id);
            free(player->voice_data);
          }

          player->voice_data = malloc(sizeof(struct coglink_player_voice_data));
          player->voice_data->session_id = single_guild_create.session_id;
        } else {
          size_t i = 0;

          do {
            if (c_client->users->array[i].id == 0) {
              c_client->users->array[i].id = single_guild_create.user_id;
              c_client->users->array[i].channel_id = single_guild_create.vc_id;

              continue;
            }

            i++;
          } while (c_client->users->size != i);

          c_client->users->array = realloc(c_client->users->array, (c_client->users->size + 1) * sizeof(struct coglink_user));
          c_client->users->array[c_client->users->size].id = single_guild_create.user_id;
          c_client->users->array[c_client->users->size].channel_id = single_guild_create.vc_id;
          c_client->users->size++;

          continue;
        }
      }

      coglink_free_guild_create(&guild_create);

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
  c_client->players->array = calloc(sizeof(struct coglink_player), 1);
  c_client->players->size = 0;

  c_client->users = malloc(sizeof(struct coglink_users));
  c_client->users->array = calloc(sizeof(struct coglink_user), 1);
  c_client->users->size = 0;

  char bot_id_str[32];
  snprintf(bot_id_str, sizeof(bot_id_str), "%" PRIu64, c_client->bot_id);

  for (size_t i = 0;i <= nodes->size - 1;i++) {
    struct coglink_node *node_info = &c_client->nodes->array[i];

    char hostname[128 + 19 + 4 + 1];
    snprintf(hostname, sizeof(hostname), "ws%s://%s%s%d/v4/websocket", nodes->array[i].ssl ? "s" : "", nodes->array[i].hostname, nodes->array[i].port ? ":" : "", nodes->array[i].port);

    nodes->array[i].mhandle = curl_multi_init();
    nodes->array[i].tstamp = 0;

    nodes->array[i].ws_data = malloc(sizeof(struct _coglink_websocket_data));
    nodes->array[i].ws_data->c_client = c_client;
    nodes->array[i].ws_data->node_id = i;

    struct ws_callbacks callbacks = {
      .data = nodes->array[i].ws_data,
      .on_connect = _ws_on_connect,
      .on_text = _ws_on_text,
      .on_close = _ws_on_close 
    };

    struct ws_attr attr = {
      .conf = &client->gw.conf
    };

    nodes->array[i].ws = ws_init(&callbacks, client->gw.mhandle, &attr);

    /* todo: use libcurl (official) websocket client */
    ws_set_url(nodes->array[i].ws, hostname, NULL);
    ws_start(nodes->array[i].ws);

    if (c_client->allow_resuming && c_client->nodes != NULL) ws_add_header(nodes->array[i].ws, "Session-Id", nodes->array[i].session_id);
    ws_add_header(nodes->array[i].ws, "Authorization", nodes->array[i].password);
    ws_add_header(nodes->array[i].ws, "Num-Shards", c_client->num_shards);
    ws_add_header(nodes->array[i].ws, "User-Id", bot_id_str);
    /* NodeLink/FrequenC Client-Name format */
    ws_add_header(nodes->array[i].ws, "Client-Name", "Coglink/3.0.1 (https://github.com/PerformanC/CogLink)");
    ws_add_header(nodes->array[i].ws, "Sec-WebSocket-Protocol", "13"); /* If not set, will be undefined */

    io_poller_curlm_add(client->io_poller, nodes->array[i].mhandle, _IO_poller, node_info);
  }

  discord_set_data(client, c_client);
  discord_set_event_scheduler(client, _coglink_handle_scheduler);

  return COGLINK_SUCCESS;
}

void coglink_cleanup(struct coglink_client *c_client) {
  for (size_t i = 0;i < c_client->nodes->size;i++) {
    free(c_client->nodes->array[i].session_id);
    ws_cleanup(c_client->nodes->array[i].ws);
    curl_multi_cleanup(c_client->nodes->array[i].mhandle);

    free(c_client->nodes->array[i].ws_data);
  }

  tablec_cleanup(&coglink_hashtable);

  for (size_t i = 0;i < c_client->players->size;i++) {
    if (c_client->players->array[i].voice_data != NULL) {
      free(c_client->players->array[i].voice_data->session_id);
      free(c_client->players->array[i].voice_data);
    }

    if (c_client->players->array[i].queue->array != NULL) {
      for (size_t j = 0;j < c_client->players->array[i].queue->size;j++) {
        free(c_client->players->array[i].queue->array[j]);
      }

      free(c_client->players->array[i].queue->array);
    }

    free(c_client->players->array[i].queue);
  }

  free(c_client->players->array);
  free(c_client->players);

  free(c_client->users->array);
  free(c_client->users);
}