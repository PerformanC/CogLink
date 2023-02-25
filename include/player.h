/** \file
 * File containing the Lavalink player related functions.
 */

#ifndef PLAYER_H
#define PLAYER_H

struct equalizerStruct {
  char bands[COGLINK_BANDS_LENGTH];
  char gain[COGLINK_GAIN_LENGTH];
};

struct karaokeStruct {
  char level[16];
  char monoLevel[16];
  char filterBand[16];
  char filterWidth[16];
};

struct timescaleStruct {
  char speed[8];
  char pitch[8];
  char rate[8];
};

struct frequencyDepthStruct {
  char frequency[8];
  char depth[4];
};

struct distortionStruct {
  char sinOffset[8];
  char sinScale[8];
  char cosOffset[8];
  char cosScale[8];
  char tanOffset[8];
  char tanScale[8];
  char offset[8];
  char scale[8];
};

struct channelMixStruct {
  char leftToLeft[4];
  char leftToRight[4];
  char rightToLeft[4];
  char rightToRight[4];
};

struct playerInfoTrack {
  char encoded[COGLINK_TRACK_LENGTH];
  struct parsedTrack *info;
};

struct playerInfoVoice {
  char token[COGLINK_TOKEN_LENGTH];
  char endpoint[COGLINK_ENDPOINT_LENGTH];
  char sessionId[COGLINK_SESSIONID_LENGTH];
  char connected[COGLINK_TRUE_FALSE_LENGTH];
  char ping[COGLINK_PING_LENGTH];
};

struct playerInfoFilters {
  char volume[COGLINK_VOLUME_LENGTH];
  struct equalizerStruct *equalizer;
  struct karaokeStruct *karaoke;
  struct timescaleStruct *timescale;
  struct frequencyDepthStruct *tremolo;
  struct frequencyDepthStruct *vibrato;
  char rotation[8];
  struct distortionStruct *distortion;
  struct channelMixStruct *channelMix;
  char lowPass[8];
};

struct playerInfo {
  char guildId[COGLINK_GUILD_ID_LENGTH];
  struct playerInfoTrack *track;
  char volume[COGLINK_VOLUME_LENGTH];
  char paused[COGLINK_TRUE_FALSE_LENGTH];
  struct playerInfoVoice *voice;
  struct playerInfoFilters *filters;
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
int coglink_parseGetPlayers(struct lavaInfo *lavaInfo, struct requestInformation *res, char *pos, struct playerInfo *playerInfoStruct);

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
