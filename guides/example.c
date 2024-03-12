#include <stdio.h>
#include <string.h>

#include <coglink/lavalink.h>

#include <concord/discord.h>

void on_ready(struct discord *client, const struct discord_ready *bot) {
  (void)client;

  log_trace("[DISCORD_GATEWAY] Logged in as %s#%s!", bot->user->username, bot->user->discriminator);
}

// int onRaw(struct coglink_lavaInfo *lavaInfo, const char *text, size_t length) {
//   (void)lavaInfo, (void)length;

//   log_debug("[COGLINK] On Raw event: %s", text);

//   return COGLINK_PROCEED; // Let's allow coglink handle it, here, we say to Coglink that it can proceed handling it.
// }

void on_coglink_ready(struct coglink_ready_payload *ready) {
  log_info("[COGLINK] Node connected [%s]", ready->session_id);
}

// void onClose(enum ws_close_reason wscode, const char *reason) {
//   log_info("[COGLINK] Node closed. [%d/%s]", wscode, reason);
// }

// void onTrackStart(char *guildId, struct coglink_parsedTrack *track) {
//   log_info("[COGLINK] Track started. [%s/%s]", guildId, track->title);
// }

// void onTrackEnd(char *guildId, struct coglink_parsedTrack *track, char *reason) {
//   log_info("[COGLINK] Track ended. [%s/%s/%s]", guildId, track->title, reason);
// }

// void onTrackException(char *guildId, struct coglink_parsedTrack *track, char *message, char *severity, char *cause) {
//   log_error("[COGLINK] Track exception. [%s/%s/%s/%s/%s]", guildId, track->title, message, severity, cause);
// }

// void onTrackStuck(char *guildId, char *thresholdMs, struct coglink_parsedTrack *track) {
//   log_error("[COGLINK] Track stuck. [%s/%d/%s]", guildId, thresholdMs, track->title);
// }

// void onWebSocketClosed(char *guildId, char *code, char *reason, int byRemote) {
//   log_error("[COGLINK] Websocket closed. [%s/%s/%s/%d]", guildId, code, reason, byRemote);
// }

// void onUnknownEvent(char *guildId, char *type, const char *text) {
//   log_error("[COGLINK] Unknown event. [%s/%s/%s]", guildId, type, text);
// }

void onStats(struct coglink_stats_payload *stats) {
  printf("InfoMemory:\n  > Free: %d\n  > Used: %d\n  > Reservable: %d\n", stats->memory->free, stats->memory->used, stats->memory->reservable);
  printf("InfoCPU:\n  > Cores: %d\n  > SystemLoad: %d\n  > LavalinkLoad: %d\n", stats->cpu->cores, stats->cpu->systemLoad, stats->cpu->lavalinkLoad);
  if (stats->frameStats) printf("InfoFrameStats:\n  > Sent: %d\n  > Nulled: %d\n  > Deficit: %d\n", stats->frameStats->sent, stats->frameStats->nulled, stats->frameStats->deficit);
  printf("PlayingPlayers: %d\nPlayers: %d\nUptime: %d\n", stats->playingPlayers, stats->players, stats->uptime);
}

// void onPlayerUpdate(char *guildId, char *time, char *position, int connected, char *ping) {
//   log_trace("[COGLINK] GuildId: %s\n Time: %s\n Position: %s\n Connected: %d\n Ping: %s", guildId, time, position, connected, ping);
// }

// void onUnknownOp(char *op, const char *text) {
//   log_error("[COGLINK] Unknown OP. [%s/%s]", op, text);
// }

void on_message(struct discord *client, const struct discord_message *message) {
  if (message->author->bot) return;

  // if (0 == strncmp(".play ", message->content, 6)) {
  //   char *songName = message->content + 6;

  //   if (songName[0] == '\0') {
  //     struct discord_embed embed[] = {
  //       {
  //         .description = "Sorry, you must put a music name after the command.",
  //         .footer =
  //           &(struct discord_embed_footer){
  //             .text = "Powered by Coglink and Concord",
  //             .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
  //           },
  //         .timestamp = discord_timestamp(client),
  //         .color = 16711680
  //       }
  //     };

  //     struct discord_create_message params = {
  //       .flags = 0,
  //       .embeds =
  //         &(struct discord_embeds){
  //           .size = 1,
  //           .array = embed,
  //         },
  //     };

  //     discord_create_message(client, message->channel_id, &params, NULL);
  //     return;
  //   }

  //   struct coglink_requestInformation res;
  //   coglink_searchSong(&lavaInfo, songName, &res);

  //   struct coglink_parsedTrackStruct parsedStruct;
  //   coglink_initParseTrack(&lavaInfo, &parsedStruct, &res);

  //   int loadType;
  //   coglink_parseLoadtype(&lavaInfo, &parsedStruct, &res, &loadType);

  //   switch(loadType) {
  //     case COGLINK_LOADTYPE_SEARCH_RESULT: {
  //       struct coglink_parsedTrack song;

  //       coglink_parseTrack(&lavaInfo, &parsedStruct, &res, "0", &song);
  //       coglink_playSong(&lavaInfo, song.encoded, message->guild_id);

  //       char description[256];
  //       snprintf(description, sizeof(description), "Now playing `%s` from `%s`", song.title, song.author);

  //       struct discord_embed embed[] = {
  //         {
  //           .title = song.title,
  //           .url = song.uri,
  //           .description = description,
  //           .footer =
  //             &(struct discord_embed_footer){
  //               .text = "Powered by Coglink and Concord",
  //               .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
  //           },
  //           .timestamp = discord_timestamp(client),
  //           .color = 15615
  //         }
  //       };

  //       struct discord_create_message params = {
  //         .flags = 0,
  //         .embeds =
  //           &(struct discord_embeds){
  //             .size = 1,
  //             .array = embed,
  //           },
  //       };

  //       discord_create_message(client, message->channel_id, &params, NULL);
  //       break;
  //     }
  //     case COGLINK_LOADTYPE_EMPTY: {
  //       struct discord_embed embed[] = {
  //         {
  //           .description = "Hmmm... The node was unable to find that music. :/",
  //           .footer =
  //             &(struct discord_embed_footer){
  //               .text = "Powered by Coglink and Concord",
  //               .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
  //             },
  //           .timestamp = discord_timestamp(client),
  //           .color = 16711680
  //         }
  //       };

  //       struct discord_create_message params = {
  //         .flags = 0,
  //         .embeds =
  //           &(struct discord_embeds){
  //             .size = 1,
  //             .array = embed,
  //           },
  //       };

  //       discord_create_message(client, message->channel_id, &params, NULL);
  //       break;
  //     }
  //     case COGLINK_LOADTYPE_LOAD_FAILED: { 
  //       struct discord_embed embed[] = {
  //         {
  //           .description = "Oop- Some wild error happened while searching the music, what so ever, it is not a Coglink error, but a node one, phew!",
  //           .footer =
  //             &(struct discord_embed_footer){
  //               .text = "Powered by Coglink and Concord",
  //               .icon_url = "https://raw.githubusercontent.com/Cogmasters/concord/master/docs/static/concord-small.png",
  //             },
  //           .timestamp = discord_timestamp(client),
  //           .color = 16711680
  //         }
  //       };

  //       struct discord_create_message params = {
  //         .flags = 0,
  //         .embeds =
  //           &(struct discord_embeds){
  //             .size = 1,
  //             .array = embed,
  //           },
  //       };

  //       discord_create_message(client, message->channel_id, &params, NULL);
  //       break;
  //     }
  //   }

  //   coglink_joinUserVoiceChannel(&lavaInfo, client, message->author->id, message->guild_id);

  //   coglink_searchSongCleanup(&res);
  // }
  // if (0 == strcmp(".stop", message->content)) {
  //   coglink_stopPlayer(&lavaInfo, message->guild_id);
  // }
  // if (0 == strcmp(".pause", message->content)) {
  //   coglink_pausePlayer(&lavaInfo, message->guild_id, "true");
  // }
  // if (0 == strcmp(".resume", message->content)) {
  //   coglink_pausePlayer(&lavaInfo, message->guild_id, "false");
  // }
  // if (0 == strncmp(".seek ", message->content, 6)) {
  //   char *seek = message->content + 6;

  //   coglink_seekTrack(&lavaInfo, message->guild_id, seek);
  // }
  // if (0 == strncmp(".volume ", message->content, 8)) {
  //   char *volume = message->content + 8;

  //   coglink_setPlayerVolume(&lavaInfo, message->guild_id, volume);
  // }
  // if (0 == strcmp(message->content, ".8d")) {
  //   coglink_setEffect(&lavaInfo, message->guild_id, COGLINK_FILTER_ROTATION, "0.2");
  // }
  // if (0 == strcmp(".destroy", message->content)) {
  //   coglink_destroyPlayer(&lavaInfo, message->guild_id);
  // }
  // if (0 == strcmp(".closeNode", message->content)) {
  //   coglink_disconnectNode(&lavaInfo, 0);
  // }
  // if (0 == strcmp(".getinfo", message->content)) {
  //   struct coglink_requestInformation res;

  //   coglink_getLavalinkInfo(&lavaInfo, message->guild_id, &res);

  //   log_debug("[COGLING] Lavalink Info: %s", res.body);
  // }
  // if (0 == strcmp(".getrouter", message->content)) {
  //   struct coglink_requestInformation res;

  //   coglink_getRouterPlanner(&lavaInfo, message->guild_id, &res);

  //   log_debug("[COGLING] Router planner: %s", res.body);
  // }
}

int main(void) {
  struct discord *client = discord_config_init("config.json");

  struct coglink_client c_client = {
    .bot_id = "1038648927693590619",
    .events = &(struct coglink_events){
      // .onRaw = &onRaw,
      .onReady = &on_coglink_ready,
      .onStats = &onStats
    },
    .num_shards = "1",
  };

  struct coglink_nodes nodes = {
    .array = (struct coglink_node[]){
      {
        .name = "Node1",
        .hostname = "127.0.0.1",
        .port = 2333,
        .password = "youshallnotpass",
        .ssl = false
      }
    },
    .size = 1
  };

  coglink_connect_nodes(&c_client, client, &nodes);

  /* Or to set events, you can also use */
  // coglink_setEvents(&lavaInfo, &lavaInfo->events);

  discord_set_on_ready(client, &on_ready);
  discord_set_on_message_create(client, &on_message);

  discord_add_intents(client, DISCORD_GATEWAY_GUILD_VOICE_STATES);
  discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);

  discord_run(client);

  // coglink_connectNodeCleanup(&lavaInfo, client);
}