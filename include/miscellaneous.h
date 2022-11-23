#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H

struct lavaDetailsIpBlock {
  char *type;
  char *size;
};

struct lavaDetailsFailingAddress {
  char *address;
  char *failingTimestamp;
  char *failingTime;
};

struct lavaRouterDetails {
  struct lavaDetailsIpBlock *ipBlock;
  struct lavaDetailsFailingAddress *failingAddress;
  /* RotatingNanoIpRoutePlanner */
  char *blockIndex;
  char *currentAddressIndex;
  /* RotatingIpRoutePlanner */
  char *rotateIndex;
  char *ipIndex;
  char *currentAddress;
};

struct lavaRouter {
  char *class;
  struct lavaRouterDetails *details;
};

int coglink_decodeTrack(struct lavaInfo *lavaInfo, char *track, struct httpRequest *res);

int coglink_parseDecodeTrack(struct lavaInfo *lavaInfo, struct httpRequest *res, struct lavaParsedTrack **songStruct);

int coglink_decodeTracks(struct lavaInfo *lavaInfo, char *trackArray, struct httpRequest *res);

int coglink_parseDecodeTracks(const struct lavaInfo *lavaInfo, struct httpRequest *req, char *songPos, struct lavaParsedTrack **songStruct);

int coglink_getPlugins(struct lavaInfo *lavaInfo, struct httpRequest *res);

int coglink_getRouterPlanner(struct lavaInfo *lavaInfo, struct httpRequest *res);

int coglink_parseRouterPlanner(struct lavaInfo *lavaInfo, struct httpRequest *req, char *ipPosition, struct lavaRouter **lavaRouterStruct);

int coglink_freeFailingAddress(struct lavaInfo *lavaInfo, char *ip);

int coglink_freeFailingAllAddresses(struct lavaInfo *lavaInfo);

#endif

