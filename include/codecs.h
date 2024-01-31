#ifndef COGLINK_CODECS
#define COGLINK_CODECS

#include "utils.h"

#include <concord/types.h>

/* coglink_parse_websocket_data */

#define COGLINK_PARSE_ERROR -1

struct coglink_ready_payload {
  char *session_id;
  bool resumed;
};

#define COGLINK_READY 0

struct coglink_player_state_payload {
  int time;
  int position;
  bool connected;
  int ping;
};

struct coglink_player_update_payload {
  u64snowflake guildId;
  struct coglink_player_state_payload *state;
};

#define COGLINK_PLAYER_UPDATE 2

struct coglink_stats_memory_payload {
  int free;
  int used;
  int allocated;
  int reservable;
};

struct coglink_stats_cpu_payload {
  int cores;
  int systemLoad;
  int lavalinkLoad;
};

struct coglink_stats_frame_stats_payload {
  int sent;
  int nulled;
  int deficit;
};

struct coglink_stats_payload {
  int players;
  int playingPlayers;
  int uptime;
  struct coglink_stats_memory_payload *memory;
  struct coglink_stats_cpu_payload *cpu;
  struct coglink_stats_frame_stats_payload *frameStats;
};

#define COGLINK_STATS 3

struct coglink_partial_track {
  char identifier[64];
  bool isSeekable;
  char author[128];
  size_t length;
  bool isStream;
  size_t position;
  char title[256];
  char uri[256];
  char isrc[64];
  char artworkUrl[256];
  char sourceName[16];
};

struct coglink_track {
  char encoded[512];
  struct coglink_partial_track *info;
};

struct coglink_track_start_payload {
  u64snowflake guildId;
  struct coglink_track *track;
};

#define COGLINK_TRACK_START 41

struct coglink_track_end_payload {
  u64snowflake guildId;
  struct coglink_track *track;
  char reason[16];
};

#define COGLINK_TRACK_END 42

/* coglink_parse_load_tracks_response */

struct coglink_tracks {
  struct coglink_partial_track *array;
  size_t size;
};

enum coglink_load_type {
  COGLINK_LOAD_TYPE_TRACK,
  COGLINK_LOAD_TYPE_PLAYLIST,
  COGLINK_LOAD_TYPE_SEARCH,
  COGLINK_LOAD_TYPE_EMPTY,
  COGLINK_LOAD_TYPE_ERROR
};

struct coglink_load_tracks_response {
  enum coglink_load_type type;
  struct coglink_tracks *tracks;
};

/* coglink_parse_voice_state */

struct coglink_voice_state {
  u64snowflake guild_id;
  u64snowflake channel_id;
  u64snowflake user_id;
  char *session_id;
};

/* coglink_parse_voice_server_update */

struct coglink_voice_server_update {
  char *token;
  char *endpoint;
  u64snowflake guild_id;
};

void *coglink_parse_websocket_data(int *type, const char *json, size_t length);

struct coglink_load_tracks_response *coglink_parse_load_tracks_response(const char *json, size_t length);

struct coglink_voice_state *coglink_parse_voice_state(const char *json, size_t length);

struct coglink_voice_server_update *coglink_parse_voice_server_update(const char *json, size_t length);

#endif
