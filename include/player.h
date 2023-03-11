/** \file
 * File containing the Lavalink player related functions.
 */

#ifndef PLAYER_H
#define PLAYER_H

struct coglink_equalizerStruct {
  char bands[COGLINK_BANDS_LENGTH];
  char gain[COGLINK_GAIN_LENGTH];
};

struct coglink_karaokeStruct {
  char level[16];
  char monoLevel[16];
  char filterBand[16];
  char filterWidth[16];
};

struct coglink_timescaleStruct {
  char speed[8];
  char pitch[8];
  char rate[8];
};

struct coglink_frequencyDepthStruct {
  char frequency[8];
  char depth[4];
};

struct coglink_distortionStruct {
  char sinOffset[8];
  char sinScale[8];
  char cosOffset[8];
  char cosScale[8];
  char tanOffset[8];
  char tanScale[8];
  char offset[8];
  char scale[8];
};

struct coglink_channelMixStruct {
  char leftToLeft[4];
  char leftToRight[4];
  char rightToLeft[4];
  char rightToRight[4];
};

struct coglink_playerInfoTrack {
  char encoded[COGLINK_TRACK_LENGTH];
  struct coglink_parsedTrack *info;
};

struct coglink_playerInfoState {
  char time[COGLINK_TIME_LENGTH];
  char position[COGLINK_VIDEO_LENGTH];
  char connected[COGLINK_TRUE_FALSE_LENGTH];
  char ping[COGLINK_PING_LENGTH];
};

struct coglink_playerInfoVoice {
  char token[COGLINK_TOKEN_LENGTH];
  char endpoint[COGLINK_ENDPOINT_LENGTH];
  char sessionId[COGLINK_SESSIONID_LENGTH];
};

struct coglink_playerInfoFilters {
  char volume[COGLINK_VOLUME_LENGTH];
  struct coglink_equalizerStruct *equalizer;
  struct coglink_karaokeStruct *karaoke;
  struct coglink_timescaleStruct *timescale;
  struct coglink_frequencyDepthStruct *tremolo;
  struct coglink_frequencyDepthStruct *vibrato;
  char rotation[8];
  struct coglink_distortionStruct *distortion;
  struct coglink_channelMixStruct *channelMix;
  char lowPass[8];
};

struct coglink_playerInfo {
  char guildId[COGLINK_GUILD_ID_LENGTH];
  struct coglink_playerInfoTrack *track;
  char volume[COGLINK_VOLUME_LENGTH];
  char paused[COGLINK_TRUE_FALSE_LENGTH];
  struct coglink_playerInfoState *state;
  struct coglink_playerInfoVoice *voice;
  struct coglink_playerInfoFilters *filters;
};

/**
 * Creates a player for the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId Id of the guild.
 */
int coglink_createPlayer(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId);

/**
 * Retrieves the players.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_getPlayers(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, struct coglink_requestInformation *res);

/**
 * Parses the response body of the function coglink_getPlayers.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param res Structure with the information of the request.
 * @param pos Position of the player that will be parsed
 * @param playerInfoStruct Structure with the parsed information.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_parseGetPlayers(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, char *pos, struct coglink_playerInfo *playerInfoStruct);

/**
 * Frees the allocations generated while performing the function coglink_getPlayers.
 * @param res Structure with the information of the request.
 */
void coglink_getPlayersCleanup(struct coglink_requestInformation *res);

/**
 * Starts playing a track in the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param track Track that will be played.
 * @param guildId ID of the guild that the track will be played.
 * @returns COGLINK_SUCCESS / ERROR
 */
int coglink_playSong(struct coglink_lavaInfo *lavaInfo, char *track, u64snowflake guildId);

/**
 * Destroys the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 */
void coglink_destroyPlayer(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId);

/**
 * Stops the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 */
void coglink_stopPlayer(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId);

/**
 * Pauses the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 * @param pause String that can be true or false to whatever pause or not.
 */
void coglink_pausePlayer(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, char *pause);

/**
 * Makes the track go back to play at x ms.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 * @param position String number of the time in ms.
 */
void coglink_seekTrack(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, char *position);

/**
 * Changes the volume of the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 * @param volume String containing the volume that will be set.
 */
void coglink_setPlayerVolume(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, char *volume);

/**
 * Sets a effect to the player of the specified guild.
 * @param lavaInfo Structure with important informations of the Lavalink.
 * @param guildId ID of the guild of the player.
 * @param effect Type of the effect that will be set.
 * @param value Value of the effect.
 */
void coglink_setEffect(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, int effect, char *value);

#endif
