#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#include <coglink/lavalink.h>
#include <coglink/definations.h>

size_t __coglink_WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t writeSize = size * nmemb;
  struct httpRequest *mem = (struct httpRequest *)userp;

  char *ptr = realloc(mem->body, mem->size + writeSize + 1);
  if (!ptr) {
    log_fatal("[SYSTEM] Not enough memory to realloc.\n");
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

  char lavaURL[1024];
  if (lavaInfo->node->ssl) snprintf(lavaURL, sizeof(lavaURL), "https://%s/loadtracks?identifier=", lavaInfo->node->hostname);
  else snprintf(lavaURL, sizeof(lavaURL), "http://%s/loadtracks?identifier=", lavaInfo->node->hostname);

  if (0 != strncmp(songEncoded, "https://", 8)) strncat(lavaURL, "ytsearch:", sizeof(lavaURL) - 1);
  strncat(lavaURL, songEncoded, sizeof(lavaURL) - 1);

  curl_free(songEncoded);

  if (!curl) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] Error while initializing libcurl.");
    return COGLINK_LIBCURL_FAIL_INITIALIZE;
  }

  struct httpRequest req;

  req.body = malloc(1);
  req.size = 0;

  CURLcode cRes = curl_easy_setopt(curl, CURLOPT_URL, lavaURL);

  if (cRes != CURLE_OK) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] curl_easy_setopt [1] failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAIL_SETOPT;
  }

  struct curl_slist *chunk = NULL;
    
  if (lavaInfo->node->password) {
    char AuthorizationH[256];
    snprintf(AuthorizationH, sizeof(AuthorizationH), "Authorization: %s", lavaInfo->node->password);
    chunk = curl_slist_append(chunk, AuthorizationH);
  }
  chunk = curl_slist_append(chunk, "Client-Name: Coglink");
  chunk = curl_slist_append(chunk, "User-Agent: libcurl");
//  chunk = curl_slist_append(chunk, "Cleanup-Threshold: 600");

  cRes = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
  if (cRes != CURLE_OK) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] curl_easy_setopt [2] failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAIL_SETOPT;
  } 

  cRes = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __coglink_WriteMemoryCallback);
  if (cRes != CURLE_OK) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] curl_easy_setopt [3] failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAIL_SETOPT;
  }

  cRes = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&req);
  if (cRes != CURLE_OK) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] curl_easy_setopt [4] failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAIL_SETOPT;
  }

  cRes = curl_easy_perform(curl);
  if (cRes != CURLE_OK) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] curl_easy_perform failed: %s\n", curl_easy_strerror(cRes));
    return COGLINK_LIBCURL_FAIL_PERFORM;
  }

  curl_easy_cleanup(curl);
  curl_slist_free_all(chunk);
  curl_global_cleanup(); 

  *res = req;
  
  if (lavaInfo->debug) log_debug("[coglink:libcurl] Search song done, response: %s", res->body);

  return COGLINK_SUCCESS;
}

void coglink_searchCleanup(struct httpRequest req) {
  free(req.body);
}

int coglink_parseSearch(struct lavaInfo *lavaInfo, struct httpRequest req, char *songPos, struct lavaSong **songStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, req.body, req.size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, req.body, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
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
    if (lavaInfo->debug) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !track ? "track" : !identifier ? "identifier": !isSeekable ? "isSeekable" : !author ? "author" : !length ? "length" : !isStream ? "isStream" : !position ? "position" : !title ? "title" : !uri ? "uri" : !sourceName ? "sourceName" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  char Track[512], Identifier[16], IsSeekable[8], Author[64], Length[32], IsStream[8], Position[16], Title[256], Uri[32], SourceName[16];

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

  if (lavaInfo->debug) log_debug("[coglink:jsmn-find] Parsed song search json, results:\n> track: %s\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", Track, Identifier, IsSeekable, Author, Length, IsStream, Position, Title, Uri, SourceName);

  struct lavaSong *song = malloc(sizeof(struct lavaSong));

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

  strcpy(song->track, Track);
  strcpy(song->identifier, Identifier);
  strcpy(song->isSeekable, IsSeekable);
  strcpy(song->author, Author);
  strcpy(song->length, Length);
  strcpy(song->isStream, IsStream);
  strcpy(song->position, Position);
  strcpy(song->title, Title);
  strcpy(song->uri, Uri);
  strcpy(song->sourceName, SourceName);

  *songStruct = song;

  return COGLINK_SUCCESS;
}