#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H

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

int coglink_decodeTrack(struct lavaInfo *lavaInfo, char *track, struct requestInformation *res);

int coglink_parseDecodeTrack(struct lavaInfo *lavaInfo, struct requestInformation *res, struct parsedTrack **songStruct);

int coglink_decodeTracks(struct lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct requestInformation *res);

int coglink_parseDecodeTracks(struct lavaInfo *lavaInfo, struct requestInformation *res, char *songPos, struct parsedTrack **songStruct);

int coglink_getLavalinkInfo(struct lavaInfo *lavaInfo, struct requestInformation *res);

int coglink_parseLavalinkInfo(struct lavaInfo *lavaInfo, struct requestInformation *res, struct lavalinkInfo **lavalinkInfoStruct);

int coglink_getLavalinkStats(struct lavaInfo *lavaInfo, struct requestInformation *res);

int coglink_parseLavalinkStats(struct lavaInfo *lavaInfo, struct requestInformation *res, struct lavalinkStats **lavalinkStatsStruct);

int coglink_getRouterPlanner(struct lavaInfo *lavaInfo, struct requestInformation *res);

int coglink_parseRouterPlanner(struct lavaInfo *lavaInfo, struct requestInformation *res, char *ipPosition, struct lavalinkRouter **lavaRouterStruct);

int coglink_freeFailingAddress(struct lavaInfo *lavaInfo, char *ip);

int coglink_freeFailingAllAddresses(struct lavaInfo *lavaInfo);

void coglink_parseDecodeTrackCleanup(struct lavaInfo *lavaInfo, struct parsedTrack *songStruct);

void coglink_parseLavalinkInfoCleanup(struct lavaInfo *lavaInfo, struct lavalinkInfo *lavalinkInfoStruct);

void coglink_parseLavalinkStatsCleanup(struct lavaInfo *lavaInfo, struct lavalinkStats *lavalinkStatsStruct);

void coglink_parseRouterPlannerCleanup(struct lavaInfo *lavaInfo, struct lavalinkRouter *lavaRouterStruct);

#endif

