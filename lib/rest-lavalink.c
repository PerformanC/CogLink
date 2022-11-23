#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include <coglink/lavalink.h>
#include <coglink/lavalink-internal.h>
#include <coglink/definitions.h>

int coglink_searchSong(struct lavaInfo *lavaInfo, char *song, struct httpRequest *res) {
  curl_global_init(CURL_GLOBAL_ALL);

  int songLen = strnlen(song, 2000);

  CURL *curl = curl_easy_init();
  char *songEncoded = curl_easy_escape(curl, song, songLen + 1);

  char reqPath[songLen + 32];
  if (lavaInfo->node->ssl) snprintf(reqPath, sizeof(reqPath), "/loadtracks?identifier=");
  else snprintf(reqPath, sizeof(reqPath), "/loadtracks?identifier=");

  if (0 != strncmp(songEncoded, "https://", 8)) strlcat(reqPath, "ytsearch:", sizeof(reqPath));
  strlcat(reqPath, songEncoded, sizeof(reqPath));

  curl_free(songEncoded);

  return __coglink_performRequest(lavaInfo, lavaInfo->debugging->searchSongSuccessDebugging, lavaInfo->debugging->searchSongErrorsDebugging, reqPath, sizeof(reqPath), NULL, 0, res, 1, curl);
}

void coglink_searchCleanup(struct httpRequest req) {
  free(req.body);
}

int coglink_parseLoadtype(struct lavaInfo *lavaInfo, struct httpRequest *req, int *loadTypeValue) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req->body, req->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req->body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
  }

  jsmnf_pair *loadType = jsmnf_find(pairs, req->body, "loadType", 8);
  if (__coglink_checkParse(lavaInfo, loadType, "loadType") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  char LoadType[16];
  snprintf(LoadType, sizeof(LoadType), "%.*s", (int)loadType->v.len, req->body + loadType->v.pos);

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

int coglink_parseTrack(const struct lavaInfo *lavaInfo, struct httpRequest *req, char *songPos, struct lavaParsedTrack **songStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req->body, req->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req->body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
  }

  char *path[] = { "tracks", songPos, "track", NULL };
  jsmnf_pair *track = jsmnf_find_path(pairs, req->body, path, 3);

  path[2] = "info";
  path[3] = "identifier";
  jsmnf_pair *identifier = jsmnf_find_path(pairs, req->body, path, 4);

  path[3] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pairs, req->body, path, 4);

  path[3] = "author";
  jsmnf_pair *author = jsmnf_find_path(pairs, req->body, path, 4);

  path[3] = "length";
  jsmnf_pair *length = jsmnf_find_path(pairs, req->body, path, 4);

  path[3] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pairs, req->body, path, 4);

  path[3] = "position";
  jsmnf_pair *position = jsmnf_find_path(pairs, req->body, path, 4);

  path[3] = "title";
  jsmnf_pair *title = jsmnf_find_path(pairs, req->body, path, 4);

  path[3] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pairs, req->body, path, 4);

  path[3] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pairs, req->body, path, 4);

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

int coglink_parsePlaylist(const struct lavaInfo *lavaInfo, struct httpRequest *req, struct lavaParsedPlaylist **playlistStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req->body, req->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req->body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
  }

  char *path[] = { "playlistInfo", "name" };
  jsmnf_pair *name = jsmnf_find_path(pairs, req->body, path, 2);

  path[1] = "selectedTrack";
  jsmnf_pair *selectedTrack = jsmnf_find_path(pairs, req->body, path, 2);

  if (!name || !selectedTrack) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !name ? "name" : !selectedTrack ? "selectedTrack" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  } 

  char Name[PLAYLIST_NAME_LENGTH], SelectedTrack[8];

  snprintf(Name, sizeof(Name), "%.*s", (int)name->v.len, req->body + name->v.pos);
  snprintf(SelectedTrack, sizeof(SelectedTrack), "%.*s", (int)selectedTrack->v.len, req->body + selectedTrack->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed playlist search json, results:\n> name: %s\n> selectedTrack: %s", Name, SelectedTrack);

  *playlistStruct = malloc(sizeof(struct lavaParsedPlaylist));

  (*playlistStruct)->name = malloc(sizeof(Name));
  (*playlistStruct)->selectedTrack = malloc(sizeof(SelectedTrack));

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for playlist structure.", sizeof(struct lavaParsedPlaylist) + sizeof(Name) + sizeof(SelectedTrack) + 2);

  strlcpy((*playlistStruct)->name, Name, PLAYLIST_NAME_LENGTH);
  strlcpy((*playlistStruct)->selectedTrack, SelectedTrack, 8);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strlcpy] Copied %d bytes to playlist structure.", sizeof(Name) + sizeof(SelectedTrack) + 2);

  return COGLINK_SUCCESS;
}

int coglink_parseError(const struct lavaInfo *lavaInfo, struct httpRequest *req, struct lavaParsedError **errorStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req->body, req->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req->body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
  }

  char *path[] = { "exception", "message" };
  jsmnf_pair *message = jsmnf_find_path(pairs, req->body, path, 2);

  path[1] = "severity";
  jsmnf_pair *severity = jsmnf_find_path(pairs, req->body, path, 2);

  if (!message || !severity) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !message ? "message" : !severity ? "severity" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  char Message[128], Severity[16];

  snprintf(Message, sizeof(Message), "%.*s", (int)message->v.len, req->body + message->v.pos);
  snprintf(Severity, sizeof(Severity), "%.*s", (int)severity->v.len, req->body + severity->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> message: %s\n> severity: %s", Message, Severity);

  *errorStruct = malloc(sizeof(struct lavaParsedError));

  (*errorStruct)->message = malloc(sizeof(Message));
  (*errorStruct)->severity = malloc(sizeof(Severity));

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for error structure.", sizeof(struct lavaParsedError) + sizeof(Message) + sizeof(Severity) + 2);

  strlcpy((*errorStruct)->message, Message, 128);
  strlcpy((*errorStruct)->severity, Severity, 16);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strlcpy] Copied %d bytes to error structure.", sizeof(Message) + sizeof(Severity) + 2);

  return COGLINK_SUCCESS;
}

void coglink_parseTrackCleanup(const struct lavaInfo *lavaInfo, struct lavaParsedTrack *songStruct) {
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

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-free] Freed %d bytes from song structure.", sizeof(struct lavaParsedTrack) + sizeof(songStruct->track) + sizeof(songStruct->identifier) + sizeof(songStruct->isSeekable) + sizeof(songStruct->author) + sizeof(songStruct->length) + sizeof(songStruct->isStream) + sizeof(songStruct->position) + sizeof(songStruct->title) + sizeof(songStruct->uri) + sizeof(songStruct->sourceName) + 10);
}

void coglink_parsePlaylistCleanup(const struct lavaInfo *lavaInfo, struct lavaParsedPlaylist *playlistStruct) {
  if (playlistStruct->name) free(playlistStruct->name);
  if (playlistStruct->selectedTrack) free(playlistStruct->selectedTrack);
  if (playlistStruct) free(playlistStruct);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-free] Freed %d bytes from playlist structure.", sizeof(struct lavaParsedPlaylist) + sizeof(playlistStruct->name) + sizeof(playlistStruct->selectedTrack) + 2);
}

void coglink_parseErrorCleanup(const struct lavaInfo *lavaInfo, struct lavaParsedError *errorStruct) {
  if (errorStruct->message) free(errorStruct->message);
  if (errorStruct->severity) free(errorStruct->severity);
  if (errorStruct) free(errorStruct);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-free] Freed %d bytes from error structure.", sizeof(struct lavaParsedError) + sizeof(errorStruct->message) + sizeof(errorStruct->severity) + 2);
}
