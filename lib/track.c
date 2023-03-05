#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>

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

  char reqPath[512 + 26];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/decodetrack?encodedTrack=%s", track);

  return __coglink_performRequest(lavaInfo, res, &(struct __coglink_requestConfig) {
                                                    .requestType = __COGLINK_GET_REQ,
                                                    .path = reqPath,
                                                    .pathLength = pathLen,
                                                    .useVPath = true,
                                                    .getResponse = true
                                                  });
}

int coglink_parseDecodeTrack(struct lavaInfo *lavaInfo, struct requestInformation *res, struct parsedTrack *parsedTrackStruct) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTrackParseRequest[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTrackParseRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackParseRequest[i](lavaInfo, &res, &parsedTrackStruct);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTrackParseRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackParseRequest[i](lavaInfoPlugin, &res, &parsedTrackStruct);
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

  path[1] = "isrc";
  jsmnf_pair *isrc = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "artworkUrl";
  jsmnf_pair *artworkUrl = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pairs, res->body, path, 2);

  if (!identifier || !isSeekable || !author || !length || !isStream || !position || !title || !uri || !sourceName) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !identifier ? "identifier": !isSeekable ? "isSeekable" : !author ? "author" : !length ? "length" : !isStream ? "isStream" : !position ? "position" : !title ? "title" : !uri ? "uri" : !sourceName ? "sourceName" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  snprintf(parsedTrackStruct->identifier, sizeof(parsedTrackStruct->identifier), "%.*s", (int)identifier->v.len, res->body + identifier->v.pos);
  snprintf(parsedTrackStruct->isSeekable, sizeof(parsedTrackStruct->isSeekable), "%.*s", (int)isSeekable->v.len, res->body + isSeekable->v.pos);
  snprintf(parsedTrackStruct->author, sizeof(parsedTrackStruct->author), "%.*s", (int)author->v.len, res->body + author->v.pos);
  snprintf(parsedTrackStruct->length, sizeof(parsedTrackStruct->length), "%.*s", (int)length->v.len, res->body + length->v.pos);
  snprintf(parsedTrackStruct->isStream, sizeof(parsedTrackStruct->isStream), "%.*s", (int)isStream->v.len, res->body + isStream->v.pos);
  snprintf(parsedTrackStruct->position, sizeof(parsedTrackStruct->position), "%.*s", (int)position->v.len, res->body + position->v.pos);
  snprintf(parsedTrackStruct->title, sizeof(parsedTrackStruct->title), "%.*s", (int)title->v.len, res->body + title->v.pos);
  snprintf(parsedTrackStruct->uri, sizeof(parsedTrackStruct->uri), "%.*s", (int)uri->v.len, res->body + uri->v.pos);
  if (isrc) snprintf(parsedTrackStruct->isrc, sizeof(parsedTrackStruct->isrc), "%.*s", (int)isrc->v.len, res->body + isrc->v.pos);
  if (artworkUrl) snprintf(parsedTrackStruct->artworkUrl, sizeof(parsedTrackStruct->artworkUrl), "%.*s", (int)artworkUrl->v.len, res->body + artworkUrl->v.pos);
  snprintf(parsedTrackStruct->sourceName, sizeof(parsedTrackStruct->sourceName), "%.*s", (int)sourceName->v.len, res->body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", parsedTrackStruct->identifier, parsedTrackStruct->isSeekable, parsedTrackStruct->author, parsedTrackStruct->length, parsedTrackStruct->isStream, parsedTrackStruct->position, parsedTrackStruct->title, parsedTrackStruct->uri, parsedTrackStruct->sourceName);

  return COGLINK_SUCCESS;
}

void coglink_decodeTrackCleanup(struct requestInformation *res) {
  free(res->body);
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

  return __coglink_performRequest(lavaInfo, res, &(struct __coglink_requestConfig) {
                                                    .requestType = __COGLINK_POST_REQ,
                                                    .path = "/decodetracks",
                                                    .pathLength = 14,
                                                    .useVPath = true,
                                                    .body = trackArray,
                                                    .bodySize = strlen(trackArray),
                                                    .getResponse = true
                                                  });
}

int coglink_parseDecodeTracks(struct lavaInfo *lavaInfo, struct requestInformation *res, char *songPos, struct parsedTrack *parsedTrackStruct) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTracksParseRequest[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTracksParseRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTracksParseRequest[i](lavaInfo, &res, songPos, &parsedTrackStruct);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onDecodeTracksParseRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onDecodeTracksParseRequest[i](lavaInfoPlugin, &res, songPos, &parsedTrackStruct);
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

  path[2] = "isrc";
  jsmnf_pair *isrc = jsmnf_find_path(pairs, res->body, path, 2);

  path[2] = "artworkUrl";
  jsmnf_pair *artworkUrl = jsmnf_find_path(pairs, res->body, path, 2);

  path[2] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pairs, res->body, path, 3);

  if (!track || !identifier || !isSeekable || !author || !length || !isStream || !position || !title || !uri || !sourceName) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !track ? "track" : !identifier ? "identifier": !isSeekable ? "isSeekable" : !author ? "author" : !length ? "length" : !isStream ? "isStream" : !position ? "position" : !title ? "title" : !uri ? "uri" : !sourceName ? "sourceName" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  snprintf(parsedTrackStruct->track, sizeof(parsedTrackStruct->track), "%.*s", (int)track->v.len, res->body + track->v.pos);
  snprintf(parsedTrackStruct->identifier, sizeof(parsedTrackStruct->identifier), "%.*s", (int)identifier->v.len, res->body + identifier->v.pos);
  snprintf(parsedTrackStruct->isSeekable, sizeof(parsedTrackStruct->isSeekable), "%.*s", (int)isSeekable->v.len, res->body + isSeekable->v.pos);
  snprintf(parsedTrackStruct->author, sizeof(parsedTrackStruct->author), "%.*s", (int)author->v.len, res->body + author->v.pos);
  snprintf(parsedTrackStruct->length, sizeof(parsedTrackStruct->length), "%.*s", (int)length->v.len, res->body + length->v.pos);
  snprintf(parsedTrackStruct->isStream, sizeof(parsedTrackStruct->isStream), "%.*s", (int)isStream->v.len, res->body + isStream->v.pos);
  snprintf(parsedTrackStruct->position, sizeof(parsedTrackStruct->position), "%.*s", (int)position->v.len, res->body + position->v.pos);
  snprintf(parsedTrackStruct->title, sizeof(parsedTrackStruct->title), "%.*s", (int)title->v.len, res->body + title->v.pos);
  snprintf(parsedTrackStruct->uri, sizeof(parsedTrackStruct->uri), "%.*s", (int)uri->v.len, res->body + uri->v.pos);
  if (isrc) snprintf(parsedTrackStruct->isrc, sizeof(parsedTrackStruct->isrc), "%.*s", (int)isrc->v.len, res->body + isrc->v.pos);
  if (artworkUrl) snprintf(parsedTrackStruct->artworkUrl, sizeof(parsedTrackStruct->artworkUrl), "%.*s", (int)artworkUrl->v.len, res->body + artworkUrl->v.pos);
  snprintf(parsedTrackStruct->sourceName, sizeof(parsedTrackStruct->sourceName), "%.*s", (int)sourceName->v.len, res->body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> track: %s\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", parsedTrackStruct->track, parsedTrackStruct->identifier, parsedTrackStruct->isSeekable, parsedTrackStruct->author, parsedTrackStruct->length, parsedTrackStruct->isStream, parsedTrackStruct->position, parsedTrackStruct->title, parsedTrackStruct->uri, parsedTrackStruct->sourceName);

  return COGLINK_SUCCESS;
}

int coglink_searchSong(struct lavaInfo *lavaInfo, char *song, struct requestInformation *res) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onSearchRequest[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onSearchRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onSearchRequest[i](lavaInfo, song, &res);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onSearchRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onSearchRequest[i](lavaInfoPlugin, song, &res);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    }
  }

  curl_global_init(CURL_GLOBAL_ALL);

  CURL *curl = curl_easy_init();
  char *songEncoded = curl_easy_escape(curl, song, strlen(song) + 1);

  char reqPath[2000 + 33];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/loadtracks?identifier=");

  if (0 != strncmp(songEncoded, "https://", 8)) strncat(reqPath, "ytsearch:", sizeof(reqPath) - pathLen - 1);
  strncat(reqPath, songEncoded, sizeof(reqPath) - pathLen - 1);

  curl_free(songEncoded);

  return __coglink_performRequest(lavaInfo, res, &(struct __coglink_requestConfig) {
                                                    .requestType = __COGLINK_GET_REQ,
                                                    .additionalDebuggingSuccess = lavaInfo->debugging->searchSongSuccessDebugging,
                                                    .additionalDebuggingError = lavaInfo->debugging->searchSongErrorsDebugging,
                                                    .path = reqPath,
                                                    .pathLength = strlen(reqPath),
                                                    .useVPath = true,
                                                    .getResponse = true,
                                                    .usedCURL = curl
                                                  });
}

void coglink_searchSongCleanup(struct requestInformation *res) {
  free(res->body);
}

int coglink_parseLoadtype(struct lavaInfo *lavaInfo, struct requestInformation *res, int *loadTypeValue) {
  jsmn_parser parser;
  jsmntok_t tokens[60000];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[60000];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  jsmnf_pair *loadType = jsmnf_find(pairs, res->body, "loadType", sizeof("loadType") - 1);
  if (__coglink_checkParse(lavaInfo, loadType, "loadType") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  char LoadType[16];
  snprintf(LoadType, sizeof(LoadType), "%.*s", (int)loadType->v.len, res->body + loadType->v.pos);

  switch(LoadType[0]) {
    case 'T':
      *loadTypeValue = COGLINK_LOADTYPE_TRACK_LOADED;
      break;
    case 'P':
      *loadTypeValue = COGLINK_LOADTYPE_PLAYLIST_LOADED;
      break;
    case 'S':
      *loadTypeValue = COGLINK_LOADTYPE_SEARCH_RESULT;
      break;
    case 'N':
      *loadTypeValue = COGLINK_LOADTYPE_NO_MATCHES;
      break;
    case 'L':
      *loadTypeValue = COGLINK_LOADTYPE_LOAD_FAILED;
      break;
    default:
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to recognize loadType.");
      *loadTypeValue = COGLINK_JSMNF_ERROR_FIND;
      return COGLINK_JSMNF_ERROR_FIND;
      break;
  }

  return COGLINK_SUCCESS;
}

int coglink_parseTrack(const struct lavaInfo *lavaInfo, struct requestInformation *res, char *songPos, struct parsedTrack *parsedTrackStruct) {
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
  jsmnf_pair pairs[2048];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  char *path[] = { "tracks", songPos, "encoded", NULL };
  jsmnf_pair *track = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "info";
  path[3] = "identifier";
  jsmnf_pair *identifier = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "author";
  jsmnf_pair *author = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "length";
  jsmnf_pair *length = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "position";
  jsmnf_pair *position = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "title";
  jsmnf_pair *title = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "isrc";
  jsmnf_pair *isrc = jsmnf_find_path(pairs, res->body, path, 2);

  path[3] = "artworkUrl";
  jsmnf_pair *artworkUrl = jsmnf_find_path(pairs, res->body, path, 2);

  path[3] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pairs, res->body, path, 4);

  if (!track || !identifier || !isSeekable || !author || !length || !isStream || !position || !title || !uri || !sourceName) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !track ? "track" : !identifier ? "identifier": !isSeekable ? "isSeekable" : !author ? "author" : !length ? "length" : !isStream ? "isStream" : !position ? "position" : !title ? "title" : !uri ? "uri" : "sourceName");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  snprintf(parsedTrackStruct->track, sizeof(parsedTrackStruct->track), "%.*s", (int)track->v.len, res->body + track->v.pos);
  snprintf(parsedTrackStruct->identifier, sizeof(parsedTrackStruct->identifier), "%.*s", (int)identifier->v.len, res->body + identifier->v.pos);
  snprintf(parsedTrackStruct->isSeekable, sizeof(parsedTrackStruct->isSeekable), "%.*s", (int)isSeekable->v.len, res->body + isSeekable->v.pos);
  snprintf(parsedTrackStruct->author, sizeof(parsedTrackStruct->author), "%.*s", (int)author->v.len, res->body + author->v.pos);
  snprintf(parsedTrackStruct->length, sizeof(parsedTrackStruct->length), "%.*s", (int)length->v.len, res->body + length->v.pos);
  snprintf(parsedTrackStruct->isStream, sizeof(parsedTrackStruct->isStream), "%.*s", (int)isStream->v.len, res->body + isStream->v.pos);
  snprintf(parsedTrackStruct->position, sizeof(parsedTrackStruct->position), "%.*s", (int)position->v.len, res->body + position->v.pos);
  snprintf(parsedTrackStruct->title, sizeof(parsedTrackStruct->title), "%.*s", (int)title->v.len, res->body + title->v.pos);
  snprintf(parsedTrackStruct->uri, sizeof(parsedTrackStruct->uri), "%.*s", (int)uri->v.len, res->body + uri->v.pos);
  if (isrc) snprintf(parsedTrackStruct->isrc, sizeof(parsedTrackStruct->isrc), "%.*s", (int)isrc->v.len, res->body + isrc->v.pos);
  if (artworkUrl) snprintf(parsedTrackStruct->artworkUrl, sizeof(parsedTrackStruct->artworkUrl), "%.*s", (int)artworkUrl->v.len, res->body + artworkUrl->v.pos);
  snprintf(parsedTrackStruct->sourceName, sizeof(parsedTrackStruct->sourceName), "%.*s", (int)sourceName->v.len, res->body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> track: %s\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", parsedTrackStruct->track, parsedTrackStruct->identifier, parsedTrackStruct->isSeekable, parsedTrackStruct->author, parsedTrackStruct->length, parsedTrackStruct->isStream, parsedTrackStruct->position, parsedTrackStruct->title, parsedTrackStruct->uri, parsedTrackStruct->sourceName);

  return COGLINK_SUCCESS;
}

int coglink_parsePlaylist(const struct lavaInfo *lavaInfo, struct requestInformation *res, struct parsedPlaylist *parsedPlaylistStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[512];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  char *path[] = { "playlistInfo", "name" };
  jsmnf_pair *name = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "selectedTrack";
  jsmnf_pair *selectedTrack = jsmnf_find_path(pairs, res->body, path, 2);

  if (!name || !selectedTrack) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !name ? "name" : "selectedTrack");
    return COGLINK_JSMNF_ERROR_FIND;
  } 

  snprintf(parsedPlaylistStruct->name, sizeof(parsedPlaylistStruct->name), "%.*s", (int)name->v.len, res->body + name->v.pos);
  snprintf(parsedPlaylistStruct->selectedTrack, sizeof(parsedPlaylistStruct->selectedTrack), "%.*s", (int)selectedTrack->v.len, res->body + selectedTrack->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed playlist search json, results:\n> name: %s\n> selectedTrack: %s", parsedPlaylistStruct->name, parsedPlaylistStruct->selectedTrack);

  return COGLINK_SUCCESS;
}

int coglink_parseError(const struct lavaInfo *lavaInfo, struct requestInformation *res, struct parsedError *parsedErrorStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[512];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  char *path[] = { "exception", "message" };
  jsmnf_pair *message = jsmnf_find_path(pairs, res->body, path, 2);

  path[1] = "severity";
  jsmnf_pair *severity = jsmnf_find_path(pairs, res->body, path, 2);

  if (!message || !severity) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !message ? "message" : "severity");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  snprintf(parsedErrorStruct->message, sizeof(parsedErrorStruct->message), "%.*s", (int)message->v.len, res->body + message->v.pos);
  snprintf(parsedErrorStruct->severity, sizeof(parsedErrorStruct->severity), "%.*s", (int)severity->v.len, res->body + severity->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> message: %s\n> severity: %s", parsedErrorStruct->message, parsedErrorStruct->severity);

  return COGLINK_SUCCESS;
}
