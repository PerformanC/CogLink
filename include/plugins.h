/** \file
 * File containing the functions for plugin support.
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include <concord/types.h>
#include <concord/discord.h>
#include <concord/discord-events.h>

struct coglink_lavaInfo;
struct coglink_requestInformation;
struct coglink_parsedTrack;

struct coglink_pluginEvents {
  int (*onSearchRequest)(struct coglink_lavaInfo *lavaInfo, char *song, struct coglink_requestInformation *res);
  int (*onPlayRequest)(struct coglink_lavaInfo *lavaInfo, char *track, u64snowflake guildId);
  int (*onLavalinkPacketReceived)(struct coglink_lavaInfo *lavaInfo, const char *text, size_t length);
  int (*onLavalinkClose)(struct coglink_lavaInfo *lavaInfo, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t length);
  int (*onCoglinkScheduler)(struct coglink_lavaInfo *lavaInfo, struct discord *client, const char data[], size_t size, enum discord_gateway_events event);
  int (*onDecodeTrackRequest)(struct coglink_lavaInfo *lavaInfo, char *track, struct coglink_requestInformation *res);
  int (*onDecodeTrackParseRequest)(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, struct coglink_parsedTrack *parsedTrackStruct);
  int (*onDecodeTracksRequest)(struct coglink_lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct coglink_requestInformation *res);
  int (*onDecodeTracksParseRequest)(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, char *songPos, struct coglink_parsedTrack *parsedTrackStruct);
};

struct coglink_pluginEventsInternal {
  int (*onSearchRequest[8])(struct coglink_lavaInfo *lavaInfo, char *song, struct coglink_requestInformation *res);
  int (*onPlayRequest[8])(struct coglink_lavaInfo *lavaInfo, char *track, u64snowflake guildId);
  int (*onLavalinkPacketReceived[8])(struct coglink_lavaInfo *lavaInfo, const char *text, size_t length);
  int (*onLavalinkClose[8])(struct coglink_lavaInfo *lavaInfo, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t length);
  int (*onCoglinkScheduler[8])(struct coglink_lavaInfo *lavaInfo, struct discord *client, const char data[], size_t size, enum discord_gateway_events event);
  int (*onDecodeTrackRequest[8])(struct coglink_lavaInfo *lavaInfo, char *track, struct coglink_requestInformation *res);
  int (*onDecodeTrackParseRequest[8])(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, struct coglink_parsedTrack *parsedTrackStruct);
  int (*onDecodeTracksRequest[8])(struct coglink_lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct coglink_requestInformation *res);
  int (*onDecodeTracksParseRequest[8])(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, char *songPos, struct coglink_parsedTrack *parsedTrackStruct);
};

struct coglink_pluginSecurity {
  _Bool allowReadWebsocket;
  _Bool allowReadBotToken;
  _Bool allowReadConcordWebsocket;
};

struct coglink_coglinkPlugins {
  struct coglink_pluginEventsInternal *events;
  struct coglink_pluginSecurity *security;
  int amount;
};

/**
 * Set the events that will be executed when performing a certain function.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param pluginEvents Structure with the pointer to the functions.
 */
void coglink_setPluginEvents(struct coglink_lavaInfo *lavaInfo, struct coglink_pluginEvents *pluginEvents);

#endif
