#ifndef COGLINK_LAVALINK_WEBSOCKET_H
#define COGLINK_LAVALINK_WEBSOCKET_H

#include "lavalink.h"
#include "codecs.h"

int coglink_connect_nodes(struct coglink_client *c_client, struct discord *client, struct coglink_nodes *nodes);

void coglink_cleanup(struct coglink_client *c_client);

#endif