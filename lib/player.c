#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>
#include <coglink/player.h>

int coglink_createPlayer(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId) {
  int node = _coglink_selectBestNode(lavaInfo);

  if (node == -1) return COGLINK_NO_NODES;

  _coglink_createPlayer(guildId, node);

  return node;
}

int coglink_getPlayers(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, struct coglink_requestInformation *res) {
  int node = _coglink_findPlayerNode(guildId);

  char reqPath[35];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players", lavaInfo->nodes[node].sessionId);

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], res, 
                                 &(struct __coglink_requestConfig) {
                                   .requestType = __COGLINK_GET_REQ,
                                   .path = reqPath,
                                   .pathLength = pathLen,
                                   .getResponse = 1
                                 });
}

int coglink_parseGetPlayers(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, char *pos, struct coglink_playerInfo *playerInfoStruct) {
  if (res->body[0] == '[' && res->body[1] == ']') return COGLINK_NO_PLAYERS;

  jsmn_parser parser;
  jsmntok_t tokens[1024];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[1024];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  char *path[] = { pos, "guildId", NULL, NULL, NULL };

  jsmnf_pair *guildId = jsmnf_find_path(pairs, res->body, path, 2);
  
  char GuildId[COGLINK_GUILD_ID_LENGTH];
  snprintf(GuildId, sizeof(GuildId), "%.*s", (int)guildId->v.len, res->body + guildId->v.pos);

  path[1] = "track";
  path[2] = "encoded";
  jsmnf_pair *encoded = jsmnf_find_path(pairs, res->body, path, 2);

  if (_coglink_checkParse(lavaInfo, encoded, "encoded") == COGLINK_SUCCESS) {
    path[2] = "info";
    path[3] = "identifier";
    jsmnf_pair *identifier = jsmnf_find_path(pairs, res->body, path, 4);
    if (_coglink_checkParse(lavaInfo, identifier, "identifier") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

    path[3] = "isSeekable";
    jsmnf_pair *isSeekable = jsmnf_find_path(pairs, res->body, path, 4);
    if (_coglink_checkParse(lavaInfo, isSeekable, "isSeekable") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

    path[3] = "author";
    jsmnf_pair *author = jsmnf_find_path(pairs, res->body, path, 4);
    if (_coglink_checkParse(lavaInfo, author, "author") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

    path[3] = "length";
    jsmnf_pair *length = jsmnf_find_path(pairs, res->body, path, 4);
    if (_coglink_checkParse(lavaInfo, length, "length") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

    path[3] = "isStream";
    jsmnf_pair *isStream = jsmnf_find_path(pairs, res->body, path, 4);
    if (_coglink_checkParse(lavaInfo, isStream, "isStream") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

    path[3] = "position";
    jsmnf_pair *position = jsmnf_find_path(pairs, res->body, path, 4);
    if (_coglink_checkParse(lavaInfo, position, "position") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

    path[3] = "title";
    jsmnf_pair *title = jsmnf_find_path(pairs, res->body, path, 4);
    if (_coglink_checkParse(lavaInfo, title, "title") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

    path[3] = "uri";
    jsmnf_pair *uri = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "artworkUrl";
    jsmnf_pair *artworkUrl = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "isrc";
    jsmnf_pair *isrc = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "sourceName";
    jsmnf_pair *sourceName = jsmnf_find_path(pairs, res->body, path, 4);
    if (_coglink_checkParse(lavaInfo, sourceName, "sourceName") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_PARSE;

    snprintf(playerInfoStruct->track->encoded, sizeof(playerInfoStruct->track->encoded), "%.*s", (int)encoded->v.len, res->body + encoded->v.pos);
    snprintf(playerInfoStruct->track->info->identifier, sizeof(playerInfoStruct->track->info->identifier), "%.*s", (int)identifier->v.len, res->body + identifier->v.pos);
    snprintf(playerInfoStruct->track->info->isSeekable, sizeof(playerInfoStruct->track->info->isSeekable), "%.*s", (int)isSeekable->v.len, res->body + isSeekable->v.pos);
    snprintf(playerInfoStruct->track->info->author, sizeof(playerInfoStruct->track->info->author), "%.*s", (int)author->v.len, res->body + author->v.pos);
    snprintf(playerInfoStruct->track->info->length, sizeof(playerInfoStruct->track->info->length), "%.*s", (int)length->v.len, res->body + length->v.pos);
    snprintf(playerInfoStruct->track->info->isStream, sizeof(playerInfoStruct->track->info->isStream), "%.*s", (int)isStream->v.len, res->body + isStream->v.pos);
    snprintf(playerInfoStruct->track->info->position, sizeof(playerInfoStruct->track->info->position), "%.*s", (int)position->v.len, res->body + position->v.pos);
    snprintf(playerInfoStruct->track->info->title, sizeof(playerInfoStruct->track->info->title), "%.*s", (int)title->v.len, res->body + title->v.pos);
    if (_coglink_checkParse(lavaInfo, uri, "uri") == COGLINK_SUCCESS) snprintf(playerInfoStruct->track->info->uri, sizeof(playerInfoStruct->track->info->uri), "%.*s", (int)uri->v.len, res->body + uri->v.pos);
    if (_coglink_checkParse(lavaInfo, artworkUrl, "artworkUrl") == COGLINK_SUCCESS) snprintf(playerInfoStruct->track->info->artworkUrl, sizeof(playerInfoStruct->track->info->artworkUrl), "%.*s", (int)artworkUrl->v.len, res->body + artworkUrl->v.pos);
    if (_coglink_checkParse(lavaInfo, isrc, "isrc") == COGLINK_SUCCESS) snprintf(playerInfoStruct->track->info->isrc, sizeof(playerInfoStruct->track->info->isrc), "%.*s", (int)isrc->v.len, res->body + isrc->v.pos);
    snprintf(playerInfoStruct->track->info->sourceName, sizeof(playerInfoStruct->track->info->sourceName), "%.*s", (int)sourceName->v.len, res->body + sourceName->v.pos);
  }

  path[1] = "volume";
  jsmnf_pair *volume = jsmnf_find_path(pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, volume, "volume") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "paused";
  jsmnf_pair *paused = jsmnf_find_path(pairs, res->body, path, 2);
  if (_coglink_checkParse(lavaInfo, paused, "paused") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND; 

  snprintf(playerInfoStruct->volume, sizeof(playerInfoStruct->volume), "%.*s", (int)volume->v.len, res->body + volume->v.pos);
  snprintf(playerInfoStruct->paused, sizeof(playerInfoStruct->paused), "%.*s", (int)paused->v.len, res->body + paused->v.pos);

  path[1] = "state";
  path[2] = "time";
  jsmnf_pair *time = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "position";
  jsmnf_pair *position = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "connected";
  jsmnf_pair *connected = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "ping";
  jsmnf_pair *ping = jsmnf_find_path(pairs, res->body, path, 3);

  if (!time || !position || !connected || !ping) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !time ? "time" : !position ? "position" : !connected ? "connected" : "ping");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  snprintf(playerInfoStruct->state->time, sizeof(playerInfoStruct->state->time), "%.*s", (int)time->v.len, res->body + time->v.pos);
  snprintf(playerInfoStruct->state->position, sizeof(playerInfoStruct->state->position), "%.*s", (int)position->v.len, res->body + position->v.pos);
  snprintf(playerInfoStruct->state->connected, sizeof(playerInfoStruct->state->connected), "%.*s", (int)connected->v.len, res->body + connected->v.pos);
  snprintf(playerInfoStruct->state->ping, sizeof(playerInfoStruct->state->ping), "%.*s", (int)ping->v.len, res->body + ping->v.pos);

  path[1] = "voice";
  path[2] = "token";
  jsmnf_pair *token = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "endpoint";
  jsmnf_pair *endpoint = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "sessionId";
  jsmnf_pair *sessionId = jsmnf_find_path(pairs, res->body, path, 3);

  if (!token || !endpoint || !sessionId) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !token ? "token" : !endpoint ? "endpoint": !sessionId ? "sessionId" : !connected ? "connected" : "ping");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  snprintf(playerInfoStruct->voice->token, sizeof(playerInfoStruct->voice->token), "%.*s", (int)token->v.len, res->body + token->v.pos);
  snprintf(playerInfoStruct->voice->endpoint, sizeof(playerInfoStruct->voice->endpoint), "%.*s", (int)endpoint->v.len, res->body + endpoint->v.pos);
  snprintf(playerInfoStruct->voice->sessionId, sizeof(playerInfoStruct->voice->sessionId), "%.*s", (int)sessionId->v.len, res->body + sessionId->v.pos);

  path[1] = "filters";
  path[2] = "volume";

  jsmnf_pair *volumeFilter = jsmnf_find_path(pairs, res->body, path, 3);
  if (volumeFilter) 
    snprintf(playerInfoStruct->filters->volume, sizeof(playerInfoStruct->filters->volume), "%.*s", (int)volumeFilter->v.len, res->body + volumeFilter->v.pos);

  path[2] = "equalizer";
  jsmnf_pair *equalizer = jsmnf_find_path(pairs, res->body, path, 3);
  if (equalizer) {
    path[3] = "bands";
    jsmnf_pair *bands = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "gain";
    jsmnf_pair *gain = jsmnf_find_path(pairs, res->body, path, 4);

    snprintf(playerInfoStruct->filters->equalizer->bands, sizeof(playerInfoStruct->filters->equalizer->bands), "%.*s", (int)bands->v.len, res->body + bands->v.pos);
    snprintf(playerInfoStruct->filters->equalizer->gain, sizeof(playerInfoStruct->filters->equalizer->gain), "%.*s", (int)gain->v.len, res->body + gain->v.pos);
  }

  path[2] = "karaoke";
  jsmnf_pair *karaoke = jsmnf_find_path(pairs, res->body, path, 3);
  if (karaoke) {
    path[3] = "level";
    jsmnf_pair *level = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "monoLevel";
    jsmnf_pair *monoLevel = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "filterBand";
    jsmnf_pair *filterBand = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "filterWidth";
    jsmnf_pair *filterWidth = jsmnf_find_path(pairs, res->body, path, 4);

    snprintf(playerInfoStruct->filters->karaoke->level, sizeof(playerInfoStruct->filters->karaoke->level), "%.*s", (int)level->v.len, res->body + level->v.pos);
    snprintf(playerInfoStruct->filters->karaoke->monoLevel, sizeof(playerInfoStruct->filters->karaoke->monoLevel), "%.*s", (int)monoLevel->v.len, res->body + monoLevel->v.pos);
    snprintf(playerInfoStruct->filters->karaoke->filterBand, sizeof(playerInfoStruct->filters->karaoke->filterBand), "%.*s", (int)filterBand->v.len, res->body + filterBand->v.pos);
    snprintf(playerInfoStruct->filters->karaoke->filterWidth, sizeof(playerInfoStruct->filters->karaoke->filterWidth), "%.*s", (int)filterWidth->v.len, res->body + filterWidth->v.pos);
  }

  path[2] = "timescale";
  jsmnf_pair *timescale = jsmnf_find_path(pairs, res->body, path, 3);
  if (timescale) {
    path[3] = "speed";
    jsmnf_pair *speed = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "pitch";
    jsmnf_pair *pitch = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "rate";
    jsmnf_pair *rate = jsmnf_find_path(pairs, res->body, path, 4);

    snprintf(playerInfoStruct->filters->timescale->speed, sizeof(playerInfoStruct->filters->timescale->speed), "%.*s", (int)speed->v.len, res->body + speed->v.pos);
    snprintf(playerInfoStruct->filters->timescale->pitch, sizeof(playerInfoStruct->filters->timescale->pitch), "%.*s", (int)pitch->v.len, res->body + pitch->v.pos);
    snprintf(playerInfoStruct->filters->timescale->rate, sizeof(playerInfoStruct->filters->timescale->rate), "%.*s", (int)rate->v.len, res->body + rate->v.pos);
  }

  path[2] = "tremolo";
  jsmnf_pair *tremolo = jsmnf_find_path(pairs, res->body, path, 3);
  if (tremolo) {
    path[3] = "frequency";
    jsmnf_pair *frequency = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "depth";
    jsmnf_pair *depth = jsmnf_find_path(pairs, res->body, path, 4);

    snprintf(playerInfoStruct->filters->tremolo->frequency, sizeof(playerInfoStruct->filters->tremolo->frequency), "%.*s", (int)frequency->v.len, res->body + frequency->v.pos);
    snprintf(playerInfoStruct->filters->tremolo->depth, sizeof(playerInfoStruct->filters->tremolo->depth), "%.*s", (int)depth->v.len, res->body + depth->v.pos);
  }

  path[2] = "vibrato";
  jsmnf_pair *vibrato = jsmnf_find_path(pairs, res->body, path, 3);
  if (vibrato) {
    path[3] = "frequency";
    jsmnf_pair *frequency = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "depth";
    jsmnf_pair *depth = jsmnf_find_path(pairs, res->body, path, 4);

    snprintf(playerInfoStruct->filters->vibrato->frequency, sizeof(playerInfoStruct->filters->vibrato->frequency), "%.*s", (int)frequency->v.len, res->body + frequency->v.pos);
    snprintf(playerInfoStruct->filters->vibrato->depth, sizeof(playerInfoStruct->filters->vibrato->depth), "%.*s", (int)depth->v.len, res->body + depth->v.pos);
  }

  path[2] = "rotation";
  jsmnf_pair *rotation = jsmnf_find_path(pairs, res->body, path, 3);
  if (rotation) {
    path[3] = "rotationHz";
    jsmnf_pair *rotationHz = jsmnf_find_path(pairs, res->body, path, 4);

    snprintf(playerInfoStruct->filters->rotation, sizeof(playerInfoStruct->filters->rotation), "%.*s", (int)rotationHz->v.len, res->body + rotationHz->v.pos);
  }

  path[2] = "distortion";
  jsmnf_pair *distortion = jsmnf_find_path(pairs, res->body, path, 3);
  if (distortion) {
    path[3] = "sinOffset";
    jsmnf_pair *sinOffset = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "sinScale";
    jsmnf_pair *sinScale = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "cosOffset";
    jsmnf_pair *cosOffset = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "cosScale";
    jsmnf_pair *cosScale = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "tanOffset";
    jsmnf_pair *tanOffset = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "tanScale";
    jsmnf_pair *tanScale = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "offset";
    jsmnf_pair *offset = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "scale";
    jsmnf_pair *scale = jsmnf_find_path(pairs, res->body, path, 4);

    snprintf(playerInfoStruct->filters->distortion->sinOffset, sizeof(playerInfoStruct->filters->distortion->sinOffset), "%.*s", (int)sinOffset->v.len, res->body + sinOffset->v.pos);
    snprintf(playerInfoStruct->filters->distortion->sinScale, sizeof(playerInfoStruct->filters->distortion->sinScale), "%.*s", (int)sinScale->v.len, res->body + sinScale->v.pos);
    snprintf(playerInfoStruct->filters->distortion->cosOffset, sizeof(playerInfoStruct->filters->distortion->cosOffset), "%.*s", (int)cosOffset->v.len, res->body + cosOffset->v.pos);
    snprintf(playerInfoStruct->filters->distortion->cosScale, sizeof(playerInfoStruct->filters->distortion->cosScale), "%.*s", (int)cosScale->v.len, res->body + cosScale->v.pos);
    snprintf(playerInfoStruct->filters->distortion->tanOffset, sizeof(playerInfoStruct->filters->distortion->tanOffset), "%.*s", (int)tanOffset->v.len, res->body + tanOffset->v.pos);
    snprintf(playerInfoStruct->filters->distortion->tanScale, sizeof(playerInfoStruct->filters->distortion->tanScale), "%.*s", (int)tanScale->v.len, res->body + tanScale->v.pos);
    snprintf(playerInfoStruct->filters->distortion->offset, sizeof(playerInfoStruct->filters->distortion->offset), "%.*s", (int)offset->v.len, res->body + offset->v.pos);
    snprintf(playerInfoStruct->filters->distortion->scale, sizeof(playerInfoStruct->filters->distortion->scale), "%.*s", (int)scale->v.len, res->body + scale->v.pos);
  }

  path[2] = "channelMix";
  jsmnf_pair *channelMix = jsmnf_find_path(pairs, res->body, path, 3);
  if (channelMix) {
    path[3] = "leftToLeft";
    jsmnf_pair *leftToLeft = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "leftToRight";
    jsmnf_pair *leftToRight = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "rightToLeft";
    jsmnf_pair *rightToLeft = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "rightToRight";
    jsmnf_pair *rightToRight = jsmnf_find_path(pairs, res->body, path, 4);

    snprintf(playerInfoStruct->filters->channelMix->leftToLeft, sizeof(playerInfoStruct->filters->channelMix->leftToLeft), "%.*s", (int)leftToLeft->v.len, res->body + leftToLeft->v.pos);
    snprintf(playerInfoStruct->filters->channelMix->leftToRight, sizeof(playerInfoStruct->filters->channelMix->leftToRight), "%.*s", (int)leftToRight->v.len, res->body + leftToRight->v.pos);
    snprintf(playerInfoStruct->filters->channelMix->rightToLeft, sizeof(playerInfoStruct->filters->channelMix->rightToLeft), "%.*s", (int)rightToLeft->v.len, res->body + rightToLeft->v.pos);
    snprintf(playerInfoStruct->filters->channelMix->rightToRight, sizeof(playerInfoStruct->filters->channelMix->rightToRight), "%.*s", (int)rightToRight->v.len, res->body + rightToRight->v.pos);
  }

  path[2] = "lowPass";
  jsmnf_pair *lowPass = jsmnf_find_path(pairs, res->body, path, 3);
  if (lowPass) {
    path[3] = "smoothing";
    jsmnf_pair *lowPass = jsmnf_find_path(pairs, res->body, path, 4);

    snprintf(playerInfoStruct->filters->lowPass, sizeof(playerInfoStruct->filters->lowPass), "%.*s", (int)lowPass->v.len, res->body + lowPass->v.pos);
  }

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strncpy] Set the value for struct members of playerInfoStruct.");

  return COGLINK_SUCCESS;
}

void coglink_getPlayersCleanup(struct coglink_requestInformation *res) {
  free(res->body);
}

int coglink_playSong(struct coglink_lavaInfo *lavaInfo, char *encodedTrack, u64snowflake guildId) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onPlayRequest[0]) {
    for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
      if (!lavaInfo->plugins->events->onPlayRequest[i]) break;

      int pluginResultCode = lavaInfo->plugins->events->onPlayRequest[i](lavaInfo, encodedTrack, guildId);
      if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
    }
  }

  int node = _coglink_findPlayerNode(guildId);

  char reqPath[64];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->nodes[node].sessionId, guildId);

  char payload[1024];
  int payloadLen = snprintf(payload, sizeof(payload), "{\"encodedTrack\":\"%s\"}", encodedTrack);

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL, 
                                 &(struct __coglink_requestConfig) {
                                   .requestType = __COGLINK_PATCH_REQ,
                                   .path = reqPath,
                                   .pathLength = pathLen,
                                   .body = payload,
                                   .bodySize = payloadLen
                                 });
}

void coglink_destroyPlayer(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId) {
  int node = _coglink_findPlayerNode(guildId);

  char reqPath[64];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->nodes[node].sessionId, guildId);

  _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL, 
                          &(struct __coglink_requestConfig) {
                            .requestType = __COGLINK_DELETE_REQ,
                            .path = reqPath,
                            .pathLength = pathLen
                          });
}

void coglink_stopPlayer(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId) {
  int node = _coglink_findPlayerNode(guildId);

  char reqPath[64];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->nodes[node].sessionId, guildId);

  _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL, 
                          &(struct __coglink_requestConfig) {
                            .requestType = __COGLINK_PATCH_REQ,
                            .path = reqPath,
                            .pathLength = pathLen,
                            .body = "{\"encodedTrack\":\"null\"}",
                            .bodySize = 23
                          });
}

void coglink_pausePlayer(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, char *pause) {
  int node = _coglink_findPlayerNode(guildId);

  char reqPath[64];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->nodes[node].sessionId, guildId);

  char payload[32];
  int payloadLen = snprintf(payload, sizeof(payload), "{\"pause\":%s}", pause);

  _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL, 
                          &(struct __coglink_requestConfig) {
                            .requestType = __COGLINK_PATCH_REQ,
                            .path = reqPath,
                            .pathLength = pathLen,
                            .body = payload,
                            .bodySize = payloadLen
                          });
}

void coglink_seekTrack(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, char *position) {
  int node = _coglink_findPlayerNode(guildId);

  char reqPath[64];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->nodes[node].sessionId, guildId);

  char payload[32];
  int payloadLen = snprintf(payload, sizeof(payload), "{\"position\":%s}", position);

  _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL, 
                          &(struct __coglink_requestConfig) {
                            .requestType = __COGLINK_PATCH_REQ,
                            .path = reqPath,
                            .pathLength = pathLen,
                            .body = payload,
                            .bodySize = payloadLen
                          });
}

void coglink_setPlayerVolume(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, char *volume) {
  int node = _coglink_findPlayerNode(guildId);

  char reqPath[64];
  int pathLen = snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->nodes[node].sessionId, guildId);

  char payload[32];
  int payloadLen = snprintf(payload, sizeof(payload), "{\"volume\":%s}", volume);

  _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL, 
                          &(struct __coglink_requestConfig) {
                            .requestType = __COGLINK_PATCH_REQ,
                            .path = reqPath,
                            .pathLength = pathLen,
                            .body = payload,
                            .bodySize = payloadLen
                          });
}

void coglink_setEffect(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, int effect, char *value) {
  char payload[512 + 128], effectStr[11] = "VOLUME";
  switch (effect) {
    case COGLINK_FILTER_VOLUME: {
      snprintf(effectStr, sizeof(effectStr), "volume");
      break;
    }
    case COGLINK_FILTER_EQUALIZER: {
      snprintf(effectStr, sizeof(effectStr), "equalizer");
      break;
    }
    case COGLINK_FILTER_KARAOKE: {
      snprintf(effectStr, sizeof(effectStr), "karaoke");
      break;
    }
    case COGLINK_FILTER_TIMESCALE: {
      snprintf(effectStr, sizeof(effectStr), "timescale");
      break;
    }
    case COGLINK_FILTER_TREMOLO: {
      snprintf(effectStr, sizeof(effectStr), "tremolo");
      break;
    }
    case COGLINK_FILTER_ROTATION: {
      snprintf(effectStr, sizeof(effectStr), "rotation");
      break;
    }
    case COGLINK_FILTER_DISTORTION: {
      snprintf(effectStr, sizeof(effectStr), "distortion");
      break;
    }
    case COGLINK_FILTER_CHANNELMIX: {
      snprintf(effectStr, sizeof(effectStr), "channelMix");
      break;
    }
    case COGLINK_FILTER_LOWPASS: {
      snprintf(effectStr, sizeof(effectStr), "lowPass");
      break;
    }
    case COGLINK_FILTER_REMOVE: {
      break;
    }
  }

  int node = _coglink_findPlayerNode(guildId);

  char reqPath[64];
  int payloadLen = 0, pathLen = snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->nodes[node].sessionId, guildId);

  if (effect != COGLINK_FILTER_REMOVE) payloadLen = snprintf(payload, sizeof(payload), "{\"filters\":{\"%s\":%s}}", effectStr, value);
  else payloadLen = snprintf(payload, sizeof(payload), "{\"filters\":{}}");

  _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL, 
                          &(struct __coglink_requestConfig) {
                            .requestType = __COGLINK_PATCH_REQ,
                            .path = reqPath,
                            .pathLength = pathLen,
                            .body = payload,
                            .bodySize = payloadLen
                          });
}
