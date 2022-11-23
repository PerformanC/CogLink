#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>
#include <coglink/miscellaneous.h>

int coglink_decodeTrack(struct lavaInfo *lavaInfo, char *track, struct httpRequest *res) {
  char reqPath[strnlen(track, 512) + 19];
  snprintf(reqPath, sizeof(reqPath), "/decodetrack?track=%s", track);

  return __coglink_performRequest(lavaInfo, lavaInfo->debugging->searchSongSuccessDebugging, lavaInfo->debugging->searchSongErrorsDebugging, reqPath, sizeof(reqPath), NULL, 0, res, 1, NULL);
}

int coglink_parseDecodeTrack(struct lavaInfo *lavaInfo, struct httpRequest *res, struct lavaParsedTrack **songStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[512];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[512];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, 512);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  jsmnf_pair *identifier = jsmnf_find(pairs, res->body, "identifier", 10);
  if (__coglink_checkParse(lavaInfo, identifier, "identifier") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *isSeekable = jsmnf_find(pairs, res->body, "isSeekable", 10);
  if (__coglink_checkParse(lavaInfo, isSeekable, "isSeekable") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *author = jsmnf_find(pairs, res->body, "author", 6);
  if (__coglink_checkParse(lavaInfo, author, "author") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *length = jsmnf_find(pairs, res->body, "length", 6);
  if (__coglink_checkParse(lavaInfo, length, "length") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *isStream = jsmnf_find(pairs, res->body, "isStream", 8);
  if (__coglink_checkParse(lavaInfo, isStream, "isStream") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *position = jsmnf_find(pairs, res->body, "position", 8);
  if (__coglink_checkParse(lavaInfo, position, "position") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *title = jsmnf_find(pairs, res->body, "title", 5);
  if (__coglink_checkParse(lavaInfo, title, "title") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *uri = jsmnf_find(pairs, res->body, "uri", 3);
  if (__coglink_checkParse(lavaInfo, uri, "uri") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  jsmnf_pair *sourceName = jsmnf_find(pairs, res->body, "sourceName", 10);
  if (__coglink_checkParse(lavaInfo, sourceName, "sourceName") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

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

  *songStruct = malloc(sizeof(struct lavaParsedTrack));

  (*songStruct)->identifier = malloc(sizeof(Identifier));
  (*songStruct)->isSeekable = malloc(sizeof(IsSeekable));
  (*songStruct)->author = malloc(sizeof(Author));
  (*songStruct)->length = malloc(sizeof(Length));
  (*songStruct)->isStream = malloc(sizeof(IsStream));
  (*songStruct)->position = malloc(sizeof(Position));
  (*songStruct)->title = malloc(sizeof(Title));
  (*songStruct)->uri = malloc(sizeof(Uri));
  (*songStruct)->sourceName = malloc(sizeof(SourceName));

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for song structure.", sizeof(struct lavaParsedTrack) + sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName) + 9);

  strlcpy((*songStruct)->identifier, Identifier, IDENTIFIER_LENGTH);
  strlcpy((*songStruct)->isSeekable, IsSeekable, TRUE_FALSE_LENGTH);
  strlcpy((*songStruct)->author, Author, AUTHOR_NAME_LENGTH);
  strlcpy((*songStruct)->length, Length, VIDEO_LENGTH);
  strlcpy((*songStruct)->isStream, IsStream,TRUE_FALSE_LENGTH);
  strlcpy((*songStruct)->position, Position, VIDEO_LENGTH);
  strlcpy((*songStruct)->title, Title, TRACK_TITLE_LENGTH);
  strlcpy((*songStruct)->uri, Uri, URL_LENGTH);
  strlcpy((*songStruct)->sourceName, SourceName, SOURCENAME_LENGTH);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strlcpy] Copied %d bytes to song structure.", sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName) + 10);

  return COGLINK_SUCCESS;
}

int coglink_decodeTracks(struct lavaInfo *lavaInfo, char *trackArray, struct httpRequest *res) {
  return __coglink_performRequest(lavaInfo, lavaInfo->debugging->searchSongSuccessDebugging, lavaInfo->debugging->searchSongErrorsDebugging, "/decodetracks", 14, trackArray, strlen(trackArray), res, 1, NULL);
}

int coglink_parseDecodeTracks(const struct lavaInfo *lavaInfo, struct httpRequest *req, char *songPos, struct lavaParsedTrack **songStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req->body, req->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req->body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  char *path[] = { songPos, "track", NULL };
  jsmnf_pair *track = jsmnf_find_path(pairs, req->body, path, 2);

  path[1] = "info";
  path[2] = "identifier";
  jsmnf_pair *identifier = jsmnf_find_path(pairs, req->body, path, 3);

  path[2] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pairs, req->body, path, 3);

  path[2] = "author";
  jsmnf_pair *author = jsmnf_find_path(pairs, req->body, path, 3);

  path[2] = "length";
  jsmnf_pair *length = jsmnf_find_path(pairs, req->body, path, 3);

  path[2] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pairs, req->body, path, 3);

  path[2] = "position";
  jsmnf_pair *position = jsmnf_find_path(pairs, req->body, path, 3);

  path[2] = "title";
  jsmnf_pair *title = jsmnf_find_path(pairs, req->body, path, 3);

  path[2] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pairs, req->body, path, 3);

  path[2] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pairs, req->body, path, 3);

  if (!track || !identifier || !isSeekable || !author || !length || !isStream || !position || !title || !uri || !sourceName) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !track ? "track" : !identifier ? "identifier": !isSeekable ? "isSeekable" : !author ? "author" : !length ? "length" : !isStream ? "isStream" : !position ? "position" : !title ? "title" : !uri ? "uri" : !sourceName ? "sourceName" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  char Track[TRACK_LENGTH], Identifier[IDENTIFIER_LENGTH], IsSeekable[TRUE_FALSE_LENGTH], Author[AUTHOR_NAME_LENGTH], Length[VIDEO_LENGTH], IsStream[TRUE_FALSE_LENGTH], Position[VIDEO_LENGTH], Title[TRACK_TITLE_LENGTH], Uri[URL_LENGTH], SourceName[SOURCENAME_LENGTH];

  snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, req->body + track->v.pos);
  snprintf(Identifier, sizeof(Identifier), "%.*s", (int)identifier->v.len, req->body + identifier->v.pos);
  snprintf(IsSeekable, sizeof(IsSeekable), "%.*s", (int)isSeekable->v.len, req->body + isSeekable->v.pos);
  snprintf(Author, sizeof(Author), "%.*s", (int)author->v.len, req->body + author->v.pos);
  snprintf(Length, sizeof(Length), "%.*s", (int)length->v.len, req->body + length->v.pos);
  snprintf(IsStream, sizeof(IsStream), "%.*s", (int)isStream->v.len, req->body + isStream->v.pos);
  snprintf(Position, sizeof(Position), "%.*s", (int)position->v.len, req->body + position->v.pos);
  snprintf(Title, sizeof(Title), "%.*s", (int)title->v.len, req->body + title->v.pos);
  snprintf(Uri, sizeof(Uri), "%.*s", (int)uri->v.len, req->body + uri->v.pos);
  snprintf(SourceName, sizeof(SourceName), "%.*s", (int)sourceName->v.len, req->body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> track: %s\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", Track, Identifier, IsSeekable, Author, Length, IsStream, Position, Title, Uri, SourceName);

  *songStruct = malloc(sizeof(struct lavaParsedTrack));

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

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for song structure.", sizeof(struct lavaParsedTrack) + sizeof(Track) + sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName) + 10);

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

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strlcpy] Copied %d bytes to song structure.", sizeof(Track) + sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName) + 10);

  return COGLINK_SUCCESS;
}

int coglink_getPlugins(struct lavaInfo *lavaInfo, struct httpRequest *res) {
  return __coglink_performRequest(lavaInfo, lavaInfo->debugging->searchSongSuccessDebugging, lavaInfo->debugging->searchSongErrorsDebugging, "/plugins", 9, NULL, 0, res, 1, NULL);
}

int coglink_getRouterPlanner(struct lavaInfo *lavaInfo, struct httpRequest *res) {
  return __coglink_performRequest(lavaInfo, lavaInfo->debugging->curlSuccessDebugging, lavaInfo->debugging->curlErrorsDebugging, "/routeplanner/status", 21, NULL, 0, res, 1, NULL);
}

int coglink_parseRouterPlanner(struct lavaInfo *lavaInfo, struct httpRequest *res, char *ipPosition, struct lavaRouter **lavaRouterStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

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
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, 1024);

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

  if (0 == strcmp("RotatingIpRoutePlanner", Class)) {
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

    *lavaRouterStruct = malloc(sizeof(struct lavaRouter));

    (*lavaRouterStruct)->class = malloc(sizeof(Class));
    (*lavaRouterStruct)->details = malloc(sizeof(struct lavaRouterDetails));
    (*lavaRouterStruct)->details->ipBlock = malloc(sizeof(struct lavaDetailsIpBlock));
    (*lavaRouterStruct)->details->failingAddress = malloc(sizeof(struct lavaDetailsFailingAddress));

    (*lavaRouterStruct)->details->ipBlock->type = malloc(sizeof(Type));
    (*lavaRouterStruct)->details->ipBlock->size = malloc(sizeof(Size));

    (*lavaRouterStruct)->details->rotateIndex = malloc(sizeof(RotateIndex));
    (*lavaRouterStruct)->details->ipIndex = malloc(sizeof(IpIndex));
    (*lavaRouterStruct)->details->currentAddress = malloc(sizeof(CurrentAddress));

    (*lavaRouterStruct)->details->failingAddress->address = malloc(sizeof(Address));
    (*lavaRouterStruct)->details->failingAddress->failingTimestamp = malloc(sizeof(FailingTimestamp));
    (*lavaRouterStruct)->details->failingAddress->failingTime = malloc(sizeof(FailingTime));

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Allocated %d bytes for router structure.", sizeof(struct lavaRouter) + sizeof(Class) + sizeof(struct lavaRouterDetails) + sizeof(struct lavaDetailsIpBlock) + sizeof(struct lavaDetailsFailingAddress) + sizeof(Type) + sizeof(Size) + sizeof(RotateIndex) + sizeof(IpIndex) + sizeof(CurrentAddress) + sizeof(Address) + sizeof(FailingTimestamp) + sizeof(FailingTime));

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
  } else if (0 == strcmp("NanoIpRoutePlanner", Class)) {
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

    *lavaRouterStruct = malloc(sizeof(struct lavaRouter));

    (*lavaRouterStruct)->class = malloc(sizeof(Class));
    (*lavaRouterStruct)->details = malloc(sizeof(struct lavaRouterDetails));
    (*lavaRouterStruct)->details->ipBlock = malloc(sizeof(struct lavaDetailsIpBlock));
    (*lavaRouterStruct)->details->failingAddress = malloc(sizeof(struct lavaDetailsFailingAddress));

    (*lavaRouterStruct)->details->ipBlock->type = malloc(sizeof(Type));
    (*lavaRouterStruct)->details->ipBlock->size = malloc(sizeof(Size));

    (*lavaRouterStruct)->details->currentAddressIndex = malloc(sizeof(CurrentAddressIndex));

    (*lavaRouterStruct)->details->failingAddress->address = malloc(sizeof(Address));
    (*lavaRouterStruct)->details->failingAddress->failingTimestamp = malloc(sizeof(FailingTimestamp));
    (*lavaRouterStruct)->details->failingAddress->failingTime = malloc(sizeof(FailingTime));
      
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Allocated %d bytes for router structure.", sizeof(struct lavaRouter) + sizeof(Class) + sizeof(struct lavaRouterDetails) + sizeof(struct lavaDetailsIpBlock) + sizeof(struct lavaDetailsFailingAddress) + sizeof(Type) + sizeof(Size) + sizeof(CurrentAddressIndex) + sizeof(Address) + sizeof(FailingTimestamp) + sizeof(FailingTime));

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

    *lavaRouterStruct = malloc(sizeof(struct lavaRouter));

    (*lavaRouterStruct)->class = malloc(sizeof(Class));
    (*lavaRouterStruct)->details = malloc(sizeof(struct lavaRouterDetails));
    (*lavaRouterStruct)->details->ipBlock = malloc(sizeof(struct lavaDetailsIpBlock));
    (*lavaRouterStruct)->details->failingAddress = malloc(sizeof(struct lavaDetailsFailingAddress));

    (*lavaRouterStruct)->details->ipBlock->type = malloc(sizeof(Type));
    (*lavaRouterStruct)->details->ipBlock->size = malloc(sizeof(Size));

    (*lavaRouterStruct)->details->blockIndex = malloc(sizeof(BlockIndex));
    (*lavaRouterStruct)->details->currentAddressIndex = malloc(sizeof(CurrentAddressIndex));

    (*lavaRouterStruct)->details->failingAddress->address = malloc(sizeof(Address));
    (*lavaRouterStruct)->details->failingAddress->failingTimestamp = malloc(sizeof(FailingTimestamp));
    (*lavaRouterStruct)->details->failingAddress->failingTime = malloc(sizeof(FailingTime));
      
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Allocated %d bytes for router structure.", sizeof(struct lavaRouter) + sizeof(Class) + sizeof(struct lavaRouterDetails) + sizeof(struct lavaDetailsIpBlock) + sizeof(struct lavaDetailsFailingAddress) + sizeof(Type) + sizeof(Size) + sizeof(BlockIndex) + sizeof(CurrentAddressIndex) + sizeof(Address) + sizeof(FailingTimestamp) + sizeof(FailingTime));

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

  return __coglink_performRequest(lavaInfo, lavaInfo->debugging->curlSuccessDebugging, lavaInfo->debugging->curlErrorsDebugging, "/routeplanner/free/address", 27, payload, (long)strlen(payload), NULL, 0, NULL);
}

int coglink_freeFailingAllAddresses(struct lavaInfo *lavaInfo) {
  return __coglink_performRequest(lavaInfo, lavaInfo->debugging->curlSuccessDebugging, lavaInfo->debugging->curlErrorsDebugging, "/routeplanner/free/address", 27, NULL, 0, NULL, 0, NULL);
}
