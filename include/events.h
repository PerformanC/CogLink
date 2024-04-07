/**
 * @file include/events.h
 * @brief Events for the Lavalink and NodeLink client.
*/

#ifndef COGLINK_LAVALINK_EVENTS_H
#define COGLINK_LAVALINK_EVENTS_H

#include "lavalink.h"
#include "codecs.h"

struct coglink_events {
  void (*on_connect)(struct coglink_client *c_client, struct coglink_node *node);
  void (*on_close)(struct coglink_client *c_client, struct coglink_node *node, enum ws_close_reason wscode, const char *reason);
  int (*on_raw)(struct coglink_client *c_client, struct coglink_node *node, const char *data, size_t length);
  void (*on_ready)(struct coglink_client *c_client, struct coglink_node *node, struct coglink_ready *ready);
  void (*on_track_start)(struct coglink_client *c_client, struct coglink_node *node, struct coglink_track_start *trackStart);
  void (*on_track_end)(struct coglink_client *c_client, struct coglink_node *node, struct coglink_track_end *trackEnd);
  void (*on_track_excetion)(struct coglink_client *c_client, struct coglink_node *node, struct coglink_track_exception *trackException);
  void (*on_track_stuck)(struct coglink_client *c_client, struct coglink_node *node, struct coglink_track_stuck *trackStuck);
  void (*on_websocket_closed)(struct coglink_client *c_client, struct coglink_node *node, struct coglink_websocket_closed *websocketClosed);
  void (*on_player_update)(struct coglink_client *c_client, struct coglink_node *node, struct coglink_player_update *playerUpdate);
  void (*on_stats)(struct coglink_client *c_client, struct coglink_node *node, struct coglink_stats *stats);
};

#endif /* COGLINK_LAVALINK_EVENTS_H */
