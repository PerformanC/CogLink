/** \file
 * File containing the Lavalink network related functions.
 */

#ifndef NETWORK_H
#define NETWORK_H

struct coglink_lavalinkDetailsIpBlock {
  char type[16];
  char size[16];
};

struct coglink_lavalinkDetailsFailingAddress {
  char address[8];
  char failingTimestamp[16];
  char failingTime[16];
};

struct coglink_lavalinkRouterDetails {
  struct coglink_lavalinkDetailsIpBlock *ipBlock;
  struct coglink_lavalinkDetailsFailingAddress *failingAddress;
  char blockIndex[16];
  char currentAddressIndex[16];
  char rotateIndex[16];
  char ipIndex[16];
  char currentAddress[8];
};

struct coglink_lavalinkRouter {
  char class[16];
  struct coglink_lavalinkRouterDetails *details;
};

/**
 * Retrieves the Lavalink router planner.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_getRouterPlanner(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, struct coglink_requestInformation *res);

/**
 * Parses the response body of coglink_getRouterPlanner function.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param ipPosition Position of the IP that will be parsed
 * @param lavalinkRouterStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseRouterPlanner(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, char *ipPosition, struct coglink_lavalinkRouter *lavalinkRouterStruct);

/**
 * Frees the allocations generated while performing the function coglink_getRouterPlanner.
 * @param res Structure with the information of the request.
 */
void coglink_getRouterPlannerCleanup(struct coglink_requestInformation *res);

/**
 * Removes an IP from failing address list.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param ip IP that will be removed from failing address list.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_freeFailingAddress(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, char *ip);

/**
 * Removes all IPs from failing address list.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_freeFailingAllAddresses(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId);

#endif
