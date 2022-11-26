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

struct lavalinkInfoVersion {
  char *semver;
  char *major;
  char *minor;
  char *patch;
  char *preRelease;
};

struct lavalinkInfoGit {
  char *branch;
  char *commit;
  char *commitTime;
};

struct lavalinkInfo {
  struct lavalinkInfoVersion *version;
  char *buildTime;
  struct lavalinkInfoGit *git;
  char *jvm;
  char *lavaplayer;
  char *sourceManagers;
  char *filters;
  char *plugins;
};

int coglink_getLavalinkVersion(struct lavaInfo *lavaInfo, char **version);

int coglink_decodeTrack(struct lavaInfo *lavaInfo, char *track, struct httpRequest *res);

int coglink_parseDecodeTrack(struct lavaInfo *lavaInfo, struct httpRequest *res, struct lavaParsedTrack **songStruct);

int coglink_decodeTracks(struct lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct httpRequest *res);

int coglink_parseDecodeTracks(const struct lavaInfo *lavaInfo, struct httpRequest *req, char *songPos, struct lavaParsedTrack **songStruct);

int coglink_getLavalinkInfo(struct lavaInfo *lavaInfo, struct httpRequest *res);

int coglink_parseLavalinkInfo(struct lavaInfo *lavaInfo, struct httpRequest *res, struct lavalinkInfo **lavalinkInfoStruct);

int coglink_getLavalinkStats(struct lavaInfo *lavaInfo, struct httpRequest *res);

int coglink_parseLavalinkStats(struct lavaInfo *lavaInfo, struct httpRequest *res, struct lavalinkStats **lavalinkStatsStruct);

int coglink_getRouterPlanner(struct lavaInfo *lavaInfo, struct httpRequest *res);

int coglink_parseRouterPlanner(struct lavaInfo *lavaInfo, struct httpRequest *req, char *ipPosition, struct lavaRouter **lavaRouterStruct);

int coglink_freeFailingAddress(struct lavaInfo *lavaInfo, char *ip);

int coglink_freeFailingAllAddresses(struct lavaInfo *lavaInfo);

void coglink_parseDecodeTrackCleanup(const struct lavaInfo *lavaInfo, struct lavaParsedTrack *songStruct);

void coglink_parseLavalinkInfoCleanup(const struct lavaInfo *lavaInfo, struct lavalinkInfo *lavalinkInfoStruct);

void coglink_parseLavalinkStatsCleanup(const struct lavaInfo *lavaInfo, struct lavalinkStats *lavalinkStatsStruct);

void coglink_parseRouterPlannerCleanup(const struct lavaInfo *lavaInfo, struct lavaRouter *lavaRouterStruct);

#endif

