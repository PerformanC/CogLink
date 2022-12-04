/** \file
 * File containing the functions for plugin support.
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include <concord/types.h>
#include <concord/discord.h>
#include <concord/discord-events.h>

struct lavaInfo;
struct requestInformation;
struct parsedTrack;

struct pluginEvents {
  int (*onSearchRequest)(struct lavaInfo *lavaInfo, char *song, struct requestInformation **res);
  int (*onPlayRequest)(struct lavaInfo *lavaInfo, char *track, u64snowflake guildId);
  int (*onLavalinkPacketReceived)(struct lavaInfo *lavaInfo, const char *text, size_t length);
  int (*onLavalinkClose)(struct lavaInfo *lavaInfo, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t length);
  int (*onCoglinkScheduler)(struct lavaInfo *lavaInfo, struct discord *client, const char data[], size_t size, enum discord_gateway_events event);
  int (*onDecodeTrackRequest)(struct lavaInfo *lavaInfo, char *track, struct requestInformation **res);
  int (*onDecodeTrackParseRequest)(struct lavaInfo *lavaInfo, struct requestInformation **res, struct parsedTrack ***parsedTrackStruct);
  int (*onDecodeTracksRequest)(struct lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct requestInformation **res);
  int (*onDecodeTracksParseRequest)(struct lavaInfo *lavaInfo, struct requestInformation **res, char *songPos, struct parsedTrack ***parsedTrackStruct);
};

struct pluginEventsInternal {
  int (*onSearchRequest[8])(struct lavaInfo *lavaInfo, char *song, struct requestInformation **res);
  int (*onPlayRequest[8])(struct lavaInfo *lavaInfo, char *track, u64snowflake guildId);
  int (*onLavalinkPacketReceived[8])(struct lavaInfo *lavaInfo, const char *text, size_t length);
  int (*onLavalinkClose[8])(struct lavaInfo *lavaInfo, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t length);
  int (*onCoglinkScheduler[8])(struct lavaInfo *lavaInfo, struct discord *client, const char data[], size_t size, enum discord_gateway_events event);
  int (*onDecodeTrackRequest[8])(struct lavaInfo *lavaInfo, char *track, struct requestInformation **res);
  int (*onDecodeTrackParseRequest[8])(struct lavaInfo *lavaInfo, struct requestInformation **res, struct parsedTrack ***parsedTrackStruct);
  int (*onDecodeTracksRequest[8])(struct lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct requestInformation **res);
  int (*onDecodeTracksParseRequest[8])(struct lavaInfo *lavaInfo, struct requestInformation **res, char *songPos, struct parsedTrack ***parsedTrackStruct);
};

struct pluginSecurity {
  _Bool allowReadIOPoller;
  _Bool allowReadBotToken;
  _Bool allowReadConcordWebsocket;
};

struct coglinkPlugins {
  struct pluginEventsInternal *events;
  struct pluginSecurity *security;
  int amount;
};

/**
 * Set the events that will be executed when performing a certain function.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param pluginEvents Structure with the pointer to the functions.
 */
void coglink_setPluginEvents(struct lavaInfo *lavaInfo, struct pluginEvents *pluginEvents);

#endif
