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

struct coglink_partial_tracks {
  struct coglink_partial_track *array;
  size_t size;
};

struct coglink_track {
  /* todo: use dynamic */
  char encoded[512];
  struct coglink_partial_track *info;
};

struct coglink_track_start_payload {
  u64snowflake guildId;
  struct coglink_track *track;
};

#define COGLINK_TRACK_START 41

/* todo: dynamic allocation but demanding copy if used for longer */
struct coglink_track_end_payload {
  u64snowflake guildId;
  struct coglink_track *track;
  char reason[16];
};

#define COGLINK_TRACK_END 42

struct coglink_exception_payload {
  char *message;
  /* todo: use enums */
  char *severity;
  char *cause;
};

struct coglink_track_exception_payload {
  u64snowflake guildId;
  struct coglink_track *track;
  struct coglink_exception_payload *exception;
};

#define COGLINK_TRACK_EXCEPTION 43

struct coglink_track_stuck_payload {
  u64snowflake guildId;
  struct coglink_track *track;
  int thresholdMs;
};

#define COGLINK_TRACK_STUCK 44

struct coglink_websocket_closed_payload {
  int code;
  char reason[256];
  bool byRemote;
};

#define COGLINK_WEBSOCKET_CLOSED 45

/* coglink_parse_load_tracks_response */

enum coglink_load_type {
  COGLINK_LOAD_TYPE_TRACK,
  COGLINK_LOAD_TYPE_PLAYLIST,
  COGLINK_LOAD_TYPE_SEARCH,
  COGLINK_LOAD_TYPE_EMPTY,
  COGLINK_LOAD_TYPE_ERROR
};

struct coglink_load_tracks_response {
  enum coglink_load_type type;
  /* todo: implement it without voiding (?) */
  void *data;
};

struct coglink_load_tracks_track_response {
  struct coglink_track *array;
  size_t size;
};

/* todo: change name (?) */
struct coglink_playlist_info {
  char name[256];
  int selectedTrack;
};

struct coglink_load_tracks_playlist_response {
  struct coglink_playlist_info *info;
  struct coglink_track *tracks;
  /* plugins are not supported. Use NodeLink instead. */
};

struct coglink_load_tracks_search_response {
  struct coglink_track *array;
  size_t size;
};

/* Empty will not allocate data */

struct coglink_load_tracks_error_response {
  char *message;
  char *severity;
  char *cause;
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

void *coglink_parse_load_tracks_response(struct coglink_load_tracks_response *response, const char *json, size_t length);

void coglink_free_load_tracks_response(struct coglink_load_tracks_response *response);

struct coglink_voice_state *coglink_parse_voice_state(const char *json, size_t length);

struct coglink_voice_server_update *coglink_parse_voice_server_update(const char *json, size_t length);

#endif
