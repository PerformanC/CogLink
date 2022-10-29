#include <stdio.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

#define COGLINK_SUCCESS 0

#define COGLINK_JSMNF_ERROR_PARSE -1
#define COGLINK_JSMNF_ERROR_LOAD -2
#define COGLINK_JSMNF_ERROR_FIND -3

#define COGLINK_LIBCURL_FAIL_INITIALIZE -4
#define COGLINK_LIBCURL_FAIL_SETOPT -5


/*
  STRUCTURES
*/

struct httpRequest {
  char *body;
  size_t size;
};

struct musicSearch {
  char *body;
  size_t size;
};

static size_t __coglink_WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct httpRequest *mem = (struct httpRequest *)userp;

  char *ptr = realloc(mem->body, mem->size + realsize + 1);
  if (!ptr) {
    log_fatal("[SYSTEM] Not enough memory to realloc.\n");
    return 1;
  }

  mem->body = ptr;
  memcpy(&(mem->body[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->body[mem->size] = 0;

  return realsize;
}

struct lavaNode {
  char *name;
  char *hostname;
  char *password;
  char *shards;
  char *botId;
  int ssl;
};

struct lavaEvents {
  // BASICS
  void (*onRaw)();
  void (*onConnect)();
  void (*onClose)(enum ws_close_reason wscode, const char *reason);
  // MUSIC
  void (*onTrackStart)(char *track, u64snowflake guildId);
};

struct lavaInfo {
  struct lavaEvents *events;
  CURLM *mhandle;
  struct websockets *ws;
  uint64_t tstamp;
  struct lavaNode *node;
  int debug;
};

struct lavaMusic {
  char *track;
  char *identifier;
  char *isSeekable;
  char *author;
  char *length;
  char *isStream;
  char *position;
  char *title;
  char *uri;
  char *sourceName;
};

/*
  STRUCTURES
*/

/*
  EVENTS
*/


void onConnectEvent(void *data, struct websockets *ws, struct ws_info *info, const char *protocols) {
  (void)ws; (void)info; (void)protocols;
  struct lavaInfo *lavaInfo = data;
  if (lavaInfo->events->onConnect) lavaInfo->events->onConnect();
}

void onCloseEvent(void *data, struct websockets *ws, struct ws_info *info, enum ws_close_reason wscode, const char *reason, size_t len) {
  (void) ws; (void) info; (void) len;
  struct lavaInfo *lavaInfo = data;
  if (lavaInfo->events->onClose) lavaInfo->events->onClose(wscode, reason);
}

void onTextEvent(void *data, struct websockets *ws, struct ws_info *info, const char *text, size_t len) {
  (void) ws; (void) info;
  struct lavaInfo *lavaInfo = data;

  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, text, len, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return;
  }

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, text, tokens, parser.toknext, pairs, 1024);

  if (r < 0) {
    if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return;
  }

  jsmnf_pair *op = jsmnf_find(pairs, text, "op", 2);

  if (!op) {
    if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to find op.");
    return;
  }

  char Op[16];
  snprintf(Op, sizeof(Op), "%.*s", (int)op->v.len, text + op->v.pos);

  if (0 == strcmp(Op, "event")) {
    jsmnf_pair *type = jsmnf_find(pairs, text, "type", 4);

    if (!type) {
      if (lavaInfo->debug) log_error("[coglink:-find] Failed to find type.");
      return;
    }

    char Type[16];
    snprintf(Type, sizeof(Type), "%.*s", (int)type->v.len, text + type->v.pos);

    jsmnf_pair *guildId = jsmnf_find(pairs, text, "guildId", 7);

    if (!guildId) {
      if (lavaInfo->debug) log_error("[coglink:-find] Failed to find guildId.");
      return;
    }

    char u_guildId[32];
    snprintf(u_guildId, sizeof(u_guildId), "%.*s", (int)guildId->v.len, text + guildId->v.pos);

    if (0 == strcmp(Type, "TrackStartEvent")) {
      jsmnf_pair *track = jsmnf_find(pairs, text, "track", 5);

      if (!track) {
        if (lavaInfo->debug) log_error("[coglink:-find] Failed to find track.");
        return;
      }

      char Track[512];
      snprintf(Track, sizeof(Track), "%.*s", (int)track->v.len, text + track->v.pos);

      if (lavaInfo->events->onTrackStart) lavaInfo->events->onTrackStart(Track, strtoull(u_guildId, NULL, 10));
    }
  }
}

/*
  EVENTS
*/

int coglink_searchMusic(struct lavaInfo *lavaInfo, char *music, struct httpRequest *res) {
  curl_global_init(CURL_GLOBAL_ALL);

  CURL *curl = curl_easy_init();
  char *musicE = curl_easy_escape(curl, music, strlen(music));

  char lavaURL[1024];
  if (lavaInfo->node->ssl) snprintf(lavaURL, sizeof(lavaURL), "https://%s/loadtracks?identifier=", lavaInfo->node->hostname);
  else snprintf(lavaURL, sizeof(lavaURL), "http://%s/loadtracks?identifier=", lavaInfo->node->hostname);

  if (0 != strncmp(music, "https://", 8)) strncat(lavaURL, "ytsearch:", sizeof(lavaURL) - 1);
  strncat(lavaURL, musicE, sizeof(lavaURL) - 1);

  curl_free(musicE);

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
    return COGLINK_LIBCURL_FAIL_SETOPT;
  }

  curl_easy_cleanup(curl);
  curl_slist_free_all(chunk);
  curl_global_cleanup(); 

  *res = req;
  
  if (lavaInfo->debug) log_debug("[coglink:libcurl] Search music done, response: %s", res->body);

  return COGLINK_SUCCESS;
}

void coglink_searchMusicCleanup(struct httpRequest req) {
  free(req.body);
}

int coglink_parseMusicSearch(struct lavaInfo *lavaInfo, struct httpRequest req, char *musicPos, struct lavaMusic *musicStruct) {
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

  char *path[] = { "tracks", musicPos, "track", NULL };
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

  if (lavaInfo->debug) log_debug("[coglink:jsmn-find] Parsed music search json, results:\n> track: %s\n> identifier: %s\n> isSeekable: %s\n> author: %s\n> length: %s\n> isStream: %s\n> position: %s\n> title: %s\n> uri: %s\n> sourceName: %s", Track, Identifier, IsSeekable, Author, Length, IsStream, Position, Title, Uri, SourceName);

  *musicStruct = (struct lavaMusic) {
    .track = Track,
    .identifier = Identifier,
    .isSeekable = IsSeekable,
    .author = Author,
    .length = Length,
    .isStream = IsStream,
    .position = Position,
    .title = Title,
    .uri = Uri,
    .sourceName = SourceName
  };

  return COGLINK_SUCCESS;
}

void coglink_wsLoop(struct lavaInfo *lavaInfo) {
  ws_easy_run(lavaInfo->ws, 5, &lavaInfo->tstamp);
}

void __coglink_sendPayload(struct lavaInfo *lavaInfo, char payload[], char *payloadOP) {
  if (ws_send_text(lavaInfo->ws, NULL, payload, strlen(payload)) == false) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] Something went wrong while sending a payload with op %s to Lavalink.", payloadOP);
    return;
  } else {
    if (lavaInfo->debug) log_debug("[coglink:libcurl] Sucessfully sent a payload with op %s to Lavalink.", payloadOP);
    ws_easy_run(lavaInfo->ws, 5, &lavaInfo->tstamp);
  }
}

void coglink_playMusic(struct lavaInfo *lavaInfo, char *track, u64snowflake guildId) {
  char payload[2048];
  snprintf(payload, sizeof(payload), "{\"op\":\"play\",\"guildId\":\"%"PRIu64"\",\"track\":\"%s\",\"noReplace\":false,\"pause\":false}", guildId, track);

  __coglink_sendPayload(lavaInfo, payload, "play");
}

void coglink_joinVoiceChannel(struct lavaInfo *lavaInfo, struct discord *client, u64snowflake voiceChannelId, u64snowflake guildId) {
  char joinVCPayload[128];
  snprintf(joinVCPayload, sizeof(joinVCPayload), "{\"op\":4,\"d\":{\"guild_id\":%"PRIu64",\"channel_id\":\"%"PRIu64"\",\"self_mute\":false,\"self_deaf\":true}}", guildId, voiceChannelId);

  if (ws_send_text(client->gw.ws, NULL, joinVCPayload, strlen(joinVCPayload)) == false) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] Something went wrong while sending a payload with op 4 to Discord.");
    return;
  } else {
    if (lavaInfo->debug) log_debug("[coglink:libcurl] Sucessfully sent the payload with op 4 to Discord.");
  }
}

char sessionID[128];

int coglink_handleScheduler(struct lavaInfo *lavaInfo, struct discord *client, const char data[], size_t size, enum discord_gateway_events event) {
  (void)client;
  switch (event) {
    case DISCORD_EV_VOICE_STATE_UPDATE: {
      jsmn_parser parser;
      jsmntok_t tokens[256];

      jsmn_init(&parser);
      int r = jsmn_parse(&parser, data, size, tokens, sizeof(tokens));

      if (r < 0) {
        if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to parse JSON.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_loader loader;
      jsmnf_pair pairs[128];

      jsmnf_init(&loader);
      r = jsmnf_load(&loader, data, tokens, parser.toknext, pairs, 128);

      if (r < 0) {
        if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", 8);

      if (!VGI) {
        if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to find guild_id.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_pair *VUI = jsmnf_find(pairs, data, "user_id", 7);

      if (!VUI) {
        if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to find user_id.");
        return DISCORD_EVENT_IGNORE;
      }

      char userId[32];
      snprintf(userId, sizeof(userId), "%.*s", (int)VUI->v.len, data + VUI->v.pos);

      jsmnf_pair *SSI = jsmnf_find(pairs, data, "session_id", 10);

      if (!SSI) {
        if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to find session_id.");
        return DISCORD_EVENT_IGNORE;
      }

      char sessionId[128];

      snprintf(sessionId, sizeof(sessionId), "%.*s", (int)SSI->v.len, data + SSI->v.pos);

      if (lavaInfo->debug) log_debug("[coglink:jsmn-find] Parsed voice state update json, results:\n> guild_id: %.*s\n> user_id: %s\n> session_id: %s", (int)VGI->v.len, data + VGI->v.pos, userId, sessionId);

      if (0 == strcmp(userId, lavaInfo->node->botId)) {
        if (0 != strcmp(sessionId, "null")) {
          snprintf(sessionID, sizeof(sessionID), "%s", sessionId);
          if (lavaInfo->debug) log_debug("[coglink:jsmn-find] The user that got updated is the bot, saving the sessionId.");
        } else {
          return DISCORD_EVENT_IGNORE;
        }
      }

    } return DISCORD_EVENT_IGNORE;
    case DISCORD_EV_VOICE_SERVER_UPDATE: {
      jsmn_parser parser;
      jsmntok_t tokens[256];

      jsmn_init(&parser);
      int r = jsmn_parse(&parser, data, size, tokens, sizeof(tokens));

      if (r < 0) {
        if (lavaInfo->debug) log_error("[jsmn-find] Failed to parse JSON.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_loader loader;
      jsmnf_pair pairs[128];

      jsmnf_init(&loader);
      r = jsmnf_load(&loader, data, tokens, parser.toknext, pairs, 128);

      if (r < 0) {
        if (lavaInfo->debug) log_error("[jsmn-find] Failed to load jsmn-find.");
        return DISCORD_EVENT_IGNORE;
      }

      jsmnf_pair *VGI = jsmnf_find(pairs, data, "guild_id", 8);

      if (!VGI) {
        if (lavaInfo->debug) log_error("[jsmn-find] Failed to find guild_id.");
        return DISCORD_EVENT_IGNORE;
      }
  
      char VUP[1024];
      snprintf(VUP, sizeof(VUP), "{\"op\":\"voiceUpdate\",\"guildId\":\"%.*s\",\"sessionId\":\"%s\",\"event\":%.*s}", (int)VGI->v.len, data + VGI->v.pos, sessionID, (int)size, data);

      __coglink_sendPayload(lavaInfo, VUP, "voiceUpdate");
    } return DISCORD_EVENT_IGNORE;
    default:
      return DISCORD_EVENT_MAIN_THREAD;
  }
}

void coglink_connectNodeCleanup(struct lavaInfo *lavaInfo) {
  ws_end(lavaInfo->ws);
  ws_cleanup(lavaInfo->ws);
  curl_multi_cleanup(lavaInfo->mhandle);
}

int coglink_connectNode(struct lavaInfo *lavaInfo, struct lavaNode *node) {
  struct ws_callbacks callbacks = {
    .on_text = &onTextEvent,
    .on_connect = &onConnectEvent,
    .on_close = &onCloseEvent,
    .data = (void *)lavaInfo
  };

  CURLM *mhandle = curl_multi_init();
  struct websockets *ws = ws_init(&callbacks, mhandle, NULL);

  char hostname[256];
  if (node->ssl) snprintf(hostname, sizeof(hostname), "wss://%s", node->hostname);
  else snprintf(hostname, sizeof(hostname), "ws://%s", node->hostname);

  ws_set_url(ws, hostname, NULL);

  ws_start(ws);

  ws_add_header(ws, "Authorization", node->password);
  ws_add_header(ws, "Num-Shards", node->shards);
  ws_add_header(ws, "User-Id", node->botId);
  ws_add_header(ws, "Client-Name", "coglink");

  lavaInfo->mhandle = mhandle;
  lavaInfo->ws = ws;
  uint64_t tstamp = 0;
  lavaInfo->tstamp = tstamp;
  lavaInfo->node = node;

  return COGLINK_SUCCESS;
}