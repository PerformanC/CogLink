#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>
#include <coglink/player.h>

int coglink_getPlayers(struct lavaInfo *lavaInfo, struct requestInformation *res) {
  char reqPath[35];
  snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players", lavaInfo->sessionId);

  return __coglink_performRequest(lavaInfo, __COGLINK_GET_REQ, 0, 0, reqPath, sizeof(reqPath), 1, NULL, 0, res, 1, NULL);
}

int coglink_parseGetPlayers(struct lavaInfo *lavaInfo, struct requestInformation *res, char *pos, struct playerInfo **playerInfoStruct) {
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
  if (!guildId) {
    jsmnf_pair *status = jsmnf_find(pairs, res->body, "status", 6);
    if (__coglink_checkParse(lavaInfo, status, "status") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

    jsmnf_pair *message = jsmnf_find(pairs, res->body, "message", 7);
    if (__coglink_checkParse(lavaInfo, message, "message") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

    char Status[8]; char Message[512];

    snprintf(Status, sizeof(Status), "%.*s", (int)status->v.len, res->body + status->v.pos);
    snprintf(Message, sizeof(Message), "%.*s", (int)message->v.len, res->body + message->v.pos);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Error while trying to get players.\n> status: %s\n> message: %s", Status, Message);
 
    *playerInfoStruct = malloc(sizeof(struct playerInfo));

    (*playerInfoStruct)->error = malloc(sizeof(struct errorStruct));

    (*playerInfoStruct)->error->status = malloc(sizeof(Status));
    (*playerInfoStruct)->error->message = malloc(sizeof(Message));

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-malloc] Allocated %d bytes for song structure.", sizeof(struct playerInfo) + sizeof(struct errorStruct) + sizeof(Status) + sizeof(Message));
 
    strlcpy((*playerInfoStruct)->error->status, Status, 8);
    strlcpy((*playerInfoStruct)->error->message, Message, 512);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:memory-strlcpy] Copied 520 bytes to song structure.");

    return COGLINK_ERROR;
  }
  
  char GuildId[GUILD_ID_LENGTH];
  snprintf(GuildId, sizeof(GuildId), "%.*s", (int)guildId->v.len, res->body + guildId->v.pos);

  *playerInfoStruct = malloc(sizeof(struct playerInfo));

  (*playerInfoStruct)->guildId = malloc(sizeof(GuildId));

  path[1] = "track";
  path[2] = "encoded";
  jsmnf_pair *track = jsmnf_find_path(pairs, res->body, path, 2);

  if (track) {
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

    path[3] = "sourceName";
    jsmnf_pair *sourceName = jsmnf_find_path(pairs, res->body, path, 4);

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

    *playerInfoStruct = malloc(sizeof(struct playerInfo));

    (*playerInfoStruct)->track = malloc(sizeof(struct playerInfoTrack));
    
    (*playerInfoStruct)->track->encoded = malloc(sizeof(Track));

    (*playerInfoStruct)->track->info = malloc(sizeof(struct parsedTrack));

    (*playerInfoStruct)->track->info->identifier = malloc(sizeof(Identifier));
    (*playerInfoStruct)->track->info->isSeekable = malloc(sizeof(IsSeekable));
    (*playerInfoStruct)->track->info->author = malloc(sizeof(Author));
    (*playerInfoStruct)->track->info->length = malloc(sizeof(Length));
    (*playerInfoStruct)->track->info->isStream = malloc(sizeof(IsStream));
    (*playerInfoStruct)->track->info->position = malloc(sizeof(Position));
    (*playerInfoStruct)->track->info->title = malloc(sizeof(Title));
    (*playerInfoStruct)->track->info->uri = malloc(sizeof(Uri));
    (*playerInfoStruct)->track->info->sourceName = malloc(sizeof(SourceName));

    strlcpy((*playerInfoStruct)->track->encoded, Track, TRACK_LENGTH);
    strlcpy((*playerInfoStruct)->track->info->identifier, Identifier, IDENTIFIER_LENGTH);
    strlcpy((*playerInfoStruct)->track->info->isSeekable, IsSeekable, TRUE_FALSE_LENGTH);
    strlcpy((*playerInfoStruct)->track->info->author, Author, AUTHOR_NAME_LENGTH);
    strlcpy((*playerInfoStruct)->track->info->length, Length, VIDEO_LENGTH);
    strlcpy((*playerInfoStruct)->track->info->isStream, IsStream, TRUE_FALSE_LENGTH);
    strlcpy((*playerInfoStruct)->track->info->position, Position, VIDEO_LENGTH);
    strlcpy((*playerInfoStruct)->track->info->title, Title, TRACK_TITLE_LENGTH);
    strlcpy((*playerInfoStruct)->track->info->uri, Uri, URL_LENGTH);
    strlcpy((*playerInfoStruct)->track->info->sourceName, SourceName, SOURCENAME_LENGTH);
  }

  path[1] = "volume";
  jsmnf_pair *volume = jsmnf_find_path(pairs, res->body, path, 2);
  if (__coglink_checkParse(lavaInfo, volume, "volume") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "paused";
  jsmnf_pair *paused = jsmnf_find_path(pairs, res->body, path, 2);
  if (__coglink_checkParse(lavaInfo, paused, "paused") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND; 

  char Volume[VOLUME_LENGTH], Paused[TRUE_FALSE_LENGTH];

  snprintf(Volume, sizeof(Volume), "%.*s", (int)volume->v.len, res->body + volume->v.pos);
  snprintf(Paused, sizeof(Paused), "%.*s", (int)paused->v.len, res->body + paused->v.pos);

  (*playerInfoStruct)->volume = malloc(sizeof(Volume));
  (*playerInfoStruct)->paused = malloc(sizeof(Paused));

  strlcpy((*playerInfoStruct)->volume, Volume, VOLUME_LENGTH);
  strlcpy((*playerInfoStruct)->paused, Paused, TRUE_FALSE_LENGTH);

  path[1] = "voice";
  path[2] = "token";
  jsmnf_pair *token = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "endpoint";
  jsmnf_pair *endpoint = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "sessionId";
  jsmnf_pair *sessionId = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "connected";
  jsmnf_pair *connected = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "ping";
  jsmnf_pair *ping = jsmnf_find_path(pairs, res->body, path, 3);

  if (!token || !endpoint || !sessionId || !connected || !ping) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !token ? "token" : !endpoint ? "endpoint": !sessionId ? "sessionId" : !connected ? "connected" : !ping ? "ping" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  char Token[TOKEN_LENGTH], Endpoint[ENDPOINT_LENGTH], SessionId[SESSIONID_LENGTH], Connected[TRUE_FALSE_LENGTH], Ping[PING_LENGTH];

  snprintf(Token, sizeof(Token), "%.*s", (int)token->v.len, res->body + token->v.pos);
  snprintf(Endpoint, sizeof(Endpoint), "%.*s", (int)endpoint->v.len, res->body + endpoint->v.pos);
  snprintf(SessionId, sizeof(SessionId), "%.*s", (int)sessionId->v.len, res->body + sessionId->v.pos);
  snprintf(Connected, sizeof(Connected), "%.*s", (int)connected->v.len, res->body + connected->v.pos);
  snprintf(Ping, sizeof(Ping), "%.*s", (int)ping->v.len, res->body + ping->v.pos);

  (*playerInfoStruct)->voice = malloc(sizeof(struct playerInfoVoice));

  (*playerInfoStruct)->voice->token = malloc(sizeof(Token));
  (*playerInfoStruct)->voice->endpoint = malloc(sizeof(Endpoint));
  (*playerInfoStruct)->voice->sessionId = malloc(sizeof(SessionId));
  (*playerInfoStruct)->voice->connected = malloc(sizeof(Connected));
  (*playerInfoStruct)->voice->ping = malloc(sizeof(Ping));

  strlcpy((*playerInfoStruct)->voice->token, Token, TOKEN_LENGTH);
  strlcpy((*playerInfoStruct)->voice->endpoint, Endpoint, ENDPOINT_LENGTH);
  strlcpy((*playerInfoStruct)->voice->sessionId, SessionId, SESSIONID_LENGTH);
  strlcpy((*playerInfoStruct)->voice->connected, Connected, TRUE_FALSE_LENGTH);
  strlcpy((*playerInfoStruct)->voice->ping, Ping, PING_LENGTH);

  _Bool malloced = false;

  path[1] = "filters";
  path[2] = "volume";

  jsmnf_pair *volumeFilter = jsmnf_find_path(pairs, res->body, path, 3);
  if (volumeFilter) {
    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));

    char VolumeFilter[VOLUME_LENGTH];

    snprintf(VolumeFilter, sizeof(VolumeFilter), "%.*s", (int)volumeFilter->v.len, res->body + volumeFilter->v.pos);

    (*playerInfoStruct)->filters->volume = malloc(sizeof(VolumeFilter));

    strlcpy((*playerInfoStruct)->filters->volume, VolumeFilter, VOLUME_LENGTH);

    malloced = true;
  }

  path[2] = "equalizer";
  jsmnf_pair *equalizer = jsmnf_find_path(pairs, res->body, path, 3);
  if (equalizer) {
    path[3] = "bands";
    jsmnf_pair *bands = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "gain";
    jsmnf_pair *gain = jsmnf_find_path(pairs, res->body, path, 4);

    char Bands[512], Gain[128];

    snprintf(Bands, sizeof(Bands), "%.*s", (int)bands->v.len, res->body + bands->v.pos);
    snprintf(Gain, sizeof(Gain), "%.*s", (int)gain->v.len, res->body + gain->v.pos);

    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));
    (*playerInfoStruct)->filters->equalizer = malloc(sizeof(struct equalizerStruct));

    (*playerInfoStruct)->filters->equalizer->bands = malloc(sizeof(Bands));
    (*playerInfoStruct)->filters->equalizer->gain = malloc(sizeof(Gain));

    strlcpy((*playerInfoStruct)->filters->equalizer->bands, Bands, 512);
    strlcpy((*playerInfoStruct)->filters->equalizer->gain, Gain, 128);

    malloced = true;
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

    char Level[16], MonoLevel[16], FilterBand[16], FilterWidth[16];

    snprintf(Level, sizeof(Level), "%.*s", (int)level->v.len, res->body + level->v.pos);
    snprintf(MonoLevel, sizeof(MonoLevel), "%.*s", (int)monoLevel->v.len, res->body + monoLevel->v.pos);
    snprintf(FilterBand, sizeof(FilterBand), "%.*s", (int)filterBand->v.len, res->body + filterBand->v.pos);
    snprintf(FilterWidth, sizeof(FilterWidth), "%.*s", (int)filterWidth->v.len, res->body + filterWidth->v.pos);

    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));
    (*playerInfoStruct)->filters->karaoke = malloc(sizeof(struct karaokeStruct));

    (*playerInfoStruct)->filters->karaoke->level = malloc(sizeof(Level));
    (*playerInfoStruct)->filters->karaoke->monoLevel = malloc(sizeof(MonoLevel));
    (*playerInfoStruct)->filters->karaoke->filterBand = malloc(sizeof(FilterBand));
    (*playerInfoStruct)->filters->karaoke->filterWidth = malloc(sizeof(FilterWidth));

    strlcpy((*playerInfoStruct)->filters->karaoke->level, Level, 16);
    strlcpy((*playerInfoStruct)->filters->karaoke->monoLevel, MonoLevel, 16);
    strlcpy((*playerInfoStruct)->filters->karaoke->filterBand, FilterBand, 16);
    strlcpy((*playerInfoStruct)->filters->karaoke->filterWidth, FilterWidth, 16);

    malloced = true;
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

    char Speed[8], Pitch[8], Rate[8];

    snprintf(Speed, sizeof(Speed), "%.*s", (int)speed->v.len, res->body + speed->v.pos);
    snprintf(Pitch, sizeof(Pitch), "%.*s", (int)pitch->v.len, res->body + pitch->v.pos);
    snprintf(Rate, sizeof(Rate), "%.*s", (int)rate->v.len, res->body + rate->v.pos);

    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));
    (*playerInfoStruct)->filters->timescale = malloc(sizeof(struct timescaleStruct));

    (*playerInfoStruct)->filters->timescale->speed = malloc(sizeof(Speed));
    (*playerInfoStruct)->filters->timescale->pitch = malloc(sizeof(Pitch));
    (*playerInfoStruct)->filters->timescale->rate = malloc(sizeof(Rate));

    strlcpy((*playerInfoStruct)->filters->timescale->speed, Speed, 8);
    strlcpy((*playerInfoStruct)->filters->timescale->pitch, Pitch, 8);
    strlcpy((*playerInfoStruct)->filters->timescale->rate, Rate, 8);

    malloced = true;
  }

  path[2] = "tremolo";
  jsmnf_pair *tremolo = jsmnf_find_path(pairs, res->body, path, 3);
  if (tremolo) {
    path[3] = "frequency";
    jsmnf_pair *frequency = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "depth";
    jsmnf_pair *depth = jsmnf_find_path(pairs, res->body, path, 4);

    char Frequency[8], Depth[4];

    snprintf(Frequency, sizeof(Frequency), "%.*s", (int)frequency->v.len, res->body + frequency->v.pos);
    snprintf(Depth, sizeof(Depth), "%.*s", (int)depth->v.len, res->body + depth->v.pos);

    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));
    (*playerInfoStruct)->filters->tremolo = malloc(sizeof(struct frequencyDepthStruct));

    (*playerInfoStruct)->filters->tremolo->frequency = malloc(sizeof(Frequency));
    (*playerInfoStruct)->filters->tremolo->depth = malloc(sizeof(Depth));

    strlcpy((*playerInfoStruct)->filters->tremolo->frequency, Frequency, 8);
    strlcpy((*playerInfoStruct)->filters->tremolo->depth, Depth, 4);

    malloced = true;
  }

  path[2] = "vibrato";
  jsmnf_pair *vibrato = jsmnf_find_path(pairs, res->body, path, 3);
  if (vibrato) {
    path[3] = "frequency";
    jsmnf_pair *frequency = jsmnf_find_path(pairs, res->body, path, 4);

    path[3] = "depth";
    jsmnf_pair *depth = jsmnf_find_path(pairs, res->body, path, 4);

    char Frequency[4], Depth[4];

    snprintf(Frequency, sizeof(Frequency), "%.*s", (int)frequency->v.len, res->body + frequency->v.pos);
    snprintf(Depth, sizeof(Depth), "%.*s", (int)depth->v.len, res->body + depth->v.pos);

    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));
    (*playerInfoStruct)->filters->vibrato = malloc(sizeof(struct frequencyDepthStruct));

    (*playerInfoStruct)->filters->vibrato->frequency = malloc(sizeof(Frequency));
    (*playerInfoStruct)->filters->vibrato->depth = malloc(sizeof(Depth));

    strlcpy((*playerInfoStruct)->filters->vibrato->frequency, Frequency, 4);
    strlcpy((*playerInfoStruct)->filters->vibrato->depth, Depth, 4);

    malloced = true;
  }

  path[2] = "rotation";
  jsmnf_pair *rotation = jsmnf_find_path(pairs, res->body, path, 3);
  if (rotation) {
    path[3] = "rotationHz";
    jsmnf_pair *rotationHz = jsmnf_find_path(pairs, res->body, path, 4);

    char RotationHz[8];

    snprintf(RotationHz, sizeof(RotationHz), "%.*s", (int)rotationHz->v.len, res->body + rotationHz->v.pos);

    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));

    (*playerInfoStruct)->filters->rotation = malloc(sizeof(RotationHz));

    strlcpy((*playerInfoStruct)->filters->rotation, RotationHz, 8);

    malloced = true;
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

    char SinOffset[8], SinScale[8], CosOffset[8], CosScale[8], TanOffset[8], TanScale[8], Offset[8], Scale[8];

    snprintf(SinOffset, sizeof(SinOffset), "%.*s", (int)sinOffset->v.len, res->body + sinOffset->v.pos);
    snprintf(SinScale, sizeof(SinScale), "%.*s", (int)sinScale->v.len, res->body + sinScale->v.pos);
    snprintf(CosOffset, sizeof(CosOffset), "%.*s", (int)cosOffset->v.len, res->body + cosOffset->v.pos);
    snprintf(CosScale, sizeof(CosScale), "%.*s", (int)cosScale->v.len, res->body + cosScale->v.pos);
    snprintf(TanOffset, sizeof(TanOffset), "%.*s", (int)tanOffset->v.len, res->body + tanOffset->v.pos);
    snprintf(TanScale, sizeof(TanScale), "%.*s", (int)tanScale->v.len, res->body + tanScale->v.pos);
    snprintf(Offset, sizeof(Offset), "%.*s", (int)offset->v.len, res->body + offset->v.pos);
    snprintf(Scale, sizeof(Scale), "%.*s", (int)scale->v.len, res->body + scale->v.pos);

    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));
    (*playerInfoStruct)->filters->distortion = malloc(sizeof(struct distortionStruct));

    (*playerInfoStruct)->filters->distortion->sinOffset = malloc(sizeof(SinOffset));
    (*playerInfoStruct)->filters->distortion->sinScale = malloc(sizeof(SinScale));
    (*playerInfoStruct)->filters->distortion->cosOffset = malloc(sizeof(CosOffset));
    (*playerInfoStruct)->filters->distortion->cosScale = malloc(sizeof(CosScale));
    (*playerInfoStruct)->filters->distortion->tanOffset = malloc(sizeof(TanOffset));
    (*playerInfoStruct)->filters->distortion->tanScale = malloc(sizeof(TanScale));
    (*playerInfoStruct)->filters->distortion->offset = malloc(sizeof(Offset));
    (*playerInfoStruct)->filters->distortion->scale = malloc(sizeof(Scale));

    strlcpy((*playerInfoStruct)->filters->distortion->sinOffset, SinOffset, 8);
    strlcpy((*playerInfoStruct)->filters->distortion->sinScale, SinScale, 8);
    strlcpy((*playerInfoStruct)->filters->distortion->cosOffset, CosOffset, 8);
    strlcpy((*playerInfoStruct)->filters->distortion->cosScale, CosScale, 8);
    strlcpy((*playerInfoStruct)->filters->distortion->tanOffset, TanOffset, 8);
    strlcpy((*playerInfoStruct)->filters->distortion->tanScale, TanScale, 8);
    strlcpy((*playerInfoStruct)->filters->distortion->offset, Offset, 8);
    strlcpy((*playerInfoStruct)->filters->distortion->scale, Scale, 8);

    malloced = true;
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

    char LeftToLeft[4], LeftToRight[4], RightToLeft[4], RightToRight[4];

    snprintf(LeftToLeft, sizeof(LeftToLeft), "%.*s", (int)leftToLeft->v.len, res->body + leftToLeft->v.pos);
    snprintf(LeftToRight, sizeof(LeftToRight), "%.*s", (int)leftToRight->v.len, res->body + leftToRight->v.pos);
    snprintf(RightToLeft, sizeof(RightToLeft), "%.*s", (int)rightToLeft->v.len, res->body + rightToLeft->v.pos);
    snprintf(RightToRight, sizeof(RightToRight), "%.*s", (int)rightToRight->v.len, res->body + rightToRight->v.pos);

    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));
    (*playerInfoStruct)->filters->channelMix = malloc(sizeof(struct channelMixStruct));

    (*playerInfoStruct)->filters->channelMix->leftToLeft = malloc(sizeof(LeftToLeft));
    (*playerInfoStruct)->filters->channelMix->leftToRight = malloc(sizeof(LeftToRight));
    (*playerInfoStruct)->filters->channelMix->rightToLeft = malloc(sizeof(RightToLeft));
    (*playerInfoStruct)->filters->channelMix->rightToRight = malloc(sizeof(RightToRight));

    strlcpy((*playerInfoStruct)->filters->channelMix->leftToLeft, LeftToLeft, 4);
    strlcpy((*playerInfoStruct)->filters->channelMix->leftToRight, LeftToRight, 4);
    strlcpy((*playerInfoStruct)->filters->channelMix->rightToLeft, RightToLeft, 4);
    strlcpy((*playerInfoStruct)->filters->channelMix->rightToRight, RightToRight, 4);

    malloced = true;
  }

  path[2] = "lowPass";
  jsmnf_pair *lowPass = jsmnf_find_path(pairs, res->body, path, 3);
  if (lowPass) {
    path[3] = "smoothing";
    jsmnf_pair *smoothing = jsmnf_find_path(pairs, res->body, path, 4);

    char Smoothing[8];

    snprintf(Smoothing, sizeof(Smoothing), "%.*s", (int)smoothing->v.len, res->body + smoothing->v.pos);

    if (!malloced) (*playerInfoStruct)->filters = malloc(sizeof(struct playerInfoFilters));

    (*playerInfoStruct)->filters->lowPass = malloc(sizeof(Smoothing));

    strlcpy((*playerInfoStruct)->filters->lowPass, Smoothing, 8);
  }

  return COGLINK_SUCCESS;
}

int coglink_playSong(struct lavaInfo *lavaInfo, char *track, u64snowflake guildId) {
  if (lavaInfo->plugins && lavaInfo->plugins->events->onPlayRequest[0]) {
    if (lavaInfo->plugins->security->allowReadIOPoller) {
      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onPlayRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onPlayRequest[i](lavaInfo, track, guildId);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    } else {
      struct lavaInfo *lavaInfoPlugin = lavaInfo;
      lavaInfoPlugin->io_poller = NULL;

      for (int i = 0;i <+ lavaInfo->plugins->amount;i++) {
        if (!lavaInfo->plugins->events->onPlayRequest[i]) break;

        int pluginResultCode = lavaInfo->plugins->events->onPlayRequest[i](lavaInfoPlugin, track, guildId);
        if (pluginResultCode != COGLINK_PROCEED) return pluginResultCode;
      }
    }
  }

  char reqPath[64];
  snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->sessionId, guildId);

  char payload[1024];
  snprintf(payload, sizeof(payload), "{\"encodedTrack\":\"%s\",\"noReplace\":false,\"pause\":false}", track);

   return __coglink_performRequest(lavaInfo, __COGLINK_PATCH_REQ, 0, 0, reqPath, sizeof(reqPath), 1, payload, sizeof(payload), NULL, 0, NULL);
}

void coglink_destroyPlayer(struct lavaInfo *lavaInfo, u64snowflake guildId) {
  char reqPath[64];
  snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->sessionId, guildId);

  __coglink_performRequest(lavaInfo, __COGLINK_DELETE_REQ, 0, 0, reqPath, sizeof(reqPath), 1, NULL, 0, NULL, 0, NULL);
}

void coglink_stopPlayer(struct lavaInfo *lavaInfo, u64snowflake guildId) {
  char reqPath[64];
  snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->sessionId, guildId);

  __coglink_performRequest(lavaInfo, __COGLINK_PATCH_REQ, 0, 0, reqPath, sizeof(reqPath), 1, "{\"encodedTrack\":\"null\"}", 32, NULL, 0, NULL);
}

void coglink_pausePlayer(struct lavaInfo *lavaInfo, u64snowflake guildId, char *pause) {
  char reqPath[64];
  snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->sessionId, guildId);

  char payload[32];
  snprintf(payload, sizeof(payload), "{\"pause\":%s}", pause);

  __coglink_performRequest(lavaInfo, __COGLINK_PATCH_REQ, 0, 0, reqPath, sizeof(reqPath), 1, payload, sizeof(payload), NULL, 0, NULL);
}

void coglink_seekTrack(struct lavaInfo *lavaInfo, u64snowflake guildId, char *position) {
  char reqPath[64];
  snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->sessionId, guildId);

  char payload[32];
  snprintf(payload, sizeof(payload), "{\"position\":%s}", position);

  __coglink_performRequest(lavaInfo, __COGLINK_PATCH_REQ, 0, 0, reqPath, sizeof(reqPath), 1, payload, sizeof(payload), NULL, 0, NULL);
}

void coglink_setPlayerVolume(struct lavaInfo *lavaInfo, u64snowflake guildId, char *volume) {
  char reqPath[64];
  snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->sessionId, guildId);

  char payload[32];
  snprintf(payload, sizeof(payload), "{\"volume\":%s}", volume);

  __coglink_performRequest(lavaInfo, __COGLINK_PATCH_REQ, 0, 0, reqPath, sizeof(reqPath), 1, payload, sizeof(payload), NULL, 0, NULL);
}

void coglink_setEffect(struct lavaInfo *lavaInfo, u64snowflake guildId, int effect, char *value) {
  char payload[strnlen(value, 512) + 128];
  char effectStr[11] = "VOLUME";
  switch (effect) {
    case FILTER_VOLUME: {
      snprintf(effectStr, sizeof(effectStr), "volume");
      break;
    }
    case FILTER_EQUALIZER: {
      snprintf(effectStr, sizeof(effectStr), "equalizer");
      break;
    }
    case FILTER_KARAOKE: {
      snprintf(effectStr, sizeof(effectStr), "karaoke");
      break;
    }
    case FILTER_TIMESCALE: {
      snprintf(effectStr, sizeof(effectStr), "timescale");
      break;
    }
    case FILTER_TREMOLO: {
      snprintf(effectStr, sizeof(effectStr), "tremolo");
      break;
    }
    case FILTER_ROTATION: {
      snprintf(effectStr, sizeof(effectStr), "rotation");
      break;
    }
    case FILTER_DISTORTION: {
      snprintf(effectStr, sizeof(effectStr), "distortion");
      break;
    }
    case FILTER_CHANNELMIX: {
      snprintf(effectStr, sizeof(effectStr), "channelMix");
      break;
    }
    case FILTER_LOWPASS: {
      snprintf(effectStr, sizeof(effectStr), "lowPass");
      break;
    }
    case FILTER_REMOVE: {
      break;
    }
  }

  char reqPath[64];
  snprintf(reqPath, sizeof(reqPath), "/sessions/%s/players/%"PRIu64"", lavaInfo->sessionId, guildId);

  if (effect != FILTER_REMOVE) snprintf(payload, sizeof(payload), "{\"filters\":{\"%s\":%s}}", effectStr, value);
  else snprintf(payload, sizeof(payload), "{\"filters\":{}}");

  printf("%s\n", payload);

  __coglink_performRequest(lavaInfo, __COGLINK_PATCH_REQ, 0, 0, reqPath, sizeof(reqPath), 1, payload, sizeof(payload), NULL, 0, NULL);
}

void coglink_parseGetPlayersCleanup(const struct lavaInfo *lavaInfo, struct playerInfo *playerInfoStruct) {
  if (playerInfoStruct->error) {
    free(playerInfoStruct->error->status);
    free(playerInfoStruct->error->message);
    free(playerInfoStruct->error);
  }
  if (playerInfoStruct->track) {
    free(playerInfoStruct->track->encoded);
    free(playerInfoStruct->track->info->identifier);
    free(playerInfoStruct->track->info->isSeekable);
    free(playerInfoStruct->track->info->author);
    free(playerInfoStruct->track->info->length);
    free(playerInfoStruct->track->info->isStream);
    free(playerInfoStruct->track->info->position);
    free(playerInfoStruct->track->info->title);
    free(playerInfoStruct->track->info->uri);
    free(playerInfoStruct->track->info->sourceName);
    free(playerInfoStruct->track->info);
    free(playerInfoStruct->track);
  }
  if (playerInfoStruct->volume) free(playerInfoStruct->volume);
  if (playerInfoStruct->paused) free(playerInfoStruct->paused);
  if (playerInfoStruct->voice) {
    free(playerInfoStruct->voice->token);
    free(playerInfoStruct->voice->endpoint);
    free(playerInfoStruct->voice->sessionId);
    free(playerInfoStruct->voice->connected);
    free(playerInfoStruct->voice->ping);
    free(playerInfoStruct->voice);
  }
  if (playerInfoStruct->filters) {
    if (playerInfoStruct->filters->volume) free(playerInfoStruct->filters->volume);
    if (playerInfoStruct->filters->equalizer) {
      free(playerInfoStruct->filters->equalizer->bands);
      free(playerInfoStruct->filters->equalizer->gain);
      free(playerInfoStruct->filters->equalizer);
    }
    if (playerInfoStruct->filters->karaoke) {
      free(playerInfoStruct->filters->karaoke->level);
      free(playerInfoStruct->filters->karaoke->monoLevel);
      free(playerInfoStruct->filters->karaoke->filterBand);
      free(playerInfoStruct->filters->karaoke->filterWidth);
      free(playerInfoStruct->filters->karaoke);
    }
    if (playerInfoStruct->filters->timescale) {
      free(playerInfoStruct->filters->timescale->speed);
      free(playerInfoStruct->filters->timescale->pitch);
      free(playerInfoStruct->filters->timescale->rate);
      free(playerInfoStruct->filters->timescale);
    }
    if (playerInfoStruct->filters->tremolo) {
      free(playerInfoStruct->filters->tremolo->frequency);
      free(playerInfoStruct->filters->tremolo->depth);
      free(playerInfoStruct->filters->tremolo);
    }
    if (playerInfoStruct->filters->rotation) free(playerInfoStruct->filters->rotation);
    if (playerInfoStruct->filters->distortion) {
      free(playerInfoStruct->filters->distortion->sinOffset);
      free(playerInfoStruct->filters->distortion->sinScale);
      free(playerInfoStruct->filters->distortion->cosOffset);
      free(playerInfoStruct->filters->distortion->cosScale);
      free(playerInfoStruct->filters->distortion->tanOffset);
      free(playerInfoStruct->filters->distortion->tanScale);
      free(playerInfoStruct->filters->distortion->offset);
      free(playerInfoStruct->filters->distortion->scale);
      free(playerInfoStruct->filters->distortion);
    }
    if (playerInfoStruct->filters->channelMix) {
      free(playerInfoStruct->filters->channelMix->leftToLeft);
      free(playerInfoStruct->filters->channelMix->leftToRight);
      free(playerInfoStruct->filters->channelMix->rightToLeft);
      free(playerInfoStruct->filters->channelMix->rightToRight);
      free(playerInfoStruct->filters->channelMix);
    }
    if (playerInfoStruct->filters->lowPass) free(playerInfoStruct->filters->lowPass);
    free(playerInfoStruct->filters);
  }
  free(playerInfoStruct);

  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->memoryDebugging) log_debug("[coglink:jsmn-find] Cleaned up the players info struct.");
}
