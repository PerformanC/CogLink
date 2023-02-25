/** \file
 * File containing the Lavalink information related functions.
 */

#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H

struct lavalinkInfoVersion {
  char semver[8];
  char major[4];
  char minor[4];
  char patch[4];
  char preRelease[8];
};

struct lavalinkInfoGit {
  char branch[16];
  char commit[32];
  char commitTime[16];
};

struct lavalinkInfo {
  struct lavalinkInfoVersion *version;
  char buildTime[16];
  struct lavalinkInfoGit *git;
  char jvm[8];
  char lavaplayer[8];
  char sourceManagers[128];
  char filters[128];
  char plugins[128];
};

/**
 * Retrieves the Lavalink version.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param version String with Lavalink version.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_getLavalinkVersion(struct lavaInfo *lavaInfo, char **version);

/**
 * Retrieves the informations of the Lavalink node Lavalink.jar.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_getLavalinkInfo(struct lavaInfo *lavaInfo, struct requestInformation *res);

/**
 * Parses the response body of coglink_getLavalinkInfo function.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param lavalinkInfoStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseLavalinkInfo(struct lavaInfo *lavaInfo, struct requestInformation *res, struct lavalinkInfo *lavalinkInfoStruct);

/**
 * Frees the allocations generated while performing the function coglink_getLavalinkInfo.
 * @param res Structure with the information of the request.
 */
void coglink_getLavalinkInfoCleanup(struct requestInformation *res);

/**
 * Retrieves the stats of the Lavalink node.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_getLavalinkStats(struct lavaInfo *lavaInfo, struct requestInformation *res);

/**
 * Parses the response body of coglink_getLavalinkStats function.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param lavalinkStatsStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseLavalinkStats(struct lavaInfo *lavaInfo, struct requestInformation *res, struct lavalinkStats *lavalinkStatsStruct);

/**
 * Frees the allocations generated while performing the function coglink_getLavalinkStats.
 * @param res Structure with the information of the request.
 */
void coglink_getLavalinkStatsCleanup(struct requestInformation *res);

#endif
