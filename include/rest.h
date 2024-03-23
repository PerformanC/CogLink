#ifndef COGLINK_LAVALINK_REST_H
#define COGLINK_LAVALINK_REST_H

#include <concord/types.h>

struct coglink_player_queue {
  char **array;
  size_t size;
};

struct coglink_player {
  size_t node;
  u64snowflake guild_id;
  struct coglink_voice_data *voice_data;
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
  /* todo: remove (?) */
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

struct coglink_update_player_params {
  struct coglink_update_player_track_params *track;
  int position;
  int endTime;
  int volume;
  bool paused;
  struct coglink_update_player_filters_params *filters;
};

struct coglink_user *coglink_get_user(struct coglink_client *c_client, u64snowflake user_id);

int coglink_join_voice_channel(struct discord *client, u64snowflake guild_id, u64snowflake channel_id);

struct coglink_player *coglink_create_player(struct coglink_client *c_client, u64snowflake guild_id);

struct coglink_player *coglink_get_player(struct coglink_client *c_client, u64snowflake guild_id);

/* Experimental */
struct coglink_player_queue *coglink_get_player_queue(struct coglink_client *c_client, struct coglink_player *player);

int coglink_add_track_to_queue(struct coglink_client *c_client, struct coglink_player *player, char *track);

int coglink_remove_track_from_queue(struct coglink_client *c_client, struct coglink_player *player, size_t position);

int coglink_remove_player(struct coglink_client *c_client, struct coglink_player *player);

int coglink_load_tracks(struct coglink_client *c_client, struct coglink_player *player, char *identifier, struct coglink_load_tracks_response *response);

int coglink_decode_track(struct coglink_client *c_client, struct coglink_player *player, char *track, struct coglink_track *response);

int coglink_decode_tracks(struct coglink_client *c_client, struct coglink_player *player, struct coglink_decode_tracks_params *params, struct coglink_tracks *response);

void coglink_free_decode_tracks(struct coglink_tracks *tracks);

int coglink_update_player(struct coglink_client *c_client, struct coglink_player *player, struct coglink_update_player_params *params, struct coglink_update_player_response *response);

void coglink_destroy_player(struct coglink_client *c_client, struct coglink_player *player);

int coglink_get_node_info(struct coglink_client *c_client, struct coglink_node *node, struct coglink_node_info *info);

char *coglink_get_node_version(struct coglink_client *c_client, struct coglink_node *node);

void coglink_free_node_version(char *version);

int coglink_get_stats(struct coglink_client *c_client, struct coglink_node *node, struct coglink_stats_payload *stats);

#endif