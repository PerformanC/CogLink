#include <stdlib.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>
#include <coglink/information.h>

int coglink_getLavalinkVersion(struct lavaInfo *lavaInfo, char **version) {
  struct requestInformation res;

  int status = __coglink_performRequest(lavaInfo, &res, &(struct __coglink_requestConfig) {
                                                          .requestType = __COGLINK_GET_REQ,
                                                          .path = "/version",
                                                          .pathLength = 8,
                                                          .getResponse = true
                                                        });
  if (status != COGLINK_SUCCESS) return status;

  *version = res.body;

  free(res.body);

  return COGLINK_SUCCESS;
}

int coglink_getLavalinkInfo(struct lavaInfo *lavaInfo, struct requestInformation *res) {
  return __coglink_performRequest(lavaInfo, res, &(struct __coglink_requestConfig) {
                                                    .requestType = __COGLINK_GET_REQ,
                                                    .path = "/info",
                                                    .pathLength = 6,
                                                    .useV3Path = true,
                                                    .getResponse = true
                                                  });
}

int coglink_parseLavalinkInfo(struct lavaInfo *lavaInfo, struct requestInformation *res, struct lavalinkInfo **lavalinkInfoStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[64];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[64];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
  
  char *path[] = { "version", "semver" };
  jsmnf_pair *semver = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "major";
  jsmnf_pair *major = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "minor";
  jsmnf_pair *minor = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "patch";
  jsmnf_pair *patch = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "preRelease";
  jsmnf_pair *preRelease = jsmnf_find_path(pairs, res->body, path, 2);

  if (!major || !minor || !patch) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmnf-find] Error while trying to find %s field.", !semver ? "semver" : !major ? "major" : !minor ? "minor" : "patch");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  jsmnf_pair *buildTime = jsmnf_find(pairs, res->body, "buildTime", 9);

  path[0] = "git";
  path[1] = "branch";
  jsmnf_pair *branch = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "commit";
  jsmnf_pair *commit = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "commitTime";
  jsmnf_pair *commitTime = jsmnf_find_path(pairs, res->body, path, 2);

  if (!buildTime || !branch || !commit || !commitTime) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmnf-find] Error while trying to find %s field.", !buildTime ? "buildTime" : !branch ? "branch" : !commit ? "commit" : "commitTime");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  jsmnf_pair *jvm = jsmnf_find(pairs, res->body, "jvm", 3);
  if (__coglink_checkParse(lavaInfo, jvm, "jvm")) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *lavaplayer = jsmnf_find(pairs, res->body, "lavaplayer", 10);
  if (__coglink_checkParse(lavaInfo, lavaplayer, "lavaplayer")) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *sourceManagers = jsmnf_find(pairs, res->body, "sourceManagers", 14);
  if (__coglink_checkParse(lavaInfo, sourceManagers, "sourceManagers")) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *filters = jsmnf_find(pairs, res->body, "filters", 7);
  if (__coglink_checkParse(lavaInfo, filters, "filters")) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *plugins = jsmnf_find(pairs, res->body, "plugins", 7);
  if (__coglink_checkParse(lavaInfo, plugins, "plugins")) return COGLINK_JSMNF_ERROR_FIND;

  char Semver[8], Major[4], Minor[4], Patch[4], PreRelease[8], BuildTime[16], Branch[16], Commit[32], CommitTime[16], Jvm[8], Lavaplayer[8], SourceManagers[128], Filters[128], Plugins[128];

  snprintf(Semver, sizeof(Semver), "%.*s", (int)semver->v.len, res->body + semver->v.pos);
  snprintf(Major, sizeof(Major), "%.*s", (int)major->v.len, res->body + major->v.pos);
  snprintf(Minor, sizeof(Minor), "%.*s", (int)minor->v.len, res->body + minor->v.pos);
  snprintf(Patch, sizeof(Patch), "%.*s", (int)patch->v.len, res->body + patch->v.pos);
  if (preRelease) snprintf(PreRelease, sizeof(PreRelease), "%.*s", (int)preRelease->v.len, res->body + preRelease->v.pos);
  snprintf(BuildTime, sizeof(BuildTime), "%.*s", (int)buildTime->v.len, res->body + buildTime->v.pos);
  snprintf(Branch, sizeof(Branch), "%.*s", (int)branch->v.len, res->body + branch->v.pos);
  snprintf(Commit, sizeof(Commit), "%.*s", (int)commit->v.len, res->body + commit->v.pos);
  snprintf(CommitTime, sizeof(CommitTime), "%.*s", (int)commitTime->v.len, res->body + commitTime->v.pos);
  snprintf(Jvm, sizeof(Jvm), "%.*s", (int)jvm->v.len, res->body + jvm->v.pos);
  snprintf(Lavaplayer, sizeof(Lavaplayer), "%.*s", (int)lavaplayer->v.len, res->body + lavaplayer->v.pos);
  snprintf(SourceManagers, sizeof(SourceManagers), "%.*s", (int)sourceManagers->v.len, res->body + sourceManagers->v.pos);
  snprintf(Filters, sizeof(Filters), "%.*s", (int)filters->v.len, res->body + filters->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> semver: %s\n> major: %s\n> minor: %s\n> patch: %s\n> preRelease: %s\n> buildTime: %s\n> branch: %s\n> commit: %s\n> commitTime: %s\n> jvm: %s\n> lavaplayer: %s\n> sourceManagers: %s\n> filters: %s\n> plugins: %s", Semver, Major, Minor, Patch, PreRelease, BuildTime, Branch, Commit, CommitTime, Jvm, Lavaplayer, SourceManagers, Filters, Plugins); 

  *lavalinkInfoStruct = &(struct lavalinkInfo) {
    .version = &(struct lavalinkInfoVersion) {
      .semver = Semver,
      .major = Major,
      .minor = Minor,
      .patch = Patch,
      .preRelease = (preRelease ? PreRelease : NULL)
    },
    .buildTime = BuildTime,
    .git = &(struct lavalinkInfoGit) {
      .branch = Branch,
      .commit = Commit,
      .commitTime = CommitTime
    },
    .jvm = Jvm,
    .lavaplayer = Lavaplayer,
    .sourceManagers = SourceManagers,
    .filters = Filters,
    .plugins = Plugins
  };

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-management] Set the value for struct members of lavalinkInfoStruct.");

  return COGLINK_SUCCESS;
}

void coglink_getLavalinkInfoCleanup(struct requestInformation *res) {
  free(res->body);
}

int coglink_getLavalinkStats(struct lavaInfo *lavaInfo, struct requestInformation *res) {
  return __coglink_performRequest(lavaInfo, res, &(struct __coglink_requestConfig) {
                                                    .requestType = __COGLINK_GET_REQ,
                                                    .path = "/stats",
                                                    .pathLength = 6,
                                                    .useV3Path = true,
                                                    .getResponse = true
                                                  });
}

int coglink_parseLavalinkStats(struct lavaInfo *lavaInfo, struct requestInformation *res, struct lavalinkStats **lavalinkStatsStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[32];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[32];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
 
  jsmnf_pair *players = jsmnf_find(pairs, res->body, "players", 7);
  if (__coglink_checkParse(lavaInfo, players, "players") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *playingPlayers = jsmnf_find(pairs, res->body, "playingPlayers", 14);
  if (__coglink_checkParse(lavaInfo, playingPlayers, "playingPlayers") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *uptime = jsmnf_find(pairs, res->body, "uptime", 6);
  if (__coglink_checkParse(lavaInfo, uptime, "uptime") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  char *path[] = { "memory", "free" };
  jsmnf_pair *lavaFree = jsmnf_find_path(pairs, res->body, path, 2);
  if (__coglink_checkParse(lavaInfo, lavaFree, "lavaFree") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "used";
  jsmnf_pair *used = jsmnf_find_path(pairs, res->body, path, 2);
  if (__coglink_checkParse(lavaInfo, used, "used") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "allocated";
  jsmnf_pair *allocated = jsmnf_find_path(pairs, res->body, path, 2);
  if (__coglink_checkParse(lavaInfo, allocated, "allocated") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "reservable";
  jsmnf_pair *reservable = jsmnf_find_path(pairs, res->body, path, 2);
  if (__coglink_checkParse(lavaInfo, reservable, "reservable") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[0] = "cpu";
  path[1] = "cores";
  jsmnf_pair *cores = jsmnf_find_path(pairs, res->body, path, 2);
  if (__coglink_checkParse(lavaInfo, cores, "cores") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "systemLoad";
  jsmnf_pair *systemLoad = jsmnf_find_path(pairs, res->body, path, 2);
  if (__coglink_checkParse(lavaInfo, systemLoad, "systemLoad") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "lavalinkLoad";
  jsmnf_pair *lavalinkLoad = jsmnf_find_path(pairs, res->body, path, 2);
  if (__coglink_checkParse(lavaInfo, lavalinkLoad, "lavalinkLoad") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  char Players[8], PlayingPlayers[8], Uptime[32], Free[16], Used[16], Allocated[16], Reservable[16], Cores[8], SystemLoad[16], LavalinkLoad[16];

  snprintf(Players, sizeof(Players), "%.*s", (int)players->v.len, res->body + players->v.pos);
  snprintf(PlayingPlayers, sizeof(PlayingPlayers), "%.*s", (int)playingPlayers->v.len, res->body + playingPlayers->v.pos);
  snprintf(Uptime, sizeof(Uptime), "%.*s", (int)uptime->v.len, res->body + uptime->v.pos);
  snprintf(Free, sizeof(Free), "%.*s", (int)lavaFree->v.len, res->body + lavaFree->v.pos);
  snprintf(Used, sizeof(Used), "%.*s", (int)used->v.len, res->body + used->v.pos);
  snprintf(Allocated, sizeof(Allocated), "%.*s", (int)allocated->v.len, res->body + allocated->v.pos);
  snprintf(Reservable, sizeof(Reservable), "%.*s", (int)reservable->v.len, res->body + reservable->v.pos);
  snprintf(Cores, sizeof(Cores), "%.*s", (int)cores->v.len, res->body + cores->v.pos);
  snprintf(SystemLoad, sizeof(SystemLoad), "%.*s", (int)systemLoad->v.len, res->body + systemLoad->v.pos);
  snprintf(LavalinkLoad, sizeof(LavalinkLoad), "%.*s", (int)lavalinkLoad->v.len, res->body + lavalinkLoad->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> Players: %s\n> PlayingPlayers: %s\n> Uptime: %s\n> Free: %s\n> Used: %s\n> Allocated: %s\n> Reservable: %s\n> Cores: %s\n> SystemLoad: %s\n> LavalinkLoad: %s", Players, PlayingPlayers, Uptime, Free, Used, Allocated, Reservable, Cores, SystemLoad, LavalinkLoad);

  *lavalinkStatsStruct = &(struct lavalinkStats) {
    .players = Players,
    .playingPlayers = PlayingPlayers,
    .uptime = Uptime,
    .memory = &(struct lavalinkStatsMemory) {
      .free = Free,
      .used = Used,
      .allocated = Allocated,
      .reservable = Reservable
    },
    .cpu = &(struct lavalinkStatsCPU) {
      .cores = Cores,
      .systemLoad = SystemLoad,
      .lavalinkLoad = LavalinkLoad
    }
  };

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-management] Set the value for struct members of lavalinkStatsStruct.");

  return COGLINK_SUCCESS;
}

void coglink_getLavalinkStatsCleanup(struct requestInformation *res) {
  free(res->body);
}
