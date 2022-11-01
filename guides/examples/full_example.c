#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>

#include <coglink/lavalink.h>
#include <coglink/rest-lavalink.h>
#include <coglink/player.h>
#include <coglink/definations.h>

#define VOICE_ID 123456789012345678
int onRaw(struct lavaInfo *lavaInfo, const char *text, size_t length) {
  log_info("RECEIVED FROM LAVALINK: %s", text);
  return COGLINK_PROCEED; // Let's allow coglink handle it, here, we say to Coglink that it can proceed handling it.
}

void on_ready(struct discord *client, const struct discord_ready *bot) {
  log_trace("[DISCORD_GATEWAY] Logged in as %s#%s!", bot->user->username, bot->user->discriminator);
}

void onConnect(void) {
  log_info("[COGLINK] Lavalink connected");
}

void onClose(enum ws_close_reason wscode, const char *reason) {
  log_info("[COGLINK] Lavalink closed. [%d/%s]", wscode, reason);
}

void onTrackStart(char *track, u64snowflake guildId) {
  log_info("[COGLINK] Track started. [%s/%"PRIu64"]", track, guildId);
}

void onTrackEnd(char *reason, char *track, u64snowflake guildId) {
  log_info("[COGLINK] Track ended. [%s/%s/%"PRIu64"]", reason, track, guildId);
}

void onTrackException(char *track, char *message, char *severity, char *cause, u64snowflake guildId) {
  log_error("[COGLINK] Track exception. [%s/%s/%s/%s/%"PRIu64"]", track, message, severity, cause, guildId);
}

void onTrackStuck(char *track, int thresholdMs, u64snowflake guildId) {
  log_error("[COGLINK] Track stuck. [%s/%d/%"PRIu64"]", track, thresholdMs, guildId);
}

void onWebSocketClosed(int code, char *reason, char *byRemote, u64snowflake guildId) {
  log_error("[COGLINK] Websocket closed. [%d/%s/%s/%"PRIu64"]", code, reason, byRemote, guildId);
}

void onUnknownEvent(char *type, const char *text, u64snowflake guildId) {
  log_error("[COGLINK] Unknown event. [%s/%s/%"PRIu64"]", type, text, guildId);
}

void onStats(int playingPlayers, struct lavaMemory *infoMemory, int players, struct lavaFStats *infoFrameStats, struct lavaCPU *infoCPU, int uptime) {
  printf("InfoMemory:\n  > Free: %s\n  > Used: %s\n  > Reservable: %s\n", infoMemory->free, infoMemory->used, infoMemory->reservable);
  printf("InfoCPU:\n  > Cores: %s\n  > SystemLoad: %s\n  > LavalinkLoad: %s\n", infoCPU->cores, infoCPU->systemLoad, infoCPU->lavalinkLoad);
  if (infoFrameStats->sent) printf("InfoFrameStats:\n  > Sent: %s\n  > Nulled: %s\n  > Deficit: %s\n", infoFrameStats->sent, infoFrameStats->nulled, infoFrameStats->deficit);
  printf("PlayingPlayers; %d\nPlayers: %d\nUptime: %d\n", playingPlayers, players, uptime);
}

void onPlayerUpdate(int time, int position, char *connected, int ping, u64snowflake guildId) {
  printf("TIME: %d/ POSITION: %d/ CONNECTED: %s/ PING: %d/ GUILDID: %"PRIu64"\n", time, position, connected, ping, guildId);
}

void onUnknownOp(char *op, const char *text) {
  log_error("[COGLINK] Unknown OP. [%s/%s]", op, text);
}


struct lavaInfo lavaInfo = {
  .events = 
    &(struct lavaEvents) {
      .onRaw = &onRaw,

      .onConnect = &onConnect,
      .onClose = &onClose,

      .onTrackStart = &onTrackStart,
      .onTrackEnd = &onTrackEnd,
      .onTrackException = &onTrackException,
      .onTrackStuck = &onTrackStuck,
      .onWebSocketClosed = &onWebSocketClosed,
      .onUnknownEvent = &onUnknownEvent,

      .onStats = &onStats,
      .onPlayerUpdate = &onPlayerUpdate,
      .onUnknownOp = &onUnknownOp
    },
  .debug = 1 // 1 = true, 0 = false
};

void on_cycle(struct discord *client) {
  coglink_wsLoop(&lavaInfo);
}

enum discord_event_scheduler scheduler(struct discord *client, const char data[], size_t size, enum discord_gateway_events event) {
  return coglink_handleScheduler(&lavaInfo, client, data, size, event);
}

void on_message(struct discord *client, const struct discord_message *message) {
  if (message->author->bot) return;
  if (0 == strncmp(".play ", message->content, 6)) {
    char *songName = message->content + 6;
    coglink_joinVoiceChannel(&lavaInfo, client, VOICE_ID, message->guild_id);

    struct httpRequest res;
    coglink_searchSong(&lavaInfo, songName, &res);

    struct lavaSong *song;
    
    coglink_parseSearch(&lavaInfo, res, "0", &song);

    coglink_playSong(&lavaInfo, song->track, message->guild_id);

    coglink_parseCleanup(song);
    coglink_searchCleanup(res);
  }
  if (0 == strcmp(".stop", message->content)) {
    coglink_stopPlayer(&lavaInfo, message->guild_id);
  }
  if (0 == strcmp(".pause", message->content)) {
    coglink_pausePlayer(&lavaInfo, message->guild_id, "true");
  }
  if (0 == strcmp(".resume", message->content)) {
    coglink_pausePlayer(&lavaInfo, message->guild_id, "false");
  }
  if (0 == strncmp(".seek ", message->content, 6)) {
    char *seek = message->content + 6;

    coglink_seekTrack(&lavaInfo, message->guild_id, seek);
  }
  if (0 == strncmp(".volume ", message->content, 8)) {
    char *volume = message->content + 8;

    coglink_setPlayerVolume(&lavaInfo, message->guild_id, volume);
  }
  if (0 == strcmp(".destroy", message->content)) {
    coglink_destroyPlayer(&lavaInfo, message->guild_id);
  }
  if (0 == strcmp(".closeNode", message->content)) {
    coglink_disconnectNode(&lavaInfo);
  }
}

int main(void) {
  struct discord *client = discord_config_init("config.json");

  struct lavaNode params = {
    .name = "Node1",
    .hostname = "Node hostname",
    .password = "youshallnotpass",
    .shards = "1",
    .botId = "123456789012345678",
    .ssl = 0
  };

  coglink_connectNode(&lavaInfo, &params);

  discord_set_on_ready(client, &on_ready);
  discord_set_on_cycle(client, &on_cycle);
  discord_set_event_scheduler(client, &scheduler);
  discord_set_on_message_create(client, &on_message);

  discord_add_intents(client, DISCORD_GATEWAY_GUILD_VOICE_STATES);
  discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);

  discord_run(client);

  coglink_connectNodeCleanup(&lavaInfo);
}