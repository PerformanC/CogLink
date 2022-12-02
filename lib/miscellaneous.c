#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>
#include <coglink/miscellaneous.h>

int coglink_getLavalinkVersion(struct lavaInfo *lavaInfo, char **version) {
  struct requestInformation res;

  int status = __coglink_performRequest(lavaInfo, __COGLINK_GET_REQ, 0, 0, "/version", 8, 0, NULL, 0, &res, 1, NULL);
  if (status != COGLINK_SUCCESS) return status;

  *version = res.body;

  free(res.body);

  return COGLINK_SUCCESS;
}

int coglink_decodeTrack(struct lavaInfo *lavaInfo, char *track, struct requestInformation *res) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTrackRequest[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTrackRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackRequest[i](lavaInfo, track, &res);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTrackRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackRequest[i](lavaInfoPlugin, track, &res);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    }
  }

  char reqPath[strnlen(track, 512) + 26];
  snprintf(reqPath, sizeof(reqPath), "/decodetrack?encodedTrack=%s", track);

  return __coglink_performRequest(lavaInfo, __COGLINK_GET_REQ, 0, 0, reqPath, sizeof(reqPath), 1, NULL, 0, res, 1, NULL);
}

int coglink_parseDecodeTrack(struct lavaInfo *lavaInfo, struct requestInformation *res, struct parsedTrack **songStruct) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTrackParseRequest[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTrackParseRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackParseRequest[i](lavaInfo, &res, &songStruct);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTrackParseRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackParseRequest[i](lavaInfoPlugin, &res, &songStruct);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    }
  }

  jsmn_parser parser;
  jsmntok_t tokens[32];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[32];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  char *path[] = { "info", "identifier" };
  jsmnf_pair *identifier = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "author";
  jsmnf_pair *author = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "length";
  jsmnf_pair *length = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "position";
  jsmnf_pair *position = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "title";
  jsmnf_pair *title = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pairs, res->body, path, 2);

  if (!identifier || !isSeekable || !author || !length || !isStream || !position || !title || !uri || !sourceName) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !identifier ? "identifier": !isSeekable ? "isSeekable" : !author ? "author" : !length ? "length" : !isStream ? "isStream" : !position ? "position" : !title ? "title" : !uri ? "uri" : !sourceName ? "sourceName" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  char Identifier[IDENTIFIER_LENGTH], IsSeekable[TRUE_FALSE_LENGTH], Author[AUTHOR_NAME_LENGTH], Length[VIDEO_LENGTH], IsStream[TRUE_FALSE_LENGTH], Position[VIDEO_LENGTH], Title[TRACK_TITLE_LENGTH], Uri[URL_LENGTH], SourceName[SOURCENAME_LENGTH];

  snprintf(Identifier, sizeof(Identifier), "%.*s", (int)identifier->v.len, res->body + identifier->v.pos);
  snprintf(IsSeekable, sizeof(IsSeekable), "%.*s", (int)isSeekable->v.len, res->body + isSeekable->v.pos);
  snprintf(Author, sizeof(Author), "%.*s", (int)author->v.len, res->body + author->v.pos);
  snprintf(Length, sizeof(Length), "%.*s", (int)length->v.len, res->body + length->v.pos);
  snprintf(IsStream, sizeof(IsStream), "%.*s", (int)isStream->v.len, res->body + isStream->v.pos);
  snprintf(Position, sizeof(Position), "%.*s", (int)position->v.len, res->body + position->v.pos);
  snprintf(Title, sizeof(Title), "%.*s", (int)title->v.len, res->body + title->v.pos);
  snprintf(Uri, sizeof(Uri), "%.*s", (int)uri->v.len, res->body + uri->v.pos);
  snprintf(SourceName, sizeof(SourceName), "%.*s", (int)sourceName->v.len, res->body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", Identifier, IsSeekable, Author, Length, IsStream, Position, Title, Uri, SourceName);

  *songStruct = malloc(sizeof(struct parsedTrack));

  (*songStruct)->identifier = malloc(sizeof(Identifier));
  (*songStruct)->isSeekable = malloc(sizeof(IsSeekable));
  (*songStruct)->author = malloc(sizeof(Author));
  (*songStruct)->length = malloc(sizeof(Length));
  (*songStruct)->isStream = malloc(sizeof(IsStream));
  (*songStruct)->position = malloc(sizeof(Position));
  (*songStruct)->title = malloc(sizeof(Title));
  (*songStruct)->uri = malloc(sizeof(Uri));
  (*songStruct)->sourceName = malloc(sizeof(SourceName));

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for song structure.", sizeof(struct parsedTrack) + sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName));

  strlcpy((*songStruct)->identifier, Identifier, IDENTIFIER_LENGTH);
  strlcpy((*songStruct)->isSeekable, IsSeekable, TRUE_FALSE_LENGTH);
  strlcpy((*songStruct)->author, Author, AUTHOR_NAME_LENGTH);
  strlcpy((*songStruct)->length, Length, VIDEO_LENGTH);
  strlcpy((*songStruct)->isStream, IsStream,TRUE_FALSE_LENGTH);
  strlcpy((*songStruct)->position, Position, VIDEO_LENGTH);
  strlcpy((*songStruct)->title, Title, TRACK_TITLE_LENGTH);
  strlcpy((*songStruct)->uri, Uri, URL_LENGTH);
  strlcpy((*songStruct)->sourceName, SourceName, SOURCENAME_LENGTH);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strlcpy] Copied %d bytes to song structure.", sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName));

  return COGLINK_SUCCESS;
}

int coglink_decodeTracks(struct lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct requestInformation *res) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTracksRequest[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTracksRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTracksRequest[i](lavaInfo, trackArray, trackArrayLength, &res);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTracksRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTracksRequest[i](lavaInfoPlugin, trackArray, trackArrayLength, &res);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    }
  }

  return __coglink_performRequest(lavaInfo, __COGLINK_GET_REQ, 0, 0, "/decodetracks", 1, 14, trackArray, strnlen(trackArray, 512 * trackArrayLength), res, 1, NULL);
}

int coglink_parseDecodeTracks(struct lavaInfo *lavaInfo, struct requestInformation *res, char *songPos, struct parsedTrack **songStruct) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTracksParseRequest[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTracksParseRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTracksParseRequest[i](lavaInfo, &res, songPos, &songStruct);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTracksParseRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTracksParseRequest[i](lavaInfoPlugin, &res, songPos, &songStruct);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    }
  }

  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  char *path[] = { songPos, "track", NULL };
  jsmnf_pair *track = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "info";
  path[2] = "identifier";
  jsmnf_pair *identifier = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "author";
  jsmnf_pair *author = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "length";
  jsmnf_pair *length = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "position";
  jsmnf_pair *position = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "title";
  jsmnf_pair *title = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pairs, res->body, path, 3);

  if (!track || !identifier || !isSeekable || !author || !length || !isStream || !position || !title || !uri || !sourceName) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !track ? "track" : !identifier ? "identifier": !isSeekable ? "isSeekable" : !author ? "author" : !length ? "length" : !isStream ? "isStream" : !position ? "position" : !title ? "title" : !uri ? "uri" : !sourceName ? "sourceName" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  char Track[TRACK_LENGTH], Identifier[IDENTIFIER_LENGTH], IsSeekable[TRUE_FALSE_LENGTH], Author[AUTHOR_NAME_LENGTH], Length[VIDEO_LENGTH], IsStream[TRUE_FALSE_LENGTH], Position[VIDEO_LENGTH], Title[TRACK_TITLE_LENGTH], Uri[URL_LENGTH], SourceName[SOURCENAME_LENGTH];

  snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, res->body + track->v.pos);
  snprintf(Identifier, sizeof(Identifier), "%.*s", (int)identifier->v.len, res->body + identifier->v.pos);
  snprintf(IsSeekable, sizeof(IsSeekable), "%.*s", (int)isSeekable->v.len, res->body + isSeekable->v.pos);
  snprintf(Author, sizeof(Author), "%.*s", (int)author->v.len, res->body + author->v.pos);
  snprintf(Length, sizeof(Length), "%.*s", (int)length->v.len, res->body + length->v.pos);
  snprintf(IsStream, sizeof(IsStream), "%.*s", (int)isStream->v.len, res->body + isStream->v.pos);
  snprintf(Position, sizeof(Position), "%.*s", (int)position->v.len, res->body + position->v.pos);
  snprintf(Title, sizeof(Title), "%.*s", (int)title->v.len, res->body + title->v.pos);
  snprintf(Uri, sizeof(Uri), "%.*s", (int)uri->v.len, res->body + uri->v.pos);
  snprintf(SourceName, sizeof(SourceName), "%.*s", (int)sourceName->v.len, res->body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> track: %s\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", Track, Identifier, IsSeekable, Author, Length, IsStream, Position, Title, Uri, SourceName);

  *songStruct = malloc(sizeof(struct parsedTrack));

  (*songStruct)->track = malloc(sizeof(Track));
  (*songStruct)->identifier = malloc(sizeof(Identifier));
  (*songStruct)->isSeekable = malloc(sizeof(IsSeekable));
  (*songStruct)->author = malloc(sizeof(Author));
  (*songStruct)->length = malloc(sizeof(Length));
  (*songStruct)->isStream = malloc(sizeof(IsStream));
  (*songStruct)->position = malloc(sizeof(Position));
  (*songStruct)->title = malloc(sizeof(Title));
  (*songStruct)->uri = malloc(sizeof(Uri));
  (*songStruct)->sourceName = malloc(sizeof(SourceName));

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for song structure.", sizeof(struct parsedTrack) + sizeof(Track) + sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName));

  strlcpy((*songStruct)->track, Track, TRACK_LENGTH);
  strlcpy((*songStruct)->identifier, Identifier, IDENTIFIER_LENGTH);
  strlcpy((*songStruct)->isSeekable, IsSeekable, TRUE_FALSE_LENGTH);
  strlcpy((*songStruct)->author, Author, AUTHOR_NAME_LENGTH);
  strlcpy((*songStruct)->length, Length, VIDEO_LENGTH);
  strlcpy((*songStruct)->isStream, IsStream,TRUE_FALSE_LENGTH);
  strlcpy((*songStruct)->position, Position, VIDEO_LENGTH);
  strlcpy((*songStruct)->title, Title, TRACK_TITLE_LENGTH);
  strlcpy((*songStruct)->uri, Uri, URL_LENGTH);
  strlcpy((*songStruct)->sourceName, SourceName, SOURCENAME_LENGTH);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strlcpy] Copied %d bytes to song structure.", sizeof(Track) + sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName));

  return COGLINK_SUCCESS;
}

int coglink_getLavalinkInfo(struct lavaInfo *lavaInfo, struct requestInformation *res) {
  return __coglink_performRequest(lavaInfo, __COGLINK_GET_REQ, 0, 0, "/info", 1, 6, NULL, 0, res, 1, NULL);
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

  *lavalinkInfoStruct = malloc(sizeof(struct lavalinkInfo));

  (*lavalinkInfoStruct)->version = malloc(sizeof(struct lavalinkInfoVersion));

  (*lavalinkInfoStruct)->version->semver = malloc(sizeof(Semver));
  (*lavalinkInfoStruct)->version->major = malloc(sizeof(Major));
  (*lavalinkInfoStruct)->version->minor = malloc(sizeof(Minor));
  (*lavalinkInfoStruct)->version->patch = malloc(sizeof(Patch));
  if (preRelease) (*lavalinkInfoStruct)->version->preRelease = malloc(sizeof(PreRelease));

  (*lavalinkInfoStruct)->buildTime = malloc(sizeof(BuildTime));

  (*lavalinkInfoStruct)->git = malloc(sizeof(struct lavalinkInfoGit));

  (*lavalinkInfoStruct)->git->branch = malloc(sizeof(Branch));
  (*lavalinkInfoStruct)->git->commit = malloc(sizeof(Commit));
  (*lavalinkInfoStruct)->git->commitTime = malloc(sizeof(CommitTime));

  (*lavalinkInfoStruct)->jvm = malloc(sizeof(Jvm));
  (*lavalinkInfoStruct)->lavaplayer = malloc(sizeof(Lavaplayer));
  (*lavalinkInfoStruct)->sourceManagers = malloc(sizeof(SourceManagers));
  (*lavalinkInfoStruct)->filters = malloc(sizeof(Filters));
  (*lavalinkInfoStruct)->plugins = malloc(sizeof(Plugins));

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Allocated %d bytes for info structure.", sizeof(struct lavalinkInfo) + sizeof(Semver) + sizeof(Major) + sizeof(Minor) + sizeof(Patch) + sizeof(PreRelease) + sizeof(BuildTime) + sizeof(Branch) + sizeof(Commit) + sizeof(CommitTime) + sizeof(Jvm) + sizeof(Lavaplayer) + sizeof(SourceManagers) + sizeof(Filters) + sizeof(Plugins));

  strlcpy((*lavalinkInfoStruct)->version->semver, Semver, 8);
  strlcpy((*lavalinkInfoStruct)->version->major, Major, 4);
  strlcpy((*lavalinkInfoStruct)->version->minor, Minor, 4);
  strlcpy((*lavalinkInfoStruct)->version->patch, Patch, 4);
  if (preRelease) strlcpy((*lavalinkInfoStruct)->version->preRelease, PreRelease, 8);
  strlcpy((*lavalinkInfoStruct)->buildTime, BuildTime, 32); 
  strlcpy((*lavalinkInfoStruct)->git->branch, Branch, 16);
  strlcpy((*lavalinkInfoStruct)->git->commit, Commit, 32);
  strlcpy((*lavalinkInfoStruct)->git->commitTime, CommitTime, 16);
  strlcpy((*lavalinkInfoStruct)->jvm, Jvm, 8);
  strlcpy((*lavalinkInfoStruct)->lavaplayer, Lavaplayer, 8);
  strlcpy((*lavalinkInfoStruct)->sourceManagers, SourceManagers, 128);
  strlcpy((*lavalinkInfoStruct)->filters, Filters, 128);
  strlcpy((*lavalinkInfoStruct)->plugins, Plugins, 128);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Copied %s bytes to info structure.", sizeof(struct lavalinkInfo) + sizeof(Semver) + sizeof(Major) + sizeof(Minor) + sizeof(Patch) + sizeof(PreRelease) + sizeof(BuildTime) + sizeof(Branch) + sizeof(Commit) + sizeof(CommitTime) + sizeof(Jvm) + sizeof(Lavaplayer) + sizeof(SourceManagers) + sizeof(Filters) + sizeof(Plugins));

  return COGLINK_SUCCESS;
}

int coglink_getLavalinkStats(struct lavaInfo *lavaInfo, struct requestInformation *res) {
  return __coglink_performRequest(lavaInfo, __COGLINK_GET_REQ, 0, 0, "/stats", 1, 6, NULL, 0, res, 1, NULL);
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

  *lavalinkStatsStruct = malloc(sizeof(struct lavalinkStats));

  (*lavalinkStatsStruct)->players = malloc(sizeof(Players));
  (*lavalinkStatsStruct)->playingPlayers = malloc(sizeof(PlayingPlayers));
  (*lavalinkStatsStruct)->uptime = malloc(sizeof(Uptime));

  (*lavalinkStatsStruct)->memory = malloc(sizeof(struct lavalinkStatsMemory));

  (*lavalinkStatsStruct)->memory->free = malloc(sizeof(Free));
  (*lavalinkStatsStruct)->memory->used = malloc(sizeof(Used));
  (*lavalinkStatsStruct)->memory->allocated = malloc(sizeof(Allocated));
  (*lavalinkStatsStruct)->memory->reservable = malloc(sizeof(Reservable));

  (*lavalinkStatsStruct)->cpu = malloc(sizeof(struct lavalinkStatsCPU));

  (*lavalinkStatsStruct)->cpu->cores = malloc(sizeof(Cores));
  (*lavalinkStatsStruct)->cpu->systemLoad = malloc(sizeof(SystemLoad));
  (*lavalinkStatsStruct)->cpu->lavalinkLoad = malloc(sizeof(LavalinkLoad));

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Allocated %d bytes for stats structure.", sizeof(struct lavalinkStats) + sizeof(struct lavalinkStatsMemory) + sizeof(struct lavalinkStatsCPU) + sizeof(Players) + sizeof(PlayingPlayers) + sizeof(Uptime) + sizeof(Free) + sizeof(Used) + sizeof(Allocated) + sizeof(Reservable) + sizeof(Cores) + sizeof(SystemLoad) + sizeof(LavalinkLoad));

  strlcpy((*lavalinkStatsStruct)->players, Players, 8);
  strlcpy((*lavalinkStatsStruct)->playingPlayers, PlayingPlayers, 8);
  strlcpy((*lavalinkStatsStruct)->uptime, Uptime, 32);
  strlcpy((*lavalinkStatsStruct)->memory->free, Free, 16);
  strlcpy((*lavalinkStatsStruct)->memory->used, Used, 16);
  strlcpy((*lavalinkStatsStruct)->memory->allocated, Allocated, 16);
  strlcpy((*lavalinkStatsStruct)->memory->reservable, Reservable, 16);
  strlcpy((*lavalinkStatsStruct)->cpu->cores, Cores, 8);
  strlcpy((*lavalinkStatsStruct)->cpu->systemLoad, SystemLoad, 16);
  strlcpy((*lavalinkStatsStruct)->cpu->lavalinkLoad, LavalinkLoad, 16);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Copied %d bytes to stats structure.", sizeof(Players) + sizeof(PlayingPlayers) + sizeof(Uptime) + sizeof(Free) + sizeof(Used) + sizeof(Allocated) + sizeof(Reservable) + sizeof(Cores) + sizeof(SystemLoad) + sizeof(LavalinkLoad));

  return COGLINK_SUCCESS;
}

int coglink_getRouterPlanner(struct lavaInfo *lavaInfo, struct requestInformation *res) {
  return __coglink_performRequest(lavaInfo, __COGLINK_GET_REQ, 0, 0, "/routeplanner/status", 1, 21, NULL, 0, res, 1, NULL);
}

int coglink_parseRouterPlanner(struct lavaInfo *lavaInfo, struct requestInformation *res, char *ipPosition, struct lavalinkRouter **lavaRouterStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[128];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[128];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  jsmnf_pair *class = jsmnf_find(pairs, res->body, "class", 5);
  if (__coglink_checkParse(lavaInfo, class, "class") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  char Class[ROUTERPLANNER_CLASS_LENGTH];
  snprintf(Class, sizeof(Class), "%.*s", (int)class->v.len, res->body + class->v.pos);

  if (Class[0] == 'n') return COGLINK_ROUTERPLANNER_NOT_SET;

  char *path[] = { "details", "ipBlock", "type", NULL };
  jsmnf_pair *type = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "size";
  jsmnf_pair *size = jsmnf_find_path(pairs, res->body, path, 3);

  path[1] = "failingAddresses";
  path[2] = ipPosition;
  path[3] = "address";
  jsmnf_pair *address = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "failingTimestamp";
  jsmnf_pair *failingTimestamp = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "failingTime";
  jsmnf_pair *failingTime = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = NULL;
  path[2] = NULL;

  if (!type || !size || !address || !failingTimestamp || !failingTime) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !type ? "type" : !size ? "size" : !address ? "address" : !failingTimestamp ? "failingTimestamp" : !failingTime ? "failingTime" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  if (Class[0] == 'R') {
    path[1] = "rotateIndex";
    jsmnf_pair *rotateIndex = jsmnf_find_path(pairs, res->body, path, 2);

    path[1] = "ipIndex";
    jsmnf_pair *ipIndex = jsmnf_find_path(pairs, res->body, path, 2);

    path[1] = "currentAddress";
    jsmnf_pair *currentAddress = jsmnf_find_path(pairs, res->body, path, 2);

    if (!rotateIndex || !ipIndex || !currentAddress) {
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !rotateIndex ? "rotateIndex" : !ipIndex ? "ipIndex" : !currentAddress ? "currentAddress" : "???");
      return COGLINK_JSMNF_ERROR_FIND;
    }

    char Type[16], Size[16], Address[8], FailingTimestamp[16], FailingTime[16], RotateIndex[16], IpIndex[16], CurrentAddress[8];

    snprintf(Type, sizeof(Type), "%.*s", (int)type->v.len, res->body + type->v.pos);
    snprintf(Size, sizeof(Size), "%.*s", (int)size->v.len, res->body + size->v.pos);
    snprintf(Address, sizeof(Address), "%.*s", (int)address->v.len, res->body + address->v.pos);
    snprintf(FailingTimestamp, sizeof(FailingTimestamp), "%.*s", (int)failingTimestamp->v.len, res->body + failingTimestamp->v.pos);
    snprintf(FailingTime, sizeof(FailingTime), "%.*s", (int)failingTime->v.len, res->body + failingTime->v.pos);
    snprintf(RotateIndex, sizeof(RotateIndex), "%.*s", (int)rotateIndex->v.len, res->body + rotateIndex->v.pos);
    snprintf(IpIndex, sizeof(IpIndex), "%.*s", (int)ipIndex->v.len, res->body + ipIndex->v.pos);
    snprintf(CurrentAddress, sizeof(CurrentAddress), "%.*s", (int)currentAddress->v.len, res->body + currentAddress->v.pos);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> class: %s\n> type: %s\n> size: %s\n> address: %s\n> failingTimestamp: %s\n> failingTime: %s\n> rotateIndex: %s\n> ipIndex: %s\n> currentAddress: %s\n", Class, Type, Size, Address, FailingTimestamp, FailingTime, RotateIndex, IpIndex, CurrentAddress);

    *lavaRouterStruct = malloc(sizeof(struct lavalinkRouter));

    (*lavaRouterStruct)->class = malloc(sizeof(Class));
    (*lavaRouterStruct)->details = malloc(sizeof(struct lavalinkRouterDetails));

    (*lavaRouterStruct)->details->ipBlock = malloc(sizeof(struct lavalinkDetailsIpBlock));

    (*lavaRouterStruct)->details->ipBlock->type = malloc(sizeof(Type));
    (*lavaRouterStruct)->details->ipBlock->size = malloc(sizeof(Size));

    (*lavaRouterStruct)->details->failingAddress = malloc(sizeof(struct lavalinkDetailsFailingAddress));

    (*lavaRouterStruct)->details->failingAddress->address = malloc(sizeof(Address));
    (*lavaRouterStruct)->details->failingAddress->failingTimestamp = malloc(sizeof(FailingTimestamp));
    (*lavaRouterStruct)->details->failingAddress->failingTime = malloc(sizeof(FailingTime));

    (*lavaRouterStruct)->details->rotateIndex = malloc(sizeof(RotateIndex));
    (*lavaRouterStruct)->details->ipIndex = malloc(sizeof(IpIndex));
    (*lavaRouterStruct)->details->currentAddress = malloc(sizeof(CurrentAddress));

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Allocated %d bytes for router structure.", sizeof(struct lavalinkRouter) + sizeof(Class) + sizeof(struct lavalinkRouterDetails) + sizeof(struct lavalinkDetailsIpBlock) + sizeof(struct lavalinkDetailsFailingAddress) + sizeof(Type) + sizeof(Size) + sizeof(RotateIndex) + sizeof(IpIndex) + sizeof(CurrentAddress) + sizeof(Address) + sizeof(FailingTimestamp) + sizeof(FailingTime));

    strlcpy((*lavaRouterStruct)->class, Class, ROUTERPLANNER_CLASS_LENGTH);
    strlcpy((*lavaRouterStruct)->details->ipBlock->type, Type, 16);
    strlcpy((*lavaRouterStruct)->details->ipBlock->size, Size, 16);
    strlcpy((*lavaRouterStruct)->details->failingAddress->address, Address, 8);
    strlcpy((*lavaRouterStruct)->details->failingAddress->failingTimestamp, FailingTimestamp, 16);
    strlcpy((*lavaRouterStruct)->details->failingAddress->failingTime, FailingTime, 16);
    strlcpy((*lavaRouterStruct)->details->rotateIndex, RotateIndex, 16);
    strlcpy((*lavaRouterStruct)->details->ipIndex, IpIndex, 16);
    strlcpy((*lavaRouterStruct)->details->currentAddress, CurrentAddress, 8);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Copied %s bytes to router structure.", sizeof(Class) + sizeof(Type) + sizeof(Size) + sizeof(Address) + sizeof(FailingTimestamp) + sizeof(FailingTime) + sizeof(RotateIndex) + sizeof(IpIndex) + sizeof(CurrentAddress));
  } else if (Class[0] == 'N') {
    path[1] = "currentAddressIndex";
    jsmnf_pair *currentAddressIndex = jsmnf_find_path(pairs, res->body, path, 2);

    if (!currentAddressIndex) {
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find currentAddressIndex field.");
      return COGLINK_JSMNF_ERROR_FIND;
    }

    char Type[16], Size[16], Address[8], FailingTimestamp[16], FailingTime[16], CurrentAddressIndex[16];

    snprintf(Type, sizeof(Type), "%.*s", (int)type->v.len, res->body + type->v.pos);
    snprintf(Size, sizeof(Size), "%.*s", (int)size->v.len, res->body + size->v.pos);
    snprintf(Address, sizeof(Address), "%.*s", (int)address->v.len, res->body + address->v.pos);
    snprintf(FailingTimestamp, sizeof(FailingTimestamp), "%.*s", (int)failingTimestamp->v.len, res->body + failingTimestamp->v.pos);
    snprintf(FailingTime, sizeof(FailingTime), "%.*s", (int)failingTime->v.len, res->body + failingTime->v.pos);
    snprintf(CurrentAddressIndex, sizeof(CurrentAddressIndex), "%.*s", (int)currentAddressIndex->v.len, res->body + currentAddressIndex->v.pos);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> class: %s\n> type: %s\n> size: %s\n> address: %s\n> failingTimestamp: %s\n> failingTime: %s\n> currentAddressIndex: %s\n", Class, Type, Size, Address, FailingTimestamp, FailingTime, CurrentAddressIndex);

    *lavaRouterStruct = malloc(sizeof(struct lavalinkRouter));

    (*lavaRouterStruct)->class = malloc(sizeof(Class));
    (*lavaRouterStruct)->details = malloc(sizeof(struct lavalinkRouterDetails));

    (*lavaRouterStruct)->details->ipBlock = malloc(sizeof(struct lavalinkDetailsIpBlock));

    (*lavaRouterStruct)->details->ipBlock->type = malloc(sizeof(Type));
    (*lavaRouterStruct)->details->ipBlock->size = malloc(sizeof(Size));

    (*lavaRouterStruct)->details->failingAddress = malloc(sizeof(struct lavalinkDetailsFailingAddress));

    (*lavaRouterStruct)->details->failingAddress->address = malloc(sizeof(Address));
    (*lavaRouterStruct)->details->failingAddress->failingTimestamp = malloc(sizeof(FailingTimestamp));
    (*lavaRouterStruct)->details->failingAddress->failingTime = malloc(sizeof(FailingTime));

    (*lavaRouterStruct)->details->currentAddressIndex = malloc(sizeof(CurrentAddressIndex));

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Allocated %d bytes for router structure.", sizeof(struct lavalinkRouter) + sizeof(Class) + sizeof(struct lavalinkRouterDetails) + sizeof(struct lavalinkDetailsIpBlock) + sizeof(struct lavalinkDetailsFailingAddress) + sizeof(Type) + sizeof(Size) + sizeof(CurrentAddressIndex) + sizeof(Address) + sizeof(FailingTimestamp) + sizeof(FailingTime));

    strlcpy((*lavaRouterStruct)->class, Class, ROUTERPLANNER_CLASS_LENGTH);
    strlcpy((*lavaRouterStruct)->details->ipBlock->type, Type, 16);
    strlcpy((*lavaRouterStruct)->details->ipBlock->size, Size, 16);
    strlcpy((*lavaRouterStruct)->details->failingAddress->address, Address, 8);
    strlcpy((*lavaRouterStruct)->details->failingAddress->failingTimestamp, FailingTimestamp, 16);
    strlcpy((*lavaRouterStruct)->details->failingAddress->failingTime, FailingTime, 16);
    strlcpy((*lavaRouterStruct)->details->currentAddressIndex, CurrentAddressIndex, 16);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Copied %s bytes to router structure.", sizeof(Class) + sizeof(Type) + sizeof(Size) + sizeof(Address) + sizeof(FailingTimestamp) + sizeof(FailingTime) + sizeof(CurrentAddressIndex));
  } else {
    path[1] = "blockIndex";
    jsmnf_pair *blockIndex = jsmnf_find_path(pairs, res->body, path, 2);

    path[1] = "currentAddressIndex";
    jsmnf_pair *currentAddressIndex = jsmnf_find_path(pairs, res->body, path, 2);

    if (!currentAddressIndex) {
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find currentAddressIndex field.");
      return COGLINK_JSMNF_ERROR_FIND;
    }

    char Type[16], Size[16], Address[8], FailingTimestamp[16], FailingTime[16], CurrentAddressIndex[16], BlockIndex[16];

    snprintf(Type, sizeof(Type), "%.*s", (int)type->v.len, res->body + type->v.pos);
    snprintf(Size, sizeof(Size), "%.*s", (int)size->v.len, res->body + size->v.pos);
    snprintf(Address, sizeof(Address), "%.*s", (int)address->v.len, res->body + address->v.pos);
    snprintf(FailingTimestamp, sizeof(FailingTimestamp), "%.*s", (int)failingTimestamp->v.len, res->body + failingTimestamp->v.pos);
    snprintf(FailingTime, sizeof(FailingTime), "%.*s", (int)failingTime->v.len, res->body + failingTime->v.pos);
    snprintf(BlockIndex, sizeof(BlockIndex), "%.*s", (int)blockIndex->v.len, res->body + blockIndex->v.pos);
    snprintf(CurrentAddressIndex, sizeof(CurrentAddressIndex), "%.*s", (int)currentAddressIndex->v.len, res->body + currentAddressIndex->v.pos);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> class: %s\n> type: %s\n> size: %s\n> address: %s\n> failingTimestamp: %s\n> failingTime: %s\n> blockIndex: %s\n> currentAddressIndex: %s\n", Class, Type, Size, Address, FailingTimestamp, FailingTime, BlockIndex, CurrentAddressIndex);

    *lavaRouterStruct = malloc(sizeof(struct lavalinkRouter));

    (*lavaRouterStruct)->class = malloc(sizeof(Class));
    (*lavaRouterStruct)->details = malloc(sizeof(struct lavalinkRouterDetails));

    (*lavaRouterStruct)->details->ipBlock = malloc(sizeof(struct lavalinkDetailsIpBlock));

    (*lavaRouterStruct)->details->ipBlock->type = malloc(sizeof(Type));
    (*lavaRouterStruct)->details->ipBlock->size = malloc(sizeof(Size));

    (*lavaRouterStruct)->details->failingAddress->address = malloc(sizeof(Address));
    (*lavaRouterStruct)->details->failingAddress->failingTimestamp = malloc(sizeof(FailingTimestamp));
    (*lavaRouterStruct)->details->failingAddress->failingTime = malloc(sizeof(FailingTime));
      
    (*lavaRouterStruct)->details->blockIndex = malloc(sizeof(BlockIndex));
    (*lavaRouterStruct)->details->currentAddressIndex = malloc(sizeof(CurrentAddressIndex));

    (*lavaRouterStruct)->details->failingAddress = malloc(sizeof(struct lavalinkDetailsFailingAddress));
      
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Allocated %d bytes for router structure.", sizeof(struct lavalinkRouter) + sizeof(Class) + sizeof(struct lavalinkRouterDetails) + sizeof(struct lavalinkDetailsIpBlock) + sizeof(struct lavalinkDetailsFailingAddress) + sizeof(Type) + sizeof(Size) + sizeof(BlockIndex) + sizeof(CurrentAddressIndex) + sizeof(Address) + sizeof(FailingTimestamp) + sizeof(FailingTime));

    strlcpy((*lavaRouterStruct)->class, Class, ROUTERPLANNER_CLASS_LENGTH);
    strlcpy((*lavaRouterStruct)->details->ipBlock->type, Type, 16);
    strlcpy((*lavaRouterStruct)->details->ipBlock->size, Size, 16);
    strlcpy((*lavaRouterStruct)->details->failingAddress->address, Address, 8);
    strlcpy((*lavaRouterStruct)->details->failingAddress->failingTimestamp, FailingTimestamp, 16);
    strlcpy((*lavaRouterStruct)->details->failingAddress->failingTime, FailingTime, 16);
    strlcpy((*lavaRouterStruct)->details->blockIndex, BlockIndex, 16);
    strlcpy((*lavaRouterStruct)->details->currentAddressIndex, CurrentAddressIndex, 16);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Copied %s bytes to router structure.", sizeof(Class) + sizeof(Type) + sizeof(Size) + sizeof(Address) + sizeof(FailingTimestamp) + sizeof(FailingTime) + sizeof(BlockIndex) + sizeof(CurrentAddressIndex));
  }

  return COGLINK_SUCCESS;
}

int coglink_freeFailingAddress(struct lavaInfo *lavaInfo, char *ip) {
  char payload[32];
  snprintf(payload, sizeof(payload), "{\"address\":\"%s\"}", ip);

  return __coglink_performRequest(lavaInfo, __COGLINK_POST_REQ, 0, 0, "/routeplanner/free/address", 1, 27, payload, (long)strnlen(payload, 32), NULL, 0, NULL);
}

int coglink_freeFailingAllAddresses(struct lavaInfo *lavaInfo) {
  return __coglink_performRequest(lavaInfo, __COGLINK_POST_REQ, 0, 0, "/routeplanner/free/address", 1, 27, NULL, 0, NULL, 0, NULL);
}

void coglink_parseDecodeTrackCleanup(struct lavaInfo *lavaInfo, struct parsedTrack *songStruct) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTrackCleanup[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTrackCleanup[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackCleanup[i](lavaInfo, &songStruct);
        if (pluginResultCode != COGLINK_PROCEED) return;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTrackCleanup[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackCleanup[i](lavaInfoPlugin, &songStruct);
        if (pluginResultCode != COGLINK_PROCEED) return;
      }
    }
  }

  if (songStruct->track) free(songStruct->track);
  if (songStruct->identifier) free(songStruct->identifier);
  if (songStruct->isSeekable) free(songStruct->isSeekable);
  if (songStruct->author) free(songStruct->author);
  if (songStruct->length) free(songStruct->length);
  if (songStruct->isStream) free(songStruct->isStream);
  if (songStruct->position) free(songStruct->position);
  if (songStruct->title) free(songStruct->title);
  if (songStruct->uri) free(songStruct->uri);
  if (songStruct->sourceName) free(songStruct->sourceName);
  if (songStruct) free(songStruct);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-free] Freed %d bytes from song structure.", sizeof(songStruct->track) + sizeof(songStruct->identifier) + sizeof(songStruct->isSeekable) + sizeof(songStruct->author) + sizeof(songStruct->length) + sizeof(songStruct->isStream) + sizeof(songStruct->position) + sizeof(songStruct->title) + sizeof(songStruct->uri) + sizeof(songStruct->sourceName) + sizeof(struct parsedTrack));
}

void coglink_parseLavalinkInfoCleanup(struct lavaInfo *lavaInfo, struct lavalinkInfo *lavalinkInfoStruct) {
  if (lavalinkInfoStruct->version) {
    free(lavalinkInfoStruct->version->semver);
    free(lavalinkInfoStruct->version->major);
    free(lavalinkInfoStruct->version->minor);
    free(lavalinkInfoStruct->version->patch);
    free(lavalinkInfoStruct->version->preRelease);
    free(lavalinkInfoStruct->version);
  }
  if (lavalinkInfoStruct->buildTime) free(lavalinkInfoStruct->buildTime);
  if (lavalinkInfoStruct->git) {
    free(lavalinkInfoStruct->git->branch);
    free(lavalinkInfoStruct->git->commit);
    free(lavalinkInfoStruct->git->commitTime);
    free(lavalinkInfoStruct->git);
  }
  if (lavalinkInfoStruct->jvm) free(lavalinkInfoStruct->jvm);
  if (lavalinkInfoStruct->lavaplayer) free(lavalinkInfoStruct->lavaplayer);
  if (lavalinkInfoStruct->sourceManagers) free(lavalinkInfoStruct->sourceManagers);
  if (lavalinkInfoStruct->filters) free(lavalinkInfoStruct->filters);
  if (lavalinkInfoStruct->plugins) free(lavalinkInfoStruct->plugins);
  if (lavalinkInfoStruct) free(lavalinkInfoStruct);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-free] Freed %d bytes from lavalink info structure.", sizeof(lavalinkInfoStruct->version->semver) + sizeof(lavalinkInfoStruct->version->major) + sizeof(lavalinkInfoStruct->version->minor) + sizeof(lavalinkInfoStruct->version->patch) + sizeof(lavalinkInfoStruct->version->preRelease) + sizeof(lavalinkInfoStruct->version) + sizeof(lavalinkInfoStruct->buildTime) + sizeof(lavalinkInfoStruct->git->branch) + sizeof(lavalinkInfoStruct->git->commit) + sizeof(lavalinkInfoStruct->git->commitTime) + sizeof(lavalinkInfoStruct->git) + sizeof(lavalinkInfoStruct->jvm) + sizeof(lavalinkInfoStruct->lavaplayer) + sizeof(lavalinkInfoStruct->sourceManagers) + sizeof(lavalinkInfoStruct->filters) + sizeof(lavalinkInfoStruct->plugins) + sizeof(lavalinkInfoStruct));
}

void coglink_parseLavalinkStatsCleanup(struct lavaInfo *lavaInfo, struct lavalinkStats *lavalinkStatsStruct) {
  if (lavalinkStatsStruct->players) free(lavalinkStatsStruct->players);
  if (lavalinkStatsStruct->playingPlayers) free(lavalinkStatsStruct->playingPlayers);
  if (lavalinkStatsStruct->uptime) free(lavalinkStatsStruct->uptime);
  if (lavalinkStatsStruct->memory) {
    free(lavalinkStatsStruct->memory->used);
    free(lavalinkStatsStruct->memory->free);
    free(lavalinkStatsStruct->memory->allocated);
    free(lavalinkStatsStruct->memory->reservable);
    free(lavalinkStatsStruct->memory);
  }
  if (lavalinkStatsStruct->cpu) {
    free(lavalinkStatsStruct->cpu->cores);
    free(lavalinkStatsStruct->cpu->systemLoad);
    free(lavalinkStatsStruct->cpu->lavalinkLoad);
    free(lavalinkStatsStruct->cpu);
  }
  if (lavalinkStatsStruct->frameStats) {
    free(lavalinkStatsStruct->frameStats->sent);
    free(lavalinkStatsStruct->frameStats->nulled);
    free(lavalinkStatsStruct->frameStats->deficit);
    free(lavalinkStatsStruct->frameStats);
  }
  if (lavalinkStatsStruct) free(lavalinkStatsStruct);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-free] Freed %d bytes from lavalink stats structure.", sizeof(lavalinkStatsStruct->players) + sizeof(lavalinkStatsStruct->playingPlayers) + sizeof(lavalinkStatsStruct->uptime) + sizeof(lavalinkStatsStruct->memory->used) + sizeof(lavalinkStatsStruct->memory->free) + sizeof(lavalinkStatsStruct->memory->allocated) + sizeof(lavalinkStatsStruct->memory->reservable) + sizeof(lavalinkStatsStruct->memory) + sizeof(lavalinkStatsStruct->cpu->cores) + sizeof(lavalinkStatsStruct->cpu->systemLoad) + sizeof(lavalinkStatsStruct->cpu->lavalinkLoad) + sizeof(lavalinkStatsStruct->cpu) + sizeof(lavalinkStatsStruct->frameStats->sent) + sizeof(lavalinkStatsStruct->frameStats->nulled) + sizeof(lavalinkStatsStruct->frameStats->deficit) + sizeof(lavalinkStatsStruct->frameStats) + sizeof(lavalinkStatsStruct));
}

void coglink_parseRouterPlannerCleanup(struct lavaInfo *lavaInfo, struct lavalinkRouter *lavaRouterStruct) {
  if (lavaRouterStruct->class) free(lavaRouterStruct->class);
  if (lavaRouterStruct->details->ipBlock) {
    free(lavaRouterStruct->details->ipBlock->type);
    free(lavaRouterStruct->details->ipBlock->size);
    free(lavaRouterStruct->details->ipBlock);
  }
  if (lavaRouterStruct->details->failingAddress) {
    free(lavaRouterStruct->details->failingAddress->address);
    free(lavaRouterStruct->details->failingAddress->failingTimestamp);
    free(lavaRouterStruct->details->failingAddress->failingTime);
    free(lavaRouterStruct->details->failingAddress);
  }
  if (lavaRouterStruct->class[0] == 'R') {
    if (lavaRouterStruct->details->rotateIndex) free(lavaRouterStruct->details->rotateIndex);
    if (lavaRouterStruct->details->ipIndex) free(lavaRouterStruct->details->ipIndex);
    if (lavaRouterStruct->details->currentAddress) free(lavaRouterStruct->details->currentAddress);
    if (lavaRouterStruct->details) free(lavaRouterStruct->details);
    if (lavaRouterStruct) free(lavaRouterStruct);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:jsmn-find] Freed %d bytes from router structure.", sizeof(lavaRouterStruct->class) + sizeof(lavaRouterStruct->details->ipBlock->type) + sizeof(lavaRouterStruct->details->ipBlock->size) + sizeof(lavaRouterStruct->details->ipBlock) + sizeof(lavaRouterStruct->details->failingAddress->address) + sizeof(lavaRouterStruct->details->failingAddress->failingTimestamp) + sizeof(lavaRouterStruct->details->failingAddress->failingTime) + sizeof(lavaRouterStruct->details->failingAddress) + sizeof(lavaRouterStruct->details->rotateIndex) + sizeof(lavaRouterStruct->details->ipIndex) + sizeof(lavaRouterStruct->details->currentAddress) + sizeof(lavaRouterStruct->details) + sizeof(lavaRouterStruct));
  } else if (lavaRouterStruct->class[0] == 'N') {
    if (lavaRouterStruct->details->blockIndex) free(lavaRouterStruct->details->blockIndex);
    if (lavaRouterStruct->details->currentAddressIndex) free(lavaRouterStruct->details->currentAddressIndex);
    if (lavaRouterStruct->details) free(lavaRouterStruct->details);
    if (lavaRouterStruct) free(lavaRouterStruct);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:jsmn-find] Freed %d bytes from router structure.", sizeof(lavaRouterStruct->class) + sizeof(lavaRouterStruct->details->ipBlock->type) + sizeof(lavaRouterStruct->details->ipBlock->size) + sizeof(lavaRouterStruct->details->ipBlock) + sizeof(lavaRouterStruct->details->failingAddress->address) + sizeof(lavaRouterStruct->details->failingAddress->failingTimestamp) + sizeof(lavaRouterStruct->details->failingAddress->failingTime) + sizeof(lavaRouterStruct->details->failingAddress) + sizeof(lavaRouterStruct->details->blockIndex) + sizeof(lavaRouterStruct->details->currentAddressIndex) + sizeof(lavaRouterStruct->details) + sizeof(lavaRouterStruct));
  } else {
    if (lavaRouterStruct->details->blockIndex) free(lavaRouterStruct->details->blockIndex);
    if (lavaRouterStruct->details->currentAddressIndex) free(lavaRouterStruct->details->currentAddressIndex);
    if (lavaRouterStruct->details) free(lavaRouterStruct->details);
    if (lavaRouterStruct) free(lavaRouterStruct);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:jsmn-find] Freed %d bytes from router structure.", sizeof(lavaRouterStruct->class) + sizeof(lavaRouterStruct->details->ipBlock->type) + sizeof(lavaRouterStruct->details->ipBlock->size) + sizeof(lavaRouterStruct->details->ipBlock) + sizeof(lavaRouterStruct->details->failingAddress->address) + sizeof(lavaRouterStruct->details->failingAddress->failingTimestamp) + sizeof(lavaRouterStruct->details->failingAddress->failingTime) + sizeof(lavaRouterStruct->details->failingAddress) + sizeof(lavaRouterStruct->details->blockIndex) + sizeof(lavaRouterStruct->details->currentAddressIndex) + sizeof(lavaRouterStruct->details) + sizeof(lavaRouterStruct));
  }
}
