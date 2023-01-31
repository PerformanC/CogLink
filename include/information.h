/** \file
 * File containing the Lavalink information related functions.
 */

#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H

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
int coglink_parseLavalinkInfo(struct lavaInfo *lavaInfo, struct requestInformation *res, struct lavalinkInfo **lavalinkInfoStruct);

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
int coglink_parseLavalinkStats(struct lavaInfo *lavaInfo, struct requestInformation *res, struct lavalinkStats **lavalinkStatsStruct);

/**
 * Frees the allocations generated while performing the function coglink_getLavalinkStats.
 * @param res Structure with the information of the request.
 */
void coglink_getLavalinkStatsCleanup(struct requestInformation *res);

#endif
