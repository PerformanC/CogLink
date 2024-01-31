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

struct coglink_user *coglink_get_user(struct coglink_client *c_client, u64snowflake user_id);

int coglink_join_voice_channel(struct discord *client, u64snowflake guild_id, u64snowflake channel_id);

struct coglink_player *coglink_create_player(struct coglink_client *c_client, u64snowflake guild_id);

struct coglink_player *coglink_get_player(struct coglink_client *c_client, u64snowflake guild_id);

/* Experimental */
struct coglink_player_queue *coglink_get_player_queue(struct coglink_client *c_client, struct coglink_player *player);

int coglink_add_track_to_queue(struct coglink_client *c_client, struct coglink_player *player, char *track);

int coglink_remove_track_from_queue(struct coglink_client *c_client, struct coglink_player *player, char *track);

int coglink_remove_player(struct coglink_client *c_client, struct coglink_player *player);

int coglink_load_tracks(struct coglink_client *c_client, struct coglink_player *player, char *identifier);

int coglink_play_track(struct coglink_client *c_client, struct coglink_player *player, char *track);

#endif