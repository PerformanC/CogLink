#ifndef COGLINK_LAVALINK_EVENTS_H
#define COGLINK_LAVALINK_EVENTS_H

#include "lavalink.h"
// #include "utils.h"
#include "codecs.h"

struct coglink_events {
  int (*onRaw)(struct coglink_client *c_client, const char *data, size_t length);
  void (*onReady)(struct coglink_ready_payload *ready);
  void (*onClose)(enum ws_close_reason wscode, const char *reason);
  // void (*onTrackStart)(char *guild_id, struct coglink_partial_track *track);
  // void (*onTrackEnd)(char *guild_id, struct coglink_partial_track *track, char *reason);
  // void (*onTrackException)(char *guild_id, struct coglink_partial_track *track, char *message, char *severity, char *cause);
  // void (*onTrackStuck)(char *guild_id, char *thresholdMs, struct coglink_track *track);
  // void (*onWebSocketClosed)(char *guild_id, char *code, char *reason, int byRemote);
  // void (*onUnknownEvent)(char *guild_id, char *type, const char *text);
  void (*onPlayerUpdate)(struct coglink_player_update_payload *playerUpdate);
  void (*onStats)(struct coglink_stats_payload *stats);
  // void (*onUnknownOp)(char *op, const char *text);
};

#endif /* COGLINK_LAVALINK_EVENTS_H */
