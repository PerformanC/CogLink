#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/websockets.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>

size_t __coglink_WriteMemoryCallback(void *data, size_t size, size_t nmemb, void *userp) {
  size_t writeSize = size * nmemb;
  struct coglink_requestInformation *mem = userp;

  char *ptr = realloc(mem->body, mem->size + writeSize + 1);
  if (!ptr) {
    perror("[SYSTEM] Not enough memory to realloc.\n");
    return 1;
  }

  mem->body = ptr;
  memcpy(&(mem->body[mem->size]), data, writeSize);
  mem->size += writeSize;
  mem->body[mem->size] = 0;

  return writeSize;
}

size_t __coglink_WriteMemoryCallbackNoSave(void *data, size_t size, size_t nmemb, void *userp) {
  (void) data; (void) size; (void) userp;
  return nmemb;
}

int __coglink_checkCurlCommand(struct coglink_lavaInfo *lavaInfo, CURL *curl, CURLcode cRes, char *pos, int additionalDebugging, int getResponse, struct coglink_requestInformation *res) {
  if (cRes != CURLE_OK) {
    if (lavaInfo->debugging->allDebugging || additionalDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] curl_easy_setopt [%s] failed: %s\n", pos, curl_easy_strerror(cRes));

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    if (getResponse) free(res->body);

    return COGLINK_LIBCURL_FAILED_SETOPT;
  }
  return COGLINK_SUCCESS;
}

int _coglink_performRequest(struct coglink_lavaInfo *lavaInfo, struct coglink_nodeInfo *nodeInfo, struct coglink_requestInformation *res, struct __coglink_requestConfig *config) {
  if (!config->usedCURL) curl_global_init(CURL_GLOBAL_ALL);
  if (!config->useVPath) config->useVPath = 1;

  CURL *curl = config->usedCURL;
  if (!config->usedCURL) curl = curl_easy_init();

  if (!curl) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] Error while initializing libcurl.");

    curl_global_cleanup();

    return COGLINK_LIBCURL_FAILED_INITIALIZE;
  }

  char lavaURL[128 + 12 + 9 + config->pathLength];

  if (config->useVPath) {
    if (nodeInfo->node.ssl) snprintf(lavaURL, sizeof(lavaURL), "https://%s/v4%s", nodeInfo->node.hostname, config->path);
    else snprintf(lavaURL, sizeof(lavaURL), "http://%s/v4%s", nodeInfo->node.hostname, config->path);
  } else {
    if (nodeInfo->node.ssl) snprintf(lavaURL, sizeof(lavaURL), "https://%s%s", nodeInfo->node.hostname, config->path);
    else snprintf(lavaURL, sizeof(lavaURL), "http://%s%s", nodeInfo->node.hostname, config->path);
  }

  CURLcode cRes = curl_easy_setopt(curl, CURLOPT_URL, lavaURL);
  if (__coglink_checkCurlCommand(lavaInfo, curl, cRes, "1", config->additionalDebuggingError, config->getResponse, res) != COGLINK_SUCCESS) return COGLINK_LIBCURL_FAILED_SETOPT;

  struct curl_slist *chunk = NULL;
    
  if (nodeInfo->node.password) {
    char AuthorizationH[256];
    snprintf(AuthorizationH, sizeof(AuthorizationH), "Authorization: %s", nodeInfo->node.password);
    chunk = curl_slist_append(chunk, AuthorizationH);

    if (lavaInfo->debugging->allDebugging || config->additionalDebuggingSuccess || lavaInfo->debugging->curlSuccessDebugging) log_debug("[coglink:libcurl] Authorization header set.");
  }

  if (config->body) {
    chunk = curl_slist_append(chunk, "Content-Type: application/json");
    if (lavaInfo->debugging->allDebugging || config->additionalDebuggingSuccess || lavaInfo->debugging->curlSuccessDebugging) log_debug("[coglink:libcurl] Content-Type header set.");
  }

  chunk = curl_slist_append(chunk, "Client-Name: Coglink");
  if (lavaInfo->debugging->allDebugging || config->additionalDebuggingSuccess || lavaInfo->debugging->curlSuccessDebugging) log_debug("[coglink:libcurl] Client-Name header set.");

  chunk = curl_slist_append(chunk, "User-Agent: libcurl");
  if (lavaInfo->debugging->allDebugging || config->additionalDebuggingSuccess || lavaInfo->debugging->curlSuccessDebugging) log_debug("[coglink:libcurl] User-Agent header set.");

  cRes = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
  if (__coglink_checkCurlCommand(lavaInfo, curl, cRes, "2", config->additionalDebuggingError, config->getResponse, res) != COGLINK_SUCCESS) {
    curl_slist_free_all(chunk);
    return COGLINK_LIBCURL_FAILED_SETOPT;
  }

  if (config->requestType == __COGLINK_DELETE_REQ || config->requestType == __COGLINK_PATCH_REQ) {
    if (config->requestType == __COGLINK_DELETE_REQ) cRes = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    else if (config->requestType == __COGLINK_PATCH_REQ) cRes = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");

    if (__coglink_checkCurlCommand(lavaInfo, curl, cRes, "3", config->additionalDebuggingError, config->getResponse, res) != COGLINK_SUCCESS) {
      curl_slist_free_all(chunk);
      return COGLINK_LIBCURL_FAILED_SETOPT;
    }
  }

  if (config->getResponse) {
    (*res).body = malloc(1);
    (*res).size = 0;

    cRes = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __coglink_WriteMemoryCallback);

    if (__coglink_checkCurlCommand(lavaInfo, curl, cRes, "4", config->additionalDebuggingError, config->getResponse, res) != COGLINK_SUCCESS) {
      curl_slist_free_all(chunk);
      return COGLINK_LIBCURL_FAILED_SETOPT;
    }

    cRes = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&(*res));
    if (__coglink_checkCurlCommand(lavaInfo, curl, cRes, "5", config->additionalDebuggingError, config->getResponse, res) != COGLINK_SUCCESS) {
      curl_slist_free_all(chunk);
      return COGLINK_LIBCURL_FAILED_SETOPT;
    }
  } else {
    cRes = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __coglink_WriteMemoryCallbackNoSave);

    if (__coglink_checkCurlCommand(lavaInfo, curl, cRes, "6", config->additionalDebuggingError, config->getResponse, res) != COGLINK_SUCCESS) {
      curl_slist_free_all(chunk);
      return COGLINK_LIBCURL_FAILED_SETOPT;
    }
  }

  if (config->body) {
    cRes = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, config->body);
    if (__coglink_checkCurlCommand(lavaInfo, curl, cRes, "7", config->additionalDebuggingError, config->getResponse, res) != COGLINK_SUCCESS) {
      curl_slist_free_all(chunk);
      return COGLINK_LIBCURL_FAILED_SETOPT;
    }

    cRes = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, config->bodySize);
    if (__coglink_checkCurlCommand(lavaInfo, curl, cRes, "8", config->additionalDebuggingError, config->getResponse, res) != COGLINK_SUCCESS) {
      curl_slist_free_all(chunk);
      return COGLINK_LIBCURL_FAILED_SETOPT;
    }
  }

  cRes = curl_easy_perform(curl);
  if (cRes != CURLE_OK) {
    if (lavaInfo->debugging->allDebugging || config->additionalDebuggingError || lavaInfo->debugging->curlErrorsDebugging) log_fatal("[coglink:libcurl] curl_easy_perform failed: %s\n", curl_easy_strerror(cRes));

    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);
    curl_global_cleanup();
    if (config->getResponse) free((*res).body);

    return COGLINK_LIBCURL_FAILED_PERFORM;
  }

  curl_easy_cleanup(curl);
  curl_slist_free_all(chunk);
  curl_global_cleanup();
  
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->curlSuccessDebugging) log_debug("[coglink:libcurl] Performed request successfully.");

  return COGLINK_SUCCESS;
}

int _coglink_checkParse(struct coglink_lavaInfo *lavaInfo, jsmnf_pair *field, char *fieldName) {
  if (!field) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->checkParseErrorsDebugging) log_error("[coglink:jsmn-find] Failed to find %s field.", fieldName);
    return COGLINK_JSMNF_ERROR_FIND;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->checkParseSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully found %s field.", fieldName);
  return COGLINK_PROCEED;
}

int _coglink_selectBestNode(struct coglink_lavaInfo *lavaInfo) {
  int i = -1;
  int bestStatsNode = 0, bestStats = -1;
  while (i++ <= lavaInfo->nodeCount) {
    int systemLoad = lavaInfo->nodes[i].stats.systemLoad ? lavaInfo->nodes[i].stats.systemLoad : 0;
    int cores = lavaInfo->nodes[i].stats.cores ? lavaInfo->nodes[i].stats.cores : 0;

    int stats = (systemLoad / cores) * 100;

    if (stats < bestStats) {
      bestStats = stats;
      bestStatsNode = i;
    }
  }

  return bestStatsNode;
}

int _coglink_findNode(struct coglink_lavaInfo *lavaInfo, char *hostname) {
  for (int i = 0; i < lavaInfo->nodeCount; i++) {
    if (strcmp(lavaInfo->nodes[i].node.hostname, hostname) == 0) return i;
  }
  return -1;
}

int _coglink_IOPoller(struct io_poller *io, CURLM *multi, void *data) {
  (void) io; (void) multi;
  struct coglink_nodeInfo *nodeInfo = data;
  return !ws_multi_socket_run(nodeInfo->ws, &nodeInfo->tstamp) ? COGLINK_WAIT : COGLINK_SUCCESS;
}

struct coglink_parsedTrack _coglink_buildTrackStruct(struct coglink_lavaInfo *lavaInfo, jsmnf_pair pairs[], const char *text) {
  char *path[] = { "track", "encoded", NULL };
  jsmnf_pair *encoded = jsmnf_find_path(pairs, text, path, 2);
  if (_coglink_checkParse(lavaInfo, encoded, "encoded") != COGLINK_PROCEED) return (struct coglink_parsedTrack) { 0 };

  path[1] = "info";
  path[2] = "identifier";
  jsmnf_pair *identifier = jsmnf_find_path(pairs, text, path, 3);
  if (_coglink_checkParse(lavaInfo, identifier, "identifier") != COGLINK_PROCEED) return (struct coglink_parsedTrack) { 0 };

  path[2] = "isSeekable";
  jsmnf_pair *isSeekable = jsmnf_find_path(pairs, text, path, 3);
  if (_coglink_checkParse(lavaInfo, isSeekable, "isSeekable") != COGLINK_PROCEED) return (struct coglink_parsedTrack) { 0 };

  path[2] = "author";
  jsmnf_pair *author = jsmnf_find_path(pairs, text, path, 3);
  if (_coglink_checkParse(lavaInfo, author, "author") != COGLINK_PROCEED) return (struct coglink_parsedTrack) { 0 };

  path[2] = "length";
  jsmnf_pair *length = jsmnf_find_path(pairs, text, path, 3);
  if (_coglink_checkParse(lavaInfo, length, "length") != COGLINK_PROCEED) return (struct coglink_parsedTrack) { 0 };

  path[2] = "isStream";
  jsmnf_pair *isStream = jsmnf_find_path(pairs, text, path, 3);
  if (_coglink_checkParse(lavaInfo, isStream, "isStream") != COGLINK_PROCEED) return (struct coglink_parsedTrack) { 0 };

  path[2] = "position";
  jsmnf_pair *position = jsmnf_find_path(pairs, text, path, 3);
  if (_coglink_checkParse(lavaInfo, position, "position") != COGLINK_PROCEED) return (struct coglink_parsedTrack) { 0 };

  path[2] = "title";
  jsmnf_pair *title = jsmnf_find_path(pairs, text, path, 3);
  if (_coglink_checkParse(lavaInfo, title, "title") != COGLINK_PROCEED) return (struct coglink_parsedTrack) { 0 };

  path[2] = "uri";
  jsmnf_pair *uri = jsmnf_find_path(pairs, text, path, 3);

  path[2] = "artworkUrl";
  jsmnf_pair *artworkUrl = jsmnf_find_path(pairs, text, path, 3);

  path[2] = "isrc";
  jsmnf_pair *isrc = jsmnf_find_path(pairs, text, path, 3);

  path[2] = "sourceName";
  jsmnf_pair *sourceName = jsmnf_find_path(pairs, text, path, 3);
  if (_coglink_checkParse(lavaInfo, sourceName, "sourceName") != COGLINK_PROCEED) return (struct coglink_parsedTrack) { 0 };

  struct coglink_parsedTrack parsedTrack;

  snprintf(parsedTrack.encoded, sizeof(parsedTrack.encoded), "%.*s", (int)encoded->v.len, text + encoded->v.pos);
  snprintf(parsedTrack.identifier, sizeof(parsedTrack.identifier), "%.*s", (int)identifier->v.len, text + identifier->v.pos);
  snprintf(parsedTrack.isSeekable, sizeof(parsedTrack.isSeekable), "%.*s", (int)isSeekable->v.len, text + isSeekable->v.pos);
  snprintf(parsedTrack.author, sizeof(parsedTrack.author), "%.*s", (int)author->v.len, text + author->v.pos);
  snprintf(parsedTrack.length, sizeof(parsedTrack.length), "%.*s", (int)length->v.len, text + length->v.pos);
  snprintf(parsedTrack.isStream, sizeof(parsedTrack.isStream), "%.*s", (int)isStream->v.len, text + isStream->v.pos);
  snprintf(parsedTrack.position, sizeof(parsedTrack.position), "%.*s", (int)position->v.len, text + position->v.pos);
  snprintf(parsedTrack.title, sizeof(parsedTrack.title), "%.*s", (int)title->v.len, text + title->v.pos);
  if (_coglink_checkParse(lavaInfo, uri, "uri") == COGLINK_PROCEED) snprintf(parsedTrack.uri, sizeof(parsedTrack.uri), "%.*s", (int)uri->v.len, text + uri->v.pos);
  if (_coglink_checkParse(lavaInfo, artworkUrl, "artworkUrl") == COGLINK_PROCEED) snprintf(parsedTrack.artworkUrl, sizeof(parsedTrack.artworkUrl), "%.*s", (int)artworkUrl->v.len, text + artworkUrl->v.pos);
  if (_coglink_checkParse(lavaInfo, isrc, "isrc") == COGLINK_PROCEED) snprintf(parsedTrack.isrc, sizeof(parsedTrack.isrc), "%.*s", (int)isrc->v.len, text + isrc->v.pos);
  snprintf(parsedTrack.sourceName, sizeof(parsedTrack.sourceName), "%.*s", (int)sourceName->v.len, text + sourceName->v.pos);

  return parsedTrack;
}