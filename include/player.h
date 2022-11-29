#ifndef PLAYER_H
#define PLAYER_H

struct equalizerStruct {
  char *bands;
  char *gain;
};

struct karaokeStruct {
  char *level;
  char *monoLevel;
  char *filterBand;
  char *filterWidth;
};

struct timescaleStruct {
  char *speed;
  char *pitch;
  char *rate;
};

struct frequencyDepthStruct {
  char *frequency;
  char *depth;
};

struct distortionStruct {
  char *sinOffset;
  char *sinScale;
  char *cosOffset;
  char *cosScale;
  char *tanOffset;
  char *tanScale;
  char *offset;
  char *scale;
};

struct channelMixStruct {
  char *leftToLeft;
  char *leftToRight;
  char *rightToLeft;
  char *rightToRight;
};

struct playerInfoTrack {
  char *encoded;
  struct lavaParsedTrack *info;
};

struct playerInfoVoice {
  char *token;
  char *endpoint;
  char *sessionId;
  char *connected;
  char *ping;
};

struct playerInfoFilters {
  char *volume;
  struct equalizerStruct *equalizer;
  struct karaokeStruct *karaoke;
  struct timescaleStruct *timescale;
  struct frequencyDepthStruct *tremolo;
  struct frequencyDepthStruct *vibrato;
  char *rotation;
  struct distortionStruct *distortion;
  struct channelMixStruct *channelMix;
  char *lowPass;
};

struct errorStruct {
  char *status;
  char *message;
};

struct playerInfo {
  char *guildId;
  struct playerInfoTrack *track;
  char *volume;
  char *paused;
  struct playerInfoVoice *voice;
  struct playerInfoFilters *filters;
  struct errorStruct *error;
};

int coglink_getPlayers(struct lavaInfo *lavaInfo, struct httpRequest *res);

int coglink_parseGetPlayers(struct lavaInfo *lavaInfo, struct httpRequest *res, struct playerInfo **playerInfoStruct);

int coglink_playSong(struct lavaInfo *lavaInfo, char *track, u64snowflake guildId);

void coglink_destroyPlayer(struct lavaInfo *lavaInfo, u64snowflake guildId);

void coglink_stopPlayer(struct lavaInfo *lavaInfo, u64snowflake guildId);

void coglink_pausePlayer(struct lavaInfo *lavaInfo, u64snowflake guildId, char *pause);

void coglink_seekTrack(struct lavaInfo *lavaInfo, u64snowflake guildId, char *position);

void coglink_setPlayerVolume(struct lavaInfo *lavaInfo, u64snowflake guildId, char *volume);

void coglink_setEffect(struct lavaInfo *lavaInfo, u64snowflake guildId, int effect, char *value);

#endif
