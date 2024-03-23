#ifndef COGLINK_LAVALINK_WEBSOCKET_H
#define COGLINK_LAVALINK_WEBSOCKET_H

#include "lavalink.h"
#include "codecs.h"

struct _coglink_websocket_data {
  struct coglink_client *c_client;
  size_t node_id;
};

struct coglink_voice_data {
  char *token;
  char *endpoint;
  char *session_id;
};

int coglink_connect_nodes(struct coglink_client *c_client, struct discord *client, struct coglink_nodes *nodes);

void coglink_cleanup(struct coglink_client *c_client, struct discord *client);

#endif