#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include <coglink/lavalink.h>
#include <coglink/lavalink-internal.h>
#include <coglink/definations.h>

size_t __coglink_WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t writeSize = size * nmemb;
  struct httpRequest *mem = (struct httpRequest *)userp;

  char *ptr = realloc(mem->body, mem->size + writeSize + 1);
  if (!ptr) {
    perror("[SYSTEM] Not enough memory to realloc.\n");
    return 1;
  }

  mem->body = ptr;
  memcpy(&(mem->body[mem->size]), contents, writeSize);
  mem->size += writeSize;
  mem->body[mem->size] = 0;

  return writeSize;
}

int coglink_searchSong(struct lavaInfo *lavaInfo, char *song, struct httpRequest *res) {
  curl_global_init(CURL_GLOBAL_ALL);

  CURL *curl = curl_easy_init();
  char *songEncoded = curl_easy_escape(curl, song, strlen(song));

  char lavaURL[strlen(lavaInfo->node.hostname) + strlen(song) + 40];
  if (lavaInfo->node.ssl) snprintf(lavaURL, sizeof(lavaURL), "https://%s/loadtracks?identifier=", lavaInfo->node.hostname);
  else snprintf(lavaURL, sizeof(lavaURL), "http://%s/loadtracks?identifier=", lavaInfo->node.hostname);

  if (0 != strncmp(songEncoded, "https://", 8)) strncat(lavaURL, "ytsearch:", sizeof(lavaURL) - 1);
  strncat(lavaURL, songEncoded, sizeof(lavaURL) - 1);

  curl_free(songEncoded);

  if (!curl) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] Error while initializing libcurl.");
    return COGLINK_LIBCURL_FAILED_INITIALIZE;
  }

  struct httpRequest req;

  req.body = malloc(1);
  req.size = 0;

  CURLcode cRes = curl_easy_setopt(curl, CURLOPT_URL, lavaURL);

  if (cRes != CURLE_OK) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] curl_easy_setopt [1] failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAILED_SETOPT;
  }

  struct curl_slist *chunk = NULL;
    
  if (lavaInfo->node.password) {
    char AuthorizationH[256];
    snprintf(AuthorizationH, sizeof(AuthorizationH), "Authorization: %s", lavaInfo->node.password);
    chunk = curl_slist_append(chunk, AuthorizationH);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->searchSongSuccessDebugging || lavaInfo->debugging->curlSuccessDebugging) log_debug("[coglink:libcurl] Authorization header set.");
  }
  chunk = curl_slist_append(chunk, "Client-Name: Coglink");
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->searchSongSuccessDebugging || lavaInfo->debugging->curlSuccessDebugging) log_debug("[coglink:libcurl] Client-Name header set.");

  chunk = curl_slist_append(chunk, "User-Agent: libcurl");
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->searchSongSuccessDebugging || lavaInfo->debugging->curlSuccessDebugging) log_debug("[coglink:libcurl] User-Agent header set.");

  cRes = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
  if (cRes != CURLE_OK) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->searchSongErrorsDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] curl_easy_setopt [2] failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAILED_SETOPT;
  } 

  cRes = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __coglink_WriteMemoryCallback);
  if (cRes != CURLE_OK) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->searchSongErrorsDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] curl_easy_setopt [3] failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAILED_SETOPT;
  }

  cRes = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&req);
  if (cRes != CURLE_OK) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->searchSongErrorsDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] curl_easy_setopt [4] failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAILED_SETOPT;
  }

  cRes = curl_easy_perform(curl);
  if (cRes != CURLE_OK) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->searchSongErrorsDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] curl_easy_perform failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAILED_PERFORM;
  }

  curl_easy_cleanup(curl);
  curl_slist_free_all(chunk);
  curl_global_cleanup(); 

  *res = req;
  
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->searchSongSuccessDebugging || lavaInfo->debugging->curlSuccessDebugging) log_debug("[coglink:libcurl] Search song done, response: %s", res->body);

  return COGLINK_SUCCESS;
}

void coglink_searchCleanup(struct httpRequest req) {
  free(req.body);
}

int coglink_parseLoadtype(struct lavaInfo *lavaInfo, struct httpRequest req, int *loadTypeValue) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req.body, req.size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req.body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseLoadtypeSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
  }

  jsmnf_pair *loadType = jsmnf_find(pairs, req.body, "loadType", 8);
  if (__coglink_checkParse(lavaInfo, loadType, "loadType") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  char LoadType[16];
  snprintf(LoadType, sizeof(LoadType), "%.*s", (int)loadType->v.len, req.body + loadType->v.pos);

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

int coglink_parseTrack(const struct lavaInfo *lavaInfo, struct httpRequest req, char *songPos, struct lavaParsedTrack **songStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req.body, req.size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req.body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
  }

  char *path[] = { "tracks", songPos, "track", NULL };
  jsmnf_pair *track = jsmnf_find_path(pairs, req.body, path, 3);

  path[2] = "info";
  path[3] = "identifier";
  jsmnf_pair *identifier = jsmnf_find_path(pairs, req.body, path, 4);

  path[3] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pairs, req.body, path, 4);

  path[3] = "author";
  jsmnf_pair *author = jsmnf_find_path(pairs, req.body, path, 4);

  path[3] = "length";
  jsmnf_pair *length = jsmnf_find_path(pairs, req.body, path, 4);

  path[3] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pairs, req.body, path, 4);

  path[3] = "position";
  jsmnf_pair *position = jsmnf_find_path(pairs, req.body, path, 4);

  path[3] = "title";
  jsmnf_pair *title = jsmnf_find_path(pairs, req.body, path, 4);

  path[3] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pairs, req.body, path, 4);

  path[3] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pairs, req.body, path, 4);

  if (!track || !identifier || !isSeekable || !author || !length || !isStream || !position || !title || !uri || !sourceName) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !track ? "track" : !identifier ? "identifier": !isSeekable ? "isSeekable" : !author ? "author" : !length ? "length" : !isStream ? "isStream" : !position ? "position" : !title ? "title" : !uri ? "uri" : !sourceName ? "sourceName" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  char Track[TRACK_LENGTH], Identifier[16], IsSeekable[TRUE_FALSE_LENGTH], Author[32], Length[16], IsStream[TRUE_FALSE_LENGTH], Position[16], Title[128], Uri[32], SourceName[16];

  snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, req.body + track->v.pos);
  snprintf(Identifier, sizeof(Identifier), "%.*s", (int)identifier->v.len, req.body + identifier->v.pos);
  snprintf(IsSeekable, sizeof(IsSeekable), "%.*s", (int)isSeekable->v.len, req.body + isSeekable->v.pos);
  snprintf(Author, sizeof(Author), "%.*s", (int)author->v.len, req.body + author->v.pos);
  snprintf(Length, sizeof(Length), "%.*s", (int)length->v.len, req.body + length->v.pos);
  snprintf(IsStream, sizeof(IsStream), "%.*s", (int)isStream->v.len, req.body + isStream->v.pos);
  snprintf(Position, sizeof(Position), "%.*s", (int)position->v.len, req.body + position->v.pos);
  snprintf(Title, sizeof(Title), "%.*s", (int)title->v.len, req.body + title->v.pos);
  snprintf(Uri, sizeof(Uri), "%.*s", (int)uri->v.len, req.body + uri->v.pos);
  snprintf(SourceName, sizeof(SourceName), "%.*s", (int)sourceName->v.len, req.body + sourceName->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseTrackSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> track: %s\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", Track, Identifier, IsSeekable, Author, Length, IsStream, Position, Title, Uri, SourceName);

  struct lavaParsedTrack *song = malloc(sizeof(struct lavaParsedTrack));

  song->track = malloc(sizeof(Track) + 1);
  song->identifier = malloc(sizeof(Identifier) + 1);
  song->isSeekable = malloc(sizeof(IsSeekable) + 1);
  song->author = malloc(sizeof(Author) + 1);
  song->length = malloc(sizeof(Length) + 1);
  song->isStream = malloc(sizeof(IsStream) + 1);
  song->position = malloc(sizeof(Position) + 1);
  song->title = malloc(sizeof(Title) + 1);
  song->uri = malloc(sizeof(Uri) + 1);
  song->sourceName = malloc(sizeof(SourceName) + 1);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for song structure.", sizeof(struct lavaParsedTrack) + sizeof(Track) + sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName) + 10);

  strncpy(song->track, Track, sizeof(Track) + 1);
  strncpy(song->identifier, Identifier, sizeof(Identifier) + 1);
  strncpy(song->isSeekable, IsSeekable, sizeof(IsSeekable) + 1);
  strncpy(song->author, Author, sizeof(Author) + 1);
  strncpy(song->length, Length, sizeof(Length) + 1);
  strncpy(song->isStream, IsStream, sizeof(IsStream) + 1);
  strncpy(song->position, Position, sizeof(Position) + 1);
  strncpy(song->title, Title, sizeof(Title) + 1);
  strncpy(song->uri, Uri, sizeof(Uri) + 1);
  strncpy(song->sourceName, SourceName, sizeof(SourceName) + 1);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strncpy] Copied %d bytes to song structure.", sizeof(Track) + sizeof(Identifier) + sizeof(IsSeekable) + sizeof(Author) + sizeof(Length) + sizeof(IsStream) + sizeof(Position) + sizeof(Title) + sizeof(Uri) + sizeof(SourceName) + 10);

  *songStruct = song;

  return COGLINK_SUCCESS;
}

int coglink_parsePlaylist(const struct lavaInfo *lavaInfo, struct httpRequest req, struct lavaParsedPlaylist **playlistStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req.body, req.size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req.body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
  }

  char *path[] = { "playlistInfo", "name" };
  jsmnf_pair *name = jsmnf_find_path(pairs, req.body, path, 2);

  path[1] = "selectedTrack";
  jsmnf_pair *selectedTrack = jsmnf_find_path(pairs, req.body, path, 2);

  if (!name || !selectedTrack) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !name ? "name" : !selectedTrack ? "selectedTrack" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  } 

  char Name[PLAYLIST_NAME_LENGTH], SelectedTrack[8];

  snprintf(Name, sizeof(Name), "%.*s", (int)name->v.len, req.body + name->v.pos);
  snprintf(SelectedTrack, sizeof(SelectedTrack), "%.*s", (int)selectedTrack->v.len, req.body + selectedTrack->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parsePlaylistSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed playlist search json, results:\n> name: %s\n> selectedTrack: %s", Name, SelectedTrack);

  struct lavaParsedPlaylist *playlist = malloc(sizeof(struct lavaParsedPlaylist));

  playlist->name = malloc(sizeof(Name) + 1);
  playlist->selectedTrack = malloc(sizeof(SelectedTrack) + 1);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for playlist structure.", sizeof(struct lavaParsedPlaylist) + sizeof(Name) + sizeof(SelectedTrack) + 2);

  strncpy(playlist->name, Name, sizeof(Name) + 1);
  strncpy(playlist->selectedTrack, SelectedTrack, sizeof(SelectedTrack) + 1);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strncpy] Copied %d bytes to playlist structure.", sizeof(Name) + sizeof(SelectedTrack) + 2);

  *playlistStruct = playlist;

  return COGLINK_SUCCESS;
}

int coglink_parseError(const struct lavaInfo *lavaInfo, struct httpRequest req, struct lavaParsedError **errorStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req.body, req.size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req.body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");
  }

  char *path[] = { "exception", "message" };
  jsmnf_pair *message = jsmnf_find_path(pairs, req.body, path, 2);

  path[1] = "severity";
  jsmnf_pair *severity = jsmnf_find_path(pairs, req.body, path, 2);

  if (!message || !severity) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !message ? "message" : !severity ? "severity" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  char Message[128], Severity[16];

  snprintf(Message, sizeof(Message), "%.*s", (int)message->v.len, req.body + message->v.pos);
  snprintf(Severity, sizeof(Severity), "%.*s", (int)severity->v.len, req.body + severity->v.pos);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> message: %s\n> severity: %s", Message, Severity);

  struct lavaParsedError *error = malloc(sizeof(struct lavaParsedError));

  error->message = malloc(sizeof(Message) + 1);
  error->severity = malloc(sizeof(Severity) + 1);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for error structure.", sizeof(struct lavaParsedError) + sizeof(Message) + sizeof(Severity) + 2);

  strncpy(error->message, Message, sizeof(Message) + 1);
  strncpy(error->severity, Severity, sizeof(Severity) + 1);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strncpy] Copied %d bytes to error structure.", sizeof(Message) + sizeof(Severity) + 2);

  *errorStruct = error;

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