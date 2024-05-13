/**
 * @file include/rest.h
 * @brief REST API function for the node.
*/

#ifndef COGLINK_LAVALINK_REST_H
#define COGLINK_LAVALINK_REST_H

#include <concord/types.h>

struct coglink_player_voice_data {
  char *token;
  char *endpoint;
  char *session_id;
};

struct coglink_player_queue {
  char **array;
  size_t size;
};

struct coglink_player {
  size_t node;
  u64snowflake guild_id;
  struct coglink_player_voice_data *voice_data;
  struct coglink_player_queue *queue;
};

struct coglink_players {
  struct coglink_player *array;
  size_t size;
};

struct coglink_user {
  u64snowflake id;
  u64snowflake channel_id;
};

struct coglink_users {
  struct coglink_user *array;
  size_t size;
};

struct coglink_decode_tracks_params {
  char **array;
  size_t size;
};

struct coglink_update_player_track_params {
  char *encoded;
  char *identifier;
  char *userData;
};

struct coglink_update_player_filters_equalizer_params {
  int band;
  float gain;
};

struct coglink_update_player_filters_karaoke_params {
  float level;
  float monoLevel;
  float filterBand;
  float filterWidth;
};

struct coglink_update_player_filters_timescale_params {
  float speed;
  float pitch;
  float rate;
};

struct coglink_update_player_filters_tremolo_params {
  float frequency;
  float depth;
};

struct coglink_update_player_filters_vibrato_params {
  float frequency;
  float depth;
};

struct coglink_update_player_filters_rotation_params {
  float frequency;
  float depth;
};

struct coglink_update_player_filters_distortion_params {
  float sinOffset;
  float sinScale;
  float cosOffset;
  float cosScale;
  float tanOffset;
  float tanScale;
  float offset;
  float scale;
};

struct coglink_update_player_filters_channelMix_params {
  float leftToLeft;
  float leftToRight;
  float rightToLeft;
  float rightToRight;
};

struct coglink_update_player_filters_lowPass_params {
  float smoothing;
};

struct coglink_update_player_filters_params {
  float volume;
  struct coglink_update_player_filters_equalizer_params *equalizer;
  struct coglink_update_player_filters_karaoke_params *karaoke;
  struct coglink_update_player_filters_timescale_params *timescale;
  struct coglink_update_player_filters_tremolo_params *tremolo;
  struct coglink_update_player_filters_vibrato_params *vibrato;
  struct coglink_update_player_filters_rotation_params *rotation;
  struct coglink_update_player_filters_distortion_params *distortion;
  struct coglink_update_player_filters_channelMix_params *channelMix;
  struct coglink_update_player_filters_lowPass_params *lowPass;
};

enum coglink_paused_state {
  COGLINK_PAUSED_STATE_FALSE = 1,
  COGLINK_PAUSED_STATE_TRUE = 2
};

struct coglink_update_player_params {
  struct coglink_update_player_track_params *track;
  int position;
  int endTime;
  int volume;
  enum coglink_paused_state paused;
  struct coglink_update_player_filters_params *filters;
};

struct coglink_update_session_params {
  bool resuming;
  int timeout;
};

/**
 * @brief Retrieves a user from the client's user list cache.
 * 
 * @param c_client The CogLink client.
 * @param user_id The ID of the user to retrieve.
 * 
 * @return The user if found, otherwise NULL.
*/
struct coglink_user *coglink_get_user(struct coglink_client *c_client, u64snowflake user_id);

/**
 * @brief Joins a voice channel.
 * 
 * @param c_client The CogLink client.
 * @param client The Discord client created with discord_init.
 * @param guild_id The ID of the guild.
 * @param channel_id The ID of the channel.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
 * 
 * @note c_client is not used. It is kept for consistency.
*/
int coglink_join_voice_channel(struct coglink_client *c_client, struct discord *client, u64snowflake guild_id, u64snowflake channel_id);

/**
 * @brief Leaves a voice channel.
 * 
 * @param c_client The CogLink client.
 * @param client The Discord client created with discord_init.
 * @param guild_id The ID of the guild.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
 * 
 * @note c_client is not used. It is kept for consistency.
*/
int coglink_leave_voice_channel(struct coglink_client *c_client, struct discord *client, u64snowflake guild_id);

/**
 * @brief Creates a local player.
 * 
 * @param c_client The CogLink client.
 * @param guild_id The ID of the guild.
 * 
 * @return The player if successful, otherwise NULL.
 * 
 * @note If the player already exists, it will return the existing player.
*/
struct coglink_player *coglink_create_player(struct coglink_client *c_client, u64snowflake guild_id);

/**
 * @brief Retrieves a local player.
 * 
 * @param c_client The CogLink client.
 * @param guild_id The ID of the guild.
 * 
 * @return The player if found, otherwise NULL.
*/
struct coglink_player *coglink_get_player(struct coglink_client *c_client, u64snowflake guild_id);

/**
 * @brief Retrieves a player's queue.
 * 
 * @param c_client The CogLink client.
 * @param player The player.
 * 
 * @return The player's queue.
 * 
 * @note c_client is not used. It is kept for consistency.
*/
struct coglink_player_queue *coglink_get_player_queue(struct coglink_client *c_client, struct coglink_player *player);

/**
 * @brief Retrieves a player's node.
 * 
 * @param c_client The CogLink client.
 * @param player The player.
 * 
 * @return The player's node.
*/
struct coglink_node *coglink_get_player_node(struct coglink_client *c_client, struct coglink_player *player);

/**
 * @brief Adds a track to a player's queue.
 * 
 * @param c_client The CogLink client.
 * @param player The player.
 * @param track The track to add.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
 * 
 * @note The track will be copied internally.
*/
int coglink_add_track_to_queue(struct coglink_client *c_client, struct coglink_player *player, char *track);

/**
 * @brief Removes a track from a player's queue.
 * 
 * @param c_client The CogLink client.
 * @param player The player.
 * @param position The position of the track to remove.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
*/
int coglink_remove_track_from_queue(struct coglink_client *c_client, struct coglink_player *player, size_t position);

/**
 * @brief Removes a local player.
 * 
 * @param c_client The CogLink client.
 * @param player The player to remove.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
*/
int coglink_remove_player(struct coglink_client *c_client, struct coglink_player *player);

/**
 * @brief Performs a search request.
 * 
 * @param c_client The CogLink client.
 * @param node The node to load the tracks from.
 * @param identifier The query/identifier to search for.
 * @param response The response to store the results.
 * 
 * @return The node if found, otherwise NULL.
 * 
 * @note The response must be freed with coglink_free_load_tracks if not COGLINK_FAILED.
 * @note c_client is not used. It is kept for consistency.
*/
int coglink_load_tracks(struct coglink_client *c_client, struct coglink_node *node, char *identifier, struct coglink_load_tracks *response);

/**
 * @brief Decodes a track.
 * 
 * @param c_client The CogLink client.
 * @param node The node.
 * @param track The track to decode.
 * @param response The response to store the results.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
 * 
 * @note The response must be freed with coglink_free_track if not COGLINK_FAILED.
 * 
 * @note c_client is not used. It is kept for consistency.
*/
int coglink_decode_track(struct coglink_client *c_client, struct coglink_node *node, char *track, struct coglink_track *response);

/**
 * @brief Decodes multiple tracks.
 * 
 * @param c_client The CogLink client.
 * @param node The node.
 * @param params The parameters to decode the tracks.
 * @param response The response to store the results.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
 * 
 * @note The response must be freed with coglink_free_tracks if not COGLINK_FAILED.
 * @note c_client is not used. It is kept for consistency.
*/
int coglink_decode_tracks(struct coglink_client *c_client, struct coglink_node *node, struct coglink_decode_tracks_params *params, struct coglink_tracks *response);

/**
 * @brief Updates a player.
 * 
 * @param c_client The CogLink client.
 * @param player The player to update.
 * @param params The parameters to update the player.
 * @param response The response to store the results.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
 * 
 * @note The response must be freed with coglink_free_update_player if not COGLINK_FAILED.
 */
int coglink_update_player(struct coglink_client *c_client, struct coglink_player *player, struct coglink_update_player_params *params, struct coglink_update_player *response);

/**
 * @brief Destroys a player.
 * 
 * @param c_client The CogLink client.
 * @param player The player to destroy.
 * 
 * @note This will NOT remove a player as a local player.
*/
void coglink_destroy_player(struct coglink_client *c_client, struct coglink_player *player);

/**
 * @brief Updates information about a session.
 * 
 * @param c_client The CogLink client.
 * @param node The node.
 * @param params The parameters to update the session.
 * @param response The response to store the results.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
 * 
 * @note The response must be freed with coglink_free_update_session if not COGLINK_FAILED.
 * @note c_client is not used. It is kept for consistency.
*/
int coglink_update_session(struct coglink_client *c_client, struct coglink_node *node, struct coglink_update_session_params *params, struct coglink_update_session *response);

/**
 * @brief Retrieves a node information.
 * 
 * @param c_client The CogLink client.
 * @param node The node to retrieve the information.
 * @param info The information to store the results.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED or COGLINK_PARSE_FAILED.
 * 
 * @note The info must be freed with coglink_free_node_info if not COGLINK_FAILED.
 * @note c_client is not used. It is kept for consistency.
*/
int coglink_get_node_info(struct coglink_client *c_client, struct coglink_node *node, struct coglink_node_info *info);

/**
 * @brief Retrieves a node version.
 * 
 * @param c_client The CogLink client.
 * @param node The node to retrieve the version.
 * @param version The version to store the results.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
 * 
 * @note The version must be freed with coglink_free_node_version if not COGLINK_FAILED.
 * @note c_client is not used. It is kept for consistency.
 */
int coglink_get_node_version(struct coglink_client *c_client, struct coglink_node *node, struct coglink_node_version *version);

/**
 * @brief Retrieves a node stats.
 * 
 * @param c_client The CogLink client.
 * @param node The node to retrieve the stats.
 * @param stats The stats to store the results.
 * 
 * @return COGLINK_SUCCESS if successful, otherwise COGLINK_FAILED.
 * 
 * @note The stats must be freed with coglink_free_stats if not COGLINK_FAILED.
 * @note c_client is not used. It is kept for consistency.
*/
int coglink_get_stats(struct coglink_client *c_client, struct coglink_node *node, struct coglink_stats *stats);

#endif