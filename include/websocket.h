/**
 * @file include/websocket.h
 * @brief Websocket client for the node.
*/

#ifndef COGLINK_LAVALINK_WEBSOCKET_H
#define COGLINK_LAVALINK_WEBSOCKET_H

#include "lavalink.h"
#include "codecs.h"

/**
 * @brief Initialize the CogLink client and connect to the nodes.
 *
 * @param c_client The CogLink client. Should be allocated outside of this function.
 * @param client The Discord client created with discord_init.
 * @param nodes The nodes to connect to.
 * 
 * @return int COGLINK_SUCCESS on success, COGLINK_FAILED on failure.
 * 
 * @note This function should be called after discord_init.
 * @note The CogLink client should be cleaned up with coglink_cleanup.
*/
int coglink_connect_nodes(struct coglink_client *c_client, struct discord *client, struct coglink_nodes *nodes);

/**
 * @brief Cleanups all the resources used by the CogLink client.
 * 
 * @param c_client The CogLink client.
 * 
 * @note This function can't be replaced (outside cleanup) as it cleans global resources.
*/
void coglink_cleanup(struct coglink_client *c_client);

#endif