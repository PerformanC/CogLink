#ifndef COGLINK_LAVALINK_EVENTS_H
#define COGLINK_LAVALINK_EVENTS_H

#include "lavalink.h"
#include "codecs.h"

struct coglink_events {
  int (*on_raw)(struct coglink_client *c_client, const char *data, size_t length);
  void (*on_ready)(struct coglink_ready *ready);
  void (*on_close)(enum ws_close_reason wscode, const char *reason);
  void (*on_track_start)(struct coglink_track_start *trackStart);
  void (*on_track_end)(struct coglink_track_end *trackEnd);
  void (*on_track_excetion)(struct coglink_track_exception *trackException);
  void (*on_track_stuck)(struct coglink_track_stuck *trackStuck);
  void (*on_websocket_closed)(struct coglink_websocket_closed *websocketClosed);
  void (*on_player_update)(struct coglink_player_update *playerUpdate);
  void (*on_stats)(struct coglink_stats *stats);
};

#endif /* COGLINK_LAVALINK_EVENTS_H */
