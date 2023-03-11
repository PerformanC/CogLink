#include <stdlib.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>
#include <coglink/information.h>

int coglink_getLavalinkVersion(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, char **version) {
  int node = _coglink_findPlayerNode(guildId);

  struct coglink_requestInformation res;

  int status = _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], &res, 
                                       &(struct __coglink_requestConfig) {
                                         .requestType = __COGLINK_GET_REQ,
                                         .path = "/version",
                                         .pathLength = 8,
                                         .getResponse = 1,
                                         .useVPath = 0
                                       });
  if (status != COGLINK_SUCCESS) return status;

  *version = res.body;

  free(res.body);

  return COGLINK_SUCCESS;
}

int coglink_getLavalinkInfo(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, struct coglink_requestInformation *res) {
  int node = _coglink_findPlayerNode(guildId);

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], res, 
                                 &(struct __coglink_requestConfig) {
                                   .requestType = __COGLINK_GET_REQ,
                                   .path = "/info",
                                   .pathLength = 6,
                                   .getResponse = 1
                                 });
}

int coglink_parseLavalinkInfo(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, struct coglink_lavalinkInfo *lavalinkInfoStruct) {
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

  jsmnf_pair *buildTime = jsmnf_find(pairs, res->body, "buildTime", sizeof("buildTime") - 1);

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

  jsmnf_pair *jvm = jsmnf_find(pairs, res->body, "jvm", sizeof("jvm") - 1);
  if (_coglink_checkParse(lavaInfo, jvm, "jvm")) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *lavaplayer = jsmnf_find(pairs, res->body, "lavaplayer", sizeof("lavaplayer") - 1);
  if (_coglink_checkParse(lavaInfo, lavaplayer, "lavaplayer")) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *sourceManagers = jsmnf_find(pairs, res->body, "sourceManagers", sizeof("sourceManagers") - 1);
  if (_coglink_checkParse(lavaInfo, sourceManagers, "sourceManagers")) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *filters = jsmnf_find(pairs, res->body, "filters", sizeof("filters") - 1);
  if (_coglink_checkParse(lavaInfo, filters, "filters")) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *plugins = jsmnf_find(pairs, res->body, "plugins", sizeof("plugins") - 1);
  if (_coglink_checkParse(lavaInfo, plugins, "plugins")) return COGLINK_JSMNF_ERROR_FIND;

  snprintf(lavalinkInfoStruct->version->semver, sizeof(lavalinkInfoStruct->version->semver), "%.*s", (int)semver->v.len, res->body + semver->v.pos);
  snprintf(lavalinkInfoStruct->version->major, sizeof(lavalinkInfoStruct->version->major), "%.*s", (int)major->v.len, res->body + major->v.pos);
  snprintf(lavalinkInfoStruct->version->minor, sizeof(lavalinkInfoStruct->version->minor), "%.*s", (int)minor->v.len, res->body + minor->v.pos);
  snprintf(lavalinkInfoStruct->version->patch, sizeof(lavalinkInfoStruct->version->patch), "%.*s", (int)patch->v.len, res->body + patch->v.pos);
  if (preRelease) snprintf(lavalinkInfoStruct->version->preRelease, sizeof(lavalinkInfoStruct->version->preRelease), "%.*s", (int)preRelease->v.len, res->body + preRelease->v.pos);
  snprintf(lavalinkInfoStruct->buildTime, sizeof(lavalinkInfoStruct->buildTime), "%.*s", (int)buildTime->v.len, res->body + buildTime->v.pos);
  snprintf(lavalinkInfoStruct->git->branch, sizeof(lavalinkInfoStruct->git->branch), "%.*s", (int)branch->v.len, res->body + branch->v.pos);
  snprintf(lavalinkInfoStruct->git->commit, sizeof(lavalinkInfoStruct->git->commit), "%.*s", (int)commit->v.len, res->body + commit->v.pos);
  snprintf(lavalinkInfoStruct->git->commitTime, sizeof(lavalinkInfoStruct->git->commitTime), "%.*s", (int)commitTime->v.len, res->body + commitTime->v.pos);
  snprintf(lavalinkInfoStruct->jvm, sizeof(lavalinkInfoStruct->jvm), "%.*s", (int)jvm->v.len, res->body + jvm->v.pos);
  snprintf(lavalinkInfoStruct->lavaplayer, sizeof(lavalinkInfoStruct->lavaplayer), "%.*s", (int)lavaplayer->v.len, res->body + lavaplayer->v.pos);
  snprintf(lavalinkInfoStruct->sourceManagers, sizeof(lavalinkInfoStruct->sourceManagers), "%.*s", (int)sourceManagers->v.len, res->body + sourceManagers->v.pos);
  snprintf(lavalinkInfoStruct->filters, sizeof(lavalinkInfoStruct->filters), "%.*s", (int)filters->v.len, res->body + filters->v.pos);
  snprintf(lavalinkInfoStruct->plugins, sizeof(lavalinkInfoStruct->plugins), "%.*s", (int)plugins->v.len, res->body + plugins->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> semver: %s\n> major: %s\n> minor: %s\n> patch: %s\n> preRelease: %s\n> buildTime: %s\n> branch: %s\n> commit: %s\n> commitTime: %s\n> jvm: %s\n> lavaplayer: %s\n> sourceManagers: %s\n> filters: %s\n> plugins: %s", lavalinkInfoStruct->version->semver, lavalinkInfoStruct->version->major, lavalinkInfoStruct->version->minor, lavalinkInfoStruct->version->patch, lavalinkInfoStruct->version->preRelease, lavalinkInfoStruct->buildTime, lavalinkInfoStruct->git->branch, lavalinkInfoStruct->git->commit, lavalinkInfoStruct->git->commitTime, lavalinkInfoStruct->jvm, lavalinkInfoStruct->lavaplayer, lavalinkInfoStruct->sourceManagers, lavalinkInfoStruct->filters, lavalinkInfoStruct->plugins); 

  return COGLINK_SUCCESS;
}

void coglink_getLavalinkInfoCleanup(struct coglink_requestInformation *res) {
  free(res->body);
}

int coglink_getLavalinkStats(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, struct coglink_requestInformation *res) {
  int node = _coglink_findPlayerNode(guildId);

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], res,
                                 &(struct __coglink_requestConfig) {
                                   .requestType = __COGLINK_GET_REQ,
                                   .path = "/stats",
                                   .pathLength = 6,
                                   .getResponse = 1
                                 });
}

int coglink_parseLavalinkStats(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, struct coglink_lavalinkStats *lavalinkStatsStruct) {
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
 
  jsmnf_pair *players = jsmnf_find(pairs, res->body, "players", sizeof("players") - 1);
  if (_coglink_checkParse(lavaInfo, players, "players") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *playingPlayers = jsmnf_find(pairs, res->body, "playingPlayers", sizeof("playingPlayers") - 1);
  if (_coglink_checkParse(lavaInfo, playingPlayers, "playingPlayers") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *uptime = jsmnf_find(pairs, res->body, "uptime", sizeof("uptime") - 1);
  if (_coglink_checkParse(lavaInfo, uptime, "uptime") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  char *path[] = { "memory", "free" };
  jsmnf_pair *lavaFree = jsmnf_find_path(pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, lavaFree, "lavaFree") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "used";
  jsmnf_pair *used = jsmnf_find_path(pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, used, "used") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "allocated";
  jsmnf_pair *allocated = jsmnf_find_path(pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, allocated, "allocated") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "reservable";
  jsmnf_pair *reservable = jsmnf_find_path(pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, reservable, "reservable") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[0] = "cpu";
  path[1] = "cores";
  jsmnf_pair *cores = jsmnf_find_path(pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, cores, "cores") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "systemLoad";
  jsmnf_pair *systemLoad = jsmnf_find_path(pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, systemLoad, "systemLoad") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "lavalinkLoad";
  jsmnf_pair *lavalinkLoad = jsmnf_find_path(pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, lavalinkLoad, "lavalinkLoad") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  snprintf(lavalinkStatsStruct->players, sizeof(lavalinkStatsStruct->players), "%.*s", (int)players->v.len, res->body + players->v.pos);
  snprintf(lavalinkStatsStruct->playingPlayers, sizeof(lavalinkStatsStruct->playingPlayers), "%.*s", (int)playingPlayers->v.len, res->body + playingPlayers->v.pos);
  snprintf(lavalinkStatsStruct->uptime, sizeof(lavalinkStatsStruct->uptime), "%.*s", (int)uptime->v.len, res->body + uptime->v.pos);
  snprintf(lavalinkStatsStruct->memory->free, sizeof(lavalinkStatsStruct->memory->free), "%.*s", (int)lavaFree->v.len, res->body + lavaFree->v.pos);
  snprintf(lavalinkStatsStruct->memory->used, sizeof(lavalinkStatsStruct->memory->used), "%.*s", (int)used->v.len, res->body + used->v.pos);
  snprintf(lavalinkStatsStruct->memory->allocated, sizeof(lavalinkStatsStruct->memory->allocated), "%.*s", (int)allocated->v.len, res->body + allocated->v.pos);
  snprintf(lavalinkStatsStruct->memory->reservable, sizeof(lavalinkStatsStruct->memory->reservable), "%.*s", (int)reservable->v.len, res->body + reservable->v.pos);
  snprintf(lavalinkStatsStruct->cpu->cores, sizeof(lavalinkStatsStruct->cpu->cores), "%.*s", (int)cores->v.len, res->body + cores->v.pos);
  snprintf(lavalinkStatsStruct->cpu->systemLoad, sizeof(lavalinkStatsStruct->cpu->systemLoad), "%.*s", (int)systemLoad->v.len, res->body + systemLoad->v.pos);
  snprintf(lavalinkStatsStruct->cpu->lavalinkLoad, sizeof(lavalinkStatsStruct->cpu->lavalinkLoad), "%.*s", (int)lavalinkLoad->v.len, res->body + lavalinkLoad->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> Players: %s\n> PlayingPlayers: %s\n> Uptime: %s\n> Free: %s\n> Used: %s\n> Allocated: %s\n> Reservable: %s\n> Cores: %s\n> SystemLoad: %s\n> LavalinkLoad: %s", lavalinkStatsStruct->players, lavalinkStatsStruct->playingPlayers, lavalinkStatsStruct->uptime, lavalinkStatsStruct->memory->free, lavalinkStatsStruct->memory->used, lavalinkStatsStruct->memory->allocated, lavalinkStatsStruct->memory->reservable, lavalinkStatsStruct->cpu->cores, lavalinkStatsStruct->cpu->systemLoad, lavalinkStatsStruct->cpu->lavalinkLoad);

  return COGLINK_SUCCESS;
}

void coglink_getLavalinkStatsCleanup(struct coglink_requestInformation *res) {
  free(res->body);
}
