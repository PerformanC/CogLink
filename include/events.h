#ifndef COGLINK_LAVALINK_EVENTS_H
#define COGLINK_LAVALINK_EVENTS_H

#include "lavalink.h"
// #include "utils.h"
#include "codecs.h"

struct coglink_events {
  int (*on_raw)(struct coglink_client *c_client, const char *data, size_t length);
  void (*on_ready)(struct coglink_ready_payload *ready);
  void (*on_close)(enum ws_close_reason wscode, const char *reason);
  void (*on_track_start)(struct coglink_track_start_payload *trackStart);
  void (*on_track_end)(struct coglink_track_end_payload *trackEnd);
  void (*on_track_excetion)(struct coglink_track_exception_payload *trackException);
  void (*on_track_stuck)(struct coglink_track_stuck_payload *trackStuck);
  void (*on_websocket_closed)(struct coglink_websocket_closed_payload *websocketClosed);
  void (*on_player_update)(struct coglink_player_update_payload *playerUpdate);
  void (*on_stats)(struct coglink_stats_payload *stats);
};

#endif /* COGLINK_LAVALINK_EVENTS_H */
