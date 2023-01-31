/** \file
 * File containing the Lavalink player related functions.
 */

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
  struct parsedTrack *info;
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

/**
 * Retrieves the players.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_getPlayers(struct lavaInfo *lavaInfo, struct requestInformation *res);

/**
 * Parses the response body of the function coglink_getPlayers.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param pos Position of the player that will be parsed
 * @param playerInfoStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseGetPlayers(struct lavaInfo *lavaInfo, struct requestInformation *res, char *pos, struct playerInfo **playerInfoStruct);

/**
 * Frees the allocations generated while performing the function coglink_getPlayers.
 * @param res Structure with the information of the request.
 */
void coglink_getPlayersCleanup(struct requestInformation *res);

/**
 * Starts playing a track in the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param track Track that will be played.
 * @param guildId ID of the guild that the track will be played.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_playSong(struct lavaInfo *lavaInfo, char *track, u64snowflake guildId);

/**
 * Destroys the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 */
void coglink_destroyPlayer(struct lavaInfo *lavaInfo, u64snowflake guildId);

/**
 * Stops the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 */
void coglink_stopPlayer(struct lavaInfo *lavaInfo, u64snowflake guildId);

/**
 * Pauses the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 * @param pause String that can be true or false to whatever pause or not.
 */
void coglink_pausePlayer(struct lavaInfo *lavaInfo, u64snowflake guildId, char *pause);

/**
 * Makes the track go back to play at x ms.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 * @param position String number of the time in ms.
 */
void coglink_seekTrack(struct lavaInfo *lavaInfo, u64snowflake guildId, char *position);

/**
 * Changes the volume of the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 * @param volume String containing the volume that will be set.
 */
void coglink_setPlayerVolume(struct lavaInfo *lavaInfo, u64snowflake guildId, char *volume);

/**
 * Sets a effect to the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 * @param effect Type of the effect that will be set.
 * @param value Value of the effect.
 */
void coglink_setEffect(struct lavaInfo *lavaInfo, u64snowflake guildId, int effect, char *value);

#endif
