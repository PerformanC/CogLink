/** \file
 * File containing the Lavalink network related functions.
 */

#ifndef NETWORK_H
#define NETWORK_H

struct lavalinkDetailsIpBlock {
  char *type;
  char *size;
};

struct lavalinkDetailsFailingAddress {
  char *address;
  char *failingTimestamp;
  char *failingTime;
};

struct lavalinkRouterDetails {
  struct lavalinkDetailsIpBlock *ipBlock;
  struct lavalinkDetailsFailingAddress *failingAddress;
  char *blockIndex;
  char *currentAddressIndex;
  char *rotateIndex;
  char *ipIndex;
  char *currentAddress;
};

struct lavalinkRouter {
  char *class;
  struct lavalinkRouterDetails *details;
};

/**
 * Retrieves the Lavalink router planner.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_getRouterPlanner(struct lavaInfo *lavaInfo, struct requestInformation *res);

/**
 * Parses the response body of coglink_getRouterPlanner function.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param ipPosition Position of the IP that will be parsed
 * @param lavalinkRouterStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseRouterPlanner(struct lavaInfo *lavaInfo, struct requestInformation *res, char *ipPosition, struct lavalinkRouter **lavalinkRouterStruct);

/**
 * Frees the allocations generated while performing the function coglink_getRouterPlanner.
 * @param res Structure with the information of the request.
 */
void coglink_getRouterPlannerCleanup(struct requestInformation *res);

/**
 * Removes an IP from failing address list.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param ip IP that will be removed from failing address list.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_freeFailingAddress(struct lavaInfo *lavaInfo, char *ip);

/**
 * Removes all IPs from failing address list.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_freeFailingAllAddresses(struct lavaInfo *lavaInfo);

#endif
