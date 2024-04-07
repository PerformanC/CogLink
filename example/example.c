#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <coglink/lavalink.h>

#include <concord/discord.h>

struct coglink_client *c_client;

void handle_sigint(int signum) {
  (void)signum;

  ccord_shutdown_async();
}

void on_ready(struct discord *client, const struct discord_ready *bot) {
  (void)client;

  log_trace("[DISCORD_GATEWAY] Logged in as %s#%s!", bot->user->username, bot->user->discriminator);
}

void on_coglink_ready(struct coglink_client *client, struct coglink_node *node, struct coglink_ready *ready) {
  (void)client; (void)node;

  log_info("[COGLINK] Node connected [%s]", ready->session_id);
}

void on_coglink_stats(struct coglink_client *client, struct coglink_node *node, struct coglink_stats *stats) {
  (void)client; (void)node;
  
  printf("InfoMemory:\n  > Free: %d\n  > Used: %d\n  > Reservable: %d\n", stats->memory->free, stats->memory->used, stats->memory->reservable);
  printf("InfoCPU:\n  > Cores: %d\n  > SystemLoad: %d\n  > LavalinkLoad: %d\n", stats->cpu->cores, stats->cpu->systemLoad, stats->cpu->lavalinkLoad);
  if (stats->frameStats) printf("InfoFrameStats:\n  > Sent: %d\n  > Nulled: %d\n  > Deficit: %d\n", stats->frameStats->sent, stats->frameStats->nulled, stats->frameStats->deficit);
  printf("PlayingPlayers: %d\nPlayers: %d\nUptime: %d\n", stats->playingPlayers, stats->players, stats->uptime);
}

void on_message(struct discord *client, const struct discord_message *message) {
  if (message->author->bot) return;

  if (0 == strncmp(".play ", message->content, sizeof(".play ") - 1)) {
    char *songName = message->content + sizeof(".play ") - 1;

    if (songName[0] == '\0') {
      struct discord_embed embed[] = {
        {
          .description = "Sorry, you must put a music name after the command.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
          .timestamp = discord_timestamp(client),
          .color = 16711680
        }
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);
      return;
    }

    struct coglink_player *player = coglink_create_player(c_client, message->guild_id);

    if (!player) {
      struct discord_embed embed[] = {
        {
          .description = "Sorry, however NodeLink is not available at the moment.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
        }
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);

      return;
    }

    struct coglink_user *user = coglink_get_user(c_client, message->author->id);

    if (user == NULL) {
      struct discord_embed embed[] = {
        {
          .description = "You are not in a voice channel.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
        }
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);

      return;
    }

    coglink_join_voice_channel(c_client, client, message->guild_id, user->channel_id);

    CURL *curl = curl_easy_init();

    if (!curl) {
      struct discord_embed embed[] = {
        {
          .description = "Failed to initialize cURL.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
        }
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);

      return;
    }

    char *search = curl_easy_escape(curl, songName, strlen(songName));

    if (!search) {
      struct discord_embed embed[] = {
        {
          .description = "Failed to escape the search query.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
        }
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);

      return;
    }

    char *searchQuery = malloc(strlen(search) + sizeof("ytsearch:") + 1);

    if (!searchQuery) {
      struct discord_embed embed[] = {
        {
          .description = "Failed to allocate memory for the search query.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
        }
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);

      curl_free(search);
      curl_easy_cleanup(curl);

      return;
    }

    if (strncmp(songName, "http://", sizeof("http://") - 1) != 0 && strncmp(songName, "https://", sizeof("https://") - 1) != 0) {
      snprintf(searchQuery, strlen(search) + sizeof("ytsearch:") + 1, "ytsearch:%s", search);
    } else {
      strcpy(searchQuery, search);
    }

    struct coglink_load_tracks response = { 0 };

    int status = coglink_load_tracks(c_client, coglink_get_player_node(c_client, player), searchQuery, &response);

    curl_free(search);
    curl_easy_cleanup(curl);
    free(searchQuery);

    if (status == COGLINK_FAILED) {
      struct discord_embed embed[] = {
        {
          .description = "Failed to load tracks.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
        }
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);

      return;
    }

    switch (response.type) {
      case COGLINK_LOAD_TYPE_TRACK: {
        struct coglink_load_tracks_track *track_response = response.data;

        char description[4000 + 1];
        struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);

        if (queue->size == 0) {
          snprintf(description, sizeof(description),
            "Playing `%s` by `%s`.", track_response->info->title, track_response->info->author
          );

          struct coglink_update_player_params params = {
            .track = &(struct coglink_update_player_track_params) {
              .encoded = track_response->encoded,
            },
          };

          coglink_update_player(c_client, player, &params, NULL);
        } else {
          snprintf(description, sizeof(description),
            "Added `%s` by `%s` to the queue.", track_response->info->title, track_response->info->author
          );
        }

        struct discord_embed embed[] = {
          {
            .description = description,
            .footer =
              &(struct discord_embed_footer){
                .text = "Powered by Coglink v3, Concord and NodeLink",
                .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
              },
          },
        };

        struct discord_create_message params = {
          .flags = 0,
          .embeds =
            &(struct discord_embeds){
              .size = 1,
              .array = embed,
            },
        };

        discord_create_message(client, message->channel_id, &params, NULL);
        
        coglink_add_track_to_queue(c_client, player, track_response->encoded);

        break;
      }
      case COGLINK_LOAD_TYPE_PLAYLIST: {
        struct coglink_load_tracks_playlist *data = response.data;

        char description[4000 + 1];
        struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);

        if (queue->size == 0) {
          snprintf(description, sizeof(description),
            "Playing a playlist with %" PRIu64 " tracks. Playing firstly `%s` by `%s`", data->tracks->size, data->tracks->array[0]->info->title, data->tracks->array[0]->info->author
          );

          struct coglink_update_player_params params = {
            .track = &(struct coglink_update_player_track_params) {
              .encoded = data->tracks->array[0]->encoded,
            },
          };

          coglink_update_player(c_client, player, &params, NULL);
        } else {
          snprintf(description, sizeof(description),
            "Adding the playlist with %" PRIu64 " tracks to the queue", data->tracks->size
          );
        }

        struct discord_embed embed[] = {
          {
            .description = description,
            .footer =
              &(struct discord_embed_footer){
                .text = "Powered by Coglink v3, Concord and NodeLink",
                .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
              },
          },
        };

        struct discord_create_message params = {
          .flags = 0,
          .embeds =
            &(struct discord_embeds){
              .size = 1,
              .array = embed,
            },
        };

        discord_create_message(client, message->channel_id, &params, NULL);

        for (size_t i = 0; i < data->tracks->size; i++) {
          coglink_add_track_to_queue(c_client, player, data->tracks->array[i]->encoded);
        }

        break;
      }
      case COGLINK_LOAD_TYPE_SEARCH: {
        struct coglink_load_tracks_search *search_response = response.data;

        char description[4000 + 1];
        struct coglink_player_queue *queue = coglink_get_player_queue(c_client, player);

        if (queue->size == 0) {
          snprintf(description, sizeof(description),
            "Playing %s by %s.", search_response->array[0]->info->title, search_response->array[0]->info->author
          );

          struct coglink_update_player_params params = {
            .track = &(struct coglink_update_player_track_params) {
              .encoded = search_response->array[0]->encoded,
            },
            .volume = 100
          };

          coglink_update_player(c_client, player, &params, NULL);
        } else {
          snprintf(description, sizeof(description),
            "Added %s by %s to the queue.", search_response->array[0]->info->title, search_response->array[0]->info->author
          );
        }

        struct discord_embed embed[] = {
          {
            .description = description,
            .footer =
              &(struct discord_embed_footer){
                .text = "Powered by Coglink v3, Concord and NodeLink",
                .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
              },
          },
        };

        struct discord_create_message params = {
          .flags = 0,
          .embeds =
            &(struct discord_embeds){
              .size = 1,
              .array = embed,
            },
        };

        discord_create_message(client, message->channel_id, &params, NULL);
        
        coglink_add_track_to_queue(c_client, player, search_response->array[0]->encoded);

        break;
      }
      case COGLINK_LOAD_TYPE_EMPTY: {
        struct discord_embed embed[] = {
          {
            .description = "No results found.",
            .footer =
              &(struct discord_embed_footer){
                .text = "Powered by Coglink v3, Concord and NodeLink",
                .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
              },
          },
        };

        struct discord_create_message params = {
          .flags = 0,
          .embeds =
            &(struct discord_embeds){
              .size = 1,
              .array = embed,
            },
        };

        discord_create_message(client, message->channel_id, &params, NULL);

        break;
      }
      case COGLINK_LOAD_TYPE_ERROR: {
        struct coglink_load_tracks_error *data = response.data;

        char description[4000 + 1];
        snprintf(description, sizeof(description), "Failed to load. %s", data->message);

        struct discord_embed embed[] = {
          {
            .description = description,
            .footer =
              &(struct discord_embed_footer){
                .text = "Powered by Coglink v3, Concord and NodeLink",
                .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
              },
          },
        };

        struct discord_create_message params = {
          .flags = 0,
          .embeds =
            &(struct discord_embeds){
              .size = 1,
              .array = embed,
            },
        };

        discord_create_message(client, message->channel_id, &params, NULL);

        break;
      }
    }

    coglink_free_load_tracks(&response);
  }

  if (strncmp(".decodetrack ", message->content, sizeof(".decodetrack ") - 1) == 0) {
    char *track = message->content + sizeof(".decodetrack ") - 1;

    struct coglink_player *player = coglink_create_player(c_client, message->guild_id);

    if (!player) {
      struct discord_embed embed[] = {
        {
          .description = "Sorry, however NodeLink is not available at the moment.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
        }
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);

      return;
    }

    struct coglink_track decoded = { 0 };

    struct coglink_node *node = coglink_get_player_node(c_client, player);

    if (coglink_decode_track(c_client, node, track, &decoded) == COGLINK_FAILED) {
      coglink_free_track(&decoded);

      struct discord_embed embed[] = {
        {
          .description = "Failed to decode track.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
        },
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);

      return;
    }

    char description[4000 + 1];
    snprintf(description, sizeof(description), "Title: %s", decoded.info->title);

    struct discord_embed embed[] = {
      {
        .description = description,
        .footer =
          &(struct discord_embed_footer){
            .text = "Powered by Coglink v3, Concord and NodeLink",
            .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
          },
      },
    };

    struct discord_create_message params = {
      .flags = 0,
      .embeds =
        &(struct discord_embeds){
          .size = 1,
          .array = embed,
        },
    };

    discord_create_message(client, message->channel_id, &params, NULL);

    coglink_free_track(&decoded);
  }

  if (strcmp(".version", message->content) == 0) {
    struct coglink_node_version info = { 0 };

    if (coglink_get_node_version(c_client, &c_client->nodes->array[0], &info) != COGLINK_SUCCESS) {
      struct discord_embed embed[] = {
        {
          .description = "Failed to get Node version.",
          .footer =
            &(struct discord_embed_footer){
              .text = "Powered by Coglink v3, Concord and NodeLink",
              .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
            },
        }
      };

      struct discord_create_message params = {
        .flags = 0,
        .embeds =
          &(struct discord_embeds){
            .size = 1,
            .array = embed,
          },
      };

      discord_create_message(client, message->channel_id, &params, NULL);

      return;
    }

    char description[4000 + 1];
    snprintf(description, sizeof(description), "Version: %d", info.major);

    struct discord_embed embed[] = {
      {
        .description = description,
        .footer =
          &(struct discord_embed_footer){
            .text = "Powered by Coglink v3, Concord and NodeLink",
            .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
          },
      },
    };

    struct discord_create_message params = {
      .flags = 0,
      .embeds =
        &(struct discord_embeds){
          .size = 1,
          .array = embed,
        },
    };

    discord_create_message(client, message->channel_id, &params, NULL);

    coglink_free_node_version(&info);
  }
}

int main(void) {
  signal(SIGINT, handle_sigint);

  c_client = malloc(sizeof(struct coglink_client));

  /* Modify the bot_id to your bot's ID */
  c_client->bot_id = 1038648927693590619;
  c_client->num_shards = "1";
  c_client->events = &(struct coglink_events){
    .on_ready = &on_coglink_ready,
    .on_stats = &on_coglink_stats
  };

  struct discord *client = discord_config_init("config.json");

  struct coglink_nodes nodes = {
    .array = (struct coglink_node[]){
      {
        .name = "Node 1",
        .hostname = "127.0.0.1",
        .port = 2333,
        .password = "youshallnotpass",
        .ssl = false
      }
    },
    .size = 1
  };

  coglink_connect_nodes(c_client, client, &nodes);

  discord_set_on_ready(client, &on_ready);
  discord_set_on_message_create(client, &on_message);

  discord_add_intents(client, DISCORD_GATEWAY_GUILD_VOICE_STATES);
  discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);
  discord_add_intents(client, DISCORD_GATEWAY_GUILDS);

  discord_run(client);

  discord_cleanup(client);
  coglink_cleanup(c_client);
  ccord_global_cleanup();
}