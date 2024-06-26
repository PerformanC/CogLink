/**
 * @file include/lavalink.h
 * @brief Lavalink and NodeLink client main header.
*/

#ifndef COGLINK_LAVALINK_H
#define COGLINK_LAVALINK_H

#include <concord/types.h>
#include <concord/websockets.h>
#include <concord/discord.h>

#include "types.h"

struct coglink_node {
  /* Params */
  char *name;
  char *hostname;
  int port;
  char *password;
  bool ssl;
  /* Public info */
  char *session_id;
  struct coglink_stats *stats;
  /* Internal */
  uint64_t tstamp;
  CURLM *mhandle;
  struct websockets *ws;
  struct _coglink_websocket_data *ws_data;
};

struct coglink_nodes {
  struct coglink_node *array;
  size_t size;
};

struct coglink_client {
  /* Params */
  u64snowflake bot_id;
  char *num_shards;
  struct coglink_events *events;
  /* Public info */
  struct coglink_nodes *nodes;
  struct coglink_players *players;
  struct coglink_users *users;
  /* Internal (passed options) */
  bool allow_resuming;
  bool allow_caching_voice_channel_ids;
};

#include "codecs.h"
#include "websocket.h"
#include "events.h"
#include "rest.h"

#endif /* COGLINK_LAVALINK_H */
