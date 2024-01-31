#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include "lavalink-internal.h"
#include "lavalink.h"
#include "definitions.h"
#include "track.h"

int coglink_searchSong(struct coglink_lavaInfo *lavaInfo, char *song, struct coglink_requestInformation *res) {
  int node = _coglink_selectBestNode(lavaInfo);

  if (lavaInfo->plugins && lavaInfo->plugins->events->onSearchRequest[0]) {
    for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
      if (!lavaInfo->plugins->events->onSearchRequest[i]) break;

      int pluginResultCode = lavaInfo->plugins->events->onSearchRequest[i](lavaInfo, song, res);
      if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
    }
  }

  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] Something went wrong while initializing libcurl (global).");
    return COGLINK_ERROR;
  }

  CURL *curl = curl_easy_init();

  if (!curl) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] Error while initializing libcurl.");
    curl_global_cleanup();
    return COGLINK_ERROR;
  }

  char *songEncoded = curl_easy_escape(curl, song, strlen(song) + 1);

  char reqPath[2000 + 33];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/loadtracks?identifier=");

  if (0 != strncmp(songEncoded, "https://", sizeof("https://"))) strncat(reqPath, "ytsearch:", sizeof(reqPath) - pathLen - 1);
  strncat(reqPath, songEncoded, sizeof(reqPath) - pathLen - 1);

  curl_free(songEncoded);

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], res, 
                                 &(struct __coglink_requestConfig) {
                                   .requestType = __COGLINK_GET_REQ,
                                   .additionalDebuggingSuccess = lavaInfo->debugging->searchSongSuccessDebugging,
                                   .additionalDebuggingError = lavaInfo->debugging->searchSongErrorsDebugging,
                                   .path = reqPath,
                                   .pathLength = strlen(reqPath),
                                   .getResponse = 1,
                                   .usedCURL = curl
                                 });
}

void coglink_searchSongCleanup(struct coglink_requestInformation *res) {
  free(res->body);
}

int coglink_decodeTrack(struct coglink_lavaInfo *lavaInfo, char *encodedTrack, struct coglink_requestInformation *res) {
  int node = _coglink_selectBestNode(lavaInfo);

  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTrackRequest[0]) {
    for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
      if (!lavaInfo->plugins->events->onDecodeTrackRequest[i]) break;

      int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackRequest[i](lavaInfo, encodedTrack, res);
      if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
    }
  }

  char reqPath[COGLINK_TRACK_LENGTH + 26];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/decodetrack?encodedTrack=%s", encodedTrack);

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], res,
                                 &(struct __coglink_requestConfig) {
                                   .requestType = __COGLINK_GET_REQ,
                                   .path = reqPath,
                                   .pathLength = pathLen,
                                   .getResponse = 1
                                 });
}

int coglink_initParseTrack(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *cStruct, struct coglink_requestInformation *res) {
  jsmn_parser parser;
  jsmntok_t tokens[1024 * 4];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, cStruct->pairs, sizeof(cStruct->pairs) / sizeof(*cStruct->pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  return COGLINK_SUCCESS;
}

int coglink_parseDecodeTrack(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, struct coglink_parsedTrack *parsedTrackStruct) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTrackParseRequest[0]) {
    for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
      if (!lavaInfo->plugins->events->onDecodeTrackParseRequest[i]) break;

      int pluginResultCode = lavaInfo->plugins->events->onDecodeTrackParseRequest[i](lavaInfo, res, parsedTrackStruct);
      if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
    }
  }

  char *path[] = { "info", "identifier" };
  jsmnf_pair *identifier = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, identifier, "identifier") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[1] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, isSeekable, "isSeekable") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[1] = "author";
  jsmnf_pair *author = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, author, "author") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[1] = "length";
  jsmnf_pair *length = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, length, "length") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[1] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, isStream, "isStream") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[1] = "position";
  jsmnf_pair *position = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, position, "position") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[1] = "title";
  jsmnf_pair *title = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, title, "title") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[1] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pStruct->pairs, res->body, path, 2);

  path[1] = "artworkUrl";
  jsmnf_pair *artworkUrl = jsmnf_find_path(pStruct->pairs, res->body, path, 2);

  path[1] = "isrc";
  jsmnf_pair *isrc = jsmnf_find_path(pStruct->pairs, res->body, path, 2);

  path[1] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, sourceName, "sourceName") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

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
  if (_coglink_checkParse(lavaInfo, uri, "uri") == COGLINK_SUCCESS) snprintf(parsedTrackStruct->uri, sizeof(parsedTrackStruct->uri), "%.*s", (int)uri->v.len, res->body + uri->v.pos);
  if (_coglink_checkParse(lavaInfo, artworkUrl, "artworkUrl") == COGLINK_SUCCESS) snprintf(parsedTrackStruct->artworkUrl, sizeof(parsedTrackStruct->artworkUrl), "%.*s", (int)artworkUrl->v.len, res->body + artworkUrl->v.pos);
  if (_coglink_checkParse(lavaInfo, isrc, "isrc") == COGLINK_SUCCESS) snprintf(parsedTrackStruct->isrc, sizeof(parsedTrackStruct->isrc), "%.*s", (int)isrc->v.len, res->body + isrc->v.pos);
  snprintf(parsedTrackStruct->sourceName, sizeof(parsedTrackStruct->sourceName), "%.*s", (int)sourceName->v.len, res->body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", parsedTrackStruct->identifier, parsedTrackStruct->isSeekable, parsedTrackStruct->author, parsedTrackStruct->length, parsedTrackStruct->isStream, parsedTrackStruct->position, parsedTrackStruct->title, parsedTrackStruct->uri, parsedTrackStruct->sourceName);

  return COGLINK_SUCCESS;
}

void coglink_decodeTrackCleanup(struct coglink_requestInformation *res) {
  free(res->body);
}

int coglink_decodeTracks(struct coglink_lavaInfo *lavaInfo, char *trackArray, long trackArrayLength, struct coglink_requestInformation *res) {
  int node = _coglink_selectBestNode(lavaInfo);

  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTracksRequest[0]) {
    for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
      if (!lavaInfo->plugins->events->onDecodeTracksRequest[i]) break;

      int pluginResultCode = lavaInfo->plugins->events->onDecodeTracksRequest[i](lavaInfo, trackArray, trackArrayLength, res);
      if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
    }
  }

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], res,
                                 &(struct __coglink_requestConfig) {
                                   .requestType = __COGLINK_POST_REQ,
                                   .path = "/decodetracks",
                                   .pathLength = 14,
                                   .useVPath = 1,
                                   .body = trackArray,
                                   .bodySize = strlen(trackArray),
                                   .getResponse = 1
                                 });
}

int coglink_parseDecodeTracks(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, char *songPos, struct coglink_parsedTrack *parsedTrackStruct) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onDecodeTracksParseRequest[0]) {
    for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
      if (!lavaInfo->plugins->events->onDecodeTracksParseRequest[i]) break;

      int pluginResultCode = lavaInfo->plugins->events->onDecodeTracksParseRequest[i](lavaInfo, res, songPos, parsedTrackStruct);
      if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
    }
  }

  char *path[] = { songPos, "encoded", NULL };
  jsmnf_pair *encoded = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, encoded, "encoded") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[1] = "info";
  path[2] = "identifier";
  jsmnf_pair *identifier = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, identifier, "identifier") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[2] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, isSeekable, "isSeekable") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[2] = "author";
  jsmnf_pair *author = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, author, "author") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[2] = "length";
  jsmnf_pair *length = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, length, "length") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[2] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, isStream, "isStream") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[2] = "position";
  jsmnf_pair *position = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, position, "position") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[2] = "title";
  jsmnf_pair *title = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, title, "title") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  path[2] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pStruct->pairs, res->body, path, 3);

  path[2] = "artworkUrl";
  jsmnf_pair *artworkUrl = jsmnf_find_path(pStruct->pairs, res->body, path, 2);

  path[2] = "isrc";
  jsmnf_pair *isrc = jsmnf_find_path(pStruct->pairs, res->body, path, 2);

  path[2] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, sourceName, "sourceName") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

  snprintf(parsedTrackStruct->encoded, sizeof(parsedTrackStruct->encoded), "%.*s", (int)encoded->v.len, res->body + encoded->v.pos);
  snprintf(parsedTrackStruct->identifier, sizeof(parsedTrackStruct->identifier), "%.*s", (int)identifier->v.len, res->body + identifier->v.pos);
  snprintf(parsedTrackStruct->isSeekable, sizeof(parsedTrackStruct->isSeekable), "%.*s", (int)isSeekable->v.len, res->body + isSeekable->v.pos);
  snprintf(parsedTrackStruct->author, sizeof(parsedTrackStruct->author), "%.*s", (int)author->v.len, res->body + author->v.pos);
  snprintf(parsedTrackStruct->length, sizeof(parsedTrackStruct->length), "%.*s", (int)length->v.len, res->body + length->v.pos);
  snprintf(parsedTrackStruct->isStream, sizeof(parsedTrackStruct->isStream), "%.*s", (int)isStream->v.len, res->body + isStream->v.pos);
  snprintf(parsedTrackStruct->position, sizeof(parsedTrackStruct->position), "%.*s", (int)position->v.len, res->body + position->v.pos);
  snprintf(parsedTrackStruct->title, sizeof(parsedTrackStruct->title), "%.*s", (int)title->v.len, res->body + title->v.pos);
  if (_coglink_checkParse(lavaInfo, uri, "uri") == COGLINK_SUCCESS) snprintf(parsedTrackStruct->uri, sizeof(parsedTrackStruct->uri), "%.*s", (int)uri->v.len, res->body + uri->v.pos);
  if (_coglink_checkParse(lavaInfo, artworkUrl, "artworkUrl") == COGLINK_SUCCESS) snprintf(parsedTrackStruct->artworkUrl, sizeof(parsedTrackStruct->artworkUrl), "%.*s", (int)artworkUrl->v.len, res->body + artworkUrl->v.pos);
  if (_coglink_checkParse(lavaInfo, isrc, "isrc") == COGLINK_SUCCESS) snprintf(parsedTrackStruct->isrc, sizeof(parsedTrackStruct->isrc), "%.*s", (int)isrc->v.len, res->body + isrc->v.pos);
  snprintf(parsedTrackStruct->sourceName, sizeof(parsedTrackStruct->sourceName), "%.*s", (int)sourceName->v.len, res->body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> encoded: %s\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", parsedTrackStruct->encoded, parsedTrackStruct->identifier, parsedTrackStruct->isSeekable, parsedTrackStruct->author, parsedTrackStruct->length, parsedTrackStruct->isStream, parsedTrackStruct->position, parsedTrackStruct->title, parsedTrackStruct->uri, parsedTrackStruct->sourceName);

  return COGLINK_SUCCESS;
}

int coglink_parseLoadtype(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, int *loadTypeValue) {
  jsmnf_pair *loadType = jsmnf_find(pStruct->pairs, res->body, "loadType", sizeof("loadType") - 1);
  if (_coglink_checkParse(lavaInfo, loadType, "loadType") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  char LoadType[16];
  snprintf(LoadType, sizeof(LoadType), "%.*s", (int)loadType->v.len, res->body + loadType->v.pos);

  switch(LoadType[0]) {
    case 't':
      *loadTypeValue = COGLINK_LOADTYPE_TRACK_LOADED;
      break;
    case 'p':
      *loadTypeValue = COGLINK_LOADTYPE_PLAYLIST_LOADED;
      break;
    case 's':
      *loadTypeValue = COGLINK_LOADTYPE_SEARCH_RESULT;
      break;
    case 'e':
      if (LoadType[1] == 'm') *loadTypeValue = COGLINK_LOADTYPE_EMPTY;
      else *loadTypeValue = COGLINK_LOADTYPE_LOAD_FAILED;
      break;
    default:
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to recognize loadType.");
      *loadTypeValue = COGLINK_JSMNF_ERROR_FIND;
      return COGLINK_JSMNF_ERROR_FIND;
      break;
  }

  return COGLINK_SUCCESS;
}

int coglink_parseTrack(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, char *songPos, struct coglink_parsedTrack *parsedTrackStruct) {
  char *path[] = { "data", songPos, "encoded", NULL };
  jsmnf_pair *encoded = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, encoded, "encoded") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[2] = "info";
  path[3] = "identifier";
  jsmnf_pair *identifier = jsmnf_find_path(pStruct->pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, identifier, "identifier") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[3] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pStruct->pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, isSeekable, "isSeekable") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[3] = "author";
  jsmnf_pair *author = jsmnf_find_path(pStruct->pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, author, "author") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[3] = "length";
  jsmnf_pair *length = jsmnf_find_path(pStruct->pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, length, "length") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[3] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pStruct->pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, isStream, "isStream") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[3] = "position";
  jsmnf_pair *position = jsmnf_find_path(pStruct->pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, position, "position") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[3] = "title";
  jsmnf_pair *title = jsmnf_find_path(pStruct->pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, title, "title") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[3] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pStruct->pairs, res->body, path, 4);

  path[3] = "artworkUrl";
  jsmnf_pair *artworkUrl = jsmnf_find_path(pStruct->pairs, res->body, path, 4);

  path[3] = "isrc";
  jsmnf_pair *isrc = jsmnf_find_path(pStruct->pairs, res->body, path, 4);

  path[3] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pStruct->pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, sourceName, "sourceName") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  snprintf(parsedTrackStruct->encoded, sizeof(parsedTrackStruct->encoded), "%.*s", (int)encoded->v.len, res->body + encoded->v.pos);
  snprintf(parsedTrackStruct->identifier, sizeof(parsedTrackStruct->identifier), "%.*s", (int)identifier->v.len, res->body + identifier->v.pos);
  snprintf(parsedTrackStruct->isSeekable, sizeof(parsedTrackStruct->isSeekable), "%.*s", (int)isSeekable->v.len, res->body + isSeekable->v.pos);
  snprintf(parsedTrackStruct->author, sizeof(parsedTrackStruct->author), "%.*s", (int)author->v.len, res->body + author->v.pos);
  snprintf(parsedTrackStruct->length, sizeof(parsedTrackStruct->length), "%.*s", (int)length->v.len, res->body + length->v.pos);
  snprintf(parsedTrackStruct->isStream, sizeof(parsedTrackStruct->isStream), "%.*s", (int)isStream->v.len, res->body + isStream->v.pos);
  snprintf(parsedTrackStruct->position, sizeof(parsedTrackStruct->position), "%.*s", (int)position->v.len, res->body + position->v.pos);
  snprintf(parsedTrackStruct->title, sizeof(parsedTrackStruct->title), "%.*s", (int)title->v.len, res->body + title->v.pos);
  if (_coglink_checkParse(lavaInfo, uri, "uri") == COGLINK_PROCEED) snprintf(parsedTrackStruct->uri, sizeof(parsedTrackStruct->uri), "%.*s", (int)uri->v.len, res->body + uri->v.pos);
  if (_coglink_checkParse(lavaInfo, artworkUrl, "artworkUrl") == COGLINK_PROCEED) snprintf(parsedTrackStruct->artworkUrl, sizeof(parsedTrackStruct->artworkUrl), "%.*s", (int)artworkUrl->v.len, res->body + artworkUrl->v.pos);
  if (_coglink_checkParse(lavaInfo, isrc, "isrc") == COGLINK_PROCEED) snprintf(parsedTrackStruct->isrc, sizeof(parsedTrackStruct->isrc), "%.*s", (int)isrc->v.len, res->body + isrc->v.pos);
  snprintf(parsedTrackStruct->sourceName, sizeof(parsedTrackStruct->sourceName), "%.*s", (int)sourceName->v.len, res->body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> encoded: %s\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", parsedTrackStruct->encoded, parsedTrackStruct->identifier, parsedTrackStruct->isSeekable, parsedTrackStruct->author, parsedTrackStruct->length, parsedTrackStruct->isStream, parsedTrackStruct->position, parsedTrackStruct->title, parsedTrackStruct->uri, parsedTrackStruct->sourceName);

  return COGLINK_SUCCESS;
}

int coglink_parsePlaylist(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, struct coglink_parsedPlaylist *parsedPlaylistStruct) {
  char *path[] = { "data", "info", "name" };
  jsmnf_pair *name = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, name, "name") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[2] = "selectedTrack";
  jsmnf_pair *selectedTrack = jsmnf_find_path(pStruct->pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, selectedTrack, "selectedTrack") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  snprintf(parsedPlaylistStruct->name, sizeof(parsedPlaylistStruct->name), "%.*s", (int)name->v.len, res->body + name->v.pos);
  snprintf(parsedPlaylistStruct->selectedTrack, sizeof(parsedPlaylistStruct->selectedTrack), "%.*s", (int)selectedTrack->v.len, res->body + selectedTrack->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed playlist search json, results:\n> name: %s\n> selectedTrack: %s", parsedPlaylistStruct->name, parsedPlaylistStruct->selectedTrack);

  return COGLINK_SUCCESS;
}

int coglink_parseError(struct coglink_lavaInfo *lavaInfo, struct coglink_parsedTrackStruct *pStruct, struct coglink_requestInformation *res, struct coglink_parsedError *parsedErrorStruct) {
  char *path[] = { "data", "message" };
  jsmnf_pair *message = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, message, "message") == COGLINK_ERROR) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "severity";
  jsmnf_pair *severity = jsmnf_find_path(pStruct->pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, severity, "severity") == COGLINK_ERROR) return COGLINK_JSMNF_ERROR_FIND;

  snprintf(parsedErrorStruct->message, sizeof(parsedErrorStruct->message), "%.*s", (int)message->v.len, res->body + message->v.pos);
  snprintf(parsedErrorStruct->severity, sizeof(parsedErrorStruct->severity), "%.*s", (int)severity->v.len, res->body + severity->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> message: %s\n> severity: %s", parsedErrorStruct->message, parsedErrorStruct->severity);

  return COGLINK_SUCCESS;
}
