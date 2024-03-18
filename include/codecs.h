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

struct coglink_tracks {
  struct coglink_track **array;
  size_t size;
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

/* same as coglink_track */
struct coglink_load_tracks_track_response {
  /* todo: use dynamic */
  char encoded[512];
  struct coglink_partial_track *info;
};

/* todo: change name (?) */
struct coglink_playlist_info {
  char name[256];
  int selectedTrack;
};

struct coglink_load_tracks_playlist_response {
  struct coglink_playlist_info *info;
  struct coglink_tracks *tracks;
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

/* coglink_parse_node_info */

struct coglink_node_info_version {
  char *semver;
  int major;
  int minor;
  int patch;
  char *preRelease;
  char *build;
};

/* todo: generic array */
struct coglink_node_info_sourceManagers {
  char **array;
  size_t size;
};

struct coglink_node_info_filters {
  char **array;
  size_t size;
};

struct coglink_node_info_git {
  char *branch;
  char *commit;
  int commitTime;
};

struct coglink_node_info {
  struct coglink_node_info_version *version;
  int buildTime;
  struct coglink_node_info_git *git;
  char *jvm;
  char *lavaplayer;
  struct coglink_node_info_sourceManagers *sourceManagers;
  struct coglink_node_info_filters *filters;
};

#define coglink_parse_track(pairs, json)                                                                                                  \
  struct coglink_track *track_info = malloc(sizeof(struct coglink_track));                                                                \
  track_info->info = malloc(sizeof(struct coglink_partial_track));                                                                        \
                                                                                                                                          \
  char *path[] = { "encoded", NULL };                                                                                                     \
  FIND_FIELD_PATH(json, pairs, encoded, "encoded", 1);                                                                                    \
  snprintf(track_info->encoded, sizeof(track_info->encoded), "%.*s", (int)encoded->v.len, json + encoded->v.pos);                         \
                                                                                                                                          \
  path[0] = "info";                                                                                                                       \
  path[1] = "identifier";                                                                                                                 \
  FIND_FIELD_PATH(json, pairs, identifier, "identifier", 2);                                                                              \
  snprintf(track_info->info->identifier, sizeof(track_info->info->identifier), "%.*s", (int)identifier->v.len, json + identifier->v.pos); \
                                                                                                                                          \
  path[1] = "isSeekable";                                                                                                                 \
  FIND_FIELD_PATH(json, pairs, isSeekable, "isSeekable", 2);                                                                              \
  if (json[isSeekable->v.pos] == 't') track_info->info->isSeekable = true;                                                                \
  else track_info->info->isSeekable = false;                                                                                              \
                                                                                                                                          \
  path[1] = "author";                                                                                                                     \
  FIND_FIELD_PATH(json, pairs, author, "author", 2);                                                                                      \
  snprintf(track_info->info->author, sizeof(track_info->info->author), "%.*s", (int)author->v.len, json + author->v.pos);                 \
                                                                                                                                          \
  path[1] = "length";                                                                                                                     \
  FIND_FIELD_PATH(json, pairs, length, "length", 2);                                                                                      \
  PAIR_TO_SIZET(json, length, lengthStr, track_info->info->length, 16);                                                                   \
                                                                                                                                          \
  path[1] = "isStream";                                                                                                                   \
  FIND_FIELD_PATH(json, pairs, isStream, "isStream", 2);                                                                                  \
  if (json[isStream->v.pos] == 't') track_info->info->isStream = true;                                                                    \
  else track_info->info->isStream = false;                                                                                                \
                                                                                                                                          \
  path[1] = "position";                                                                                                                   \
  FIND_FIELD_PATH(json, pairs, position, "position", 2);                                                                                  \
  PAIR_TO_SIZET(json, position, positionStr, track_info->info->position, 16);                                                             \
                                                                                                                                          \
  path[1] = "title";                                                                                                                      \
  FIND_FIELD_PATH(json, pairs, title, "title", 2);                                                                                        \
  snprintf(track_info->info->title, sizeof(track_info->info->title), "%.*s", (int)title->v.len, json + title->v.pos);                     \
                                                                                                                                          \
  path[1] = "uri";                                                                                                                        \
  FIND_FIELD_PATH(json, pairs, uri, "uri", 2);                                                                                            \
  snprintf(track_info->info->uri, sizeof(track_info->info->uri), "%.*s", (int)uri->v.len, json + uri->v.pos);                             \
                                                                                                                                          \
  path[1] = "isrc";                                                                                                                       \
  FIND_FIELD_PATH(json, pairs, isrc, "isrc", 2);                                                                                          \
  snprintf(track_info->info->isrc, sizeof(track_info->info->isrc), "%.*s", (int)isrc->v.len, json + isrc->v.pos);                         \
                                                                                                                                          \
  path[1] = "artworkUrl";                                                                                                                 \
  FIND_FIELD_PATH(json, pairs, artworkUrl, "artworkUrl", 2);                                                                              \
  snprintf(track_info->info->artworkUrl, sizeof(track_info->info->artworkUrl), "%.*s", (int)artworkUrl->v.len, json + artworkUrl->v.pos); \
                                                                                                                                          \
  path[1] = "sourceName";                                                                                                                 \
  FIND_FIELD_PATH(json, pairs, sourceName, "sourceName", 2);                                                                              \
  snprintf(track_info->info->sourceName, sizeof(track_info->info->sourceName), "%.*s", (int)sourceName->v.len, json + sourceName->v.pos);

#define coglink_parse_partial_track(pairs, json)                                                                              \
  struct coglink_partial_track *track_info = malloc(sizeof(struct coglink_partial_track));                                    \
                                                                                                                              \
  FIND_FIELD(json, identifier, "identifier");                                                                  \
  snprintf(track_info->identifier, sizeof(track_info->identifier), "%.*s", (int)identifier->v.len, json + identifier->v.pos); \
                                                                                                                              \
  FIND_FIELD(json, isSeekable, "isSeekable");                                                                  \
  if (json[isSeekable->v.pos] == 't') track_info->isSeekable = true;                                                          \
  else track_info->isSeekable = false;                                                                                        \
                                                                                                                              \
  FIND_FIELD(json, author, "author");                                                                          \
  snprintf(track_info->author, sizeof(track_info->author), "%.*s", (int)author->v.len, json + author->v.pos);                 \
                                                                                                                              \
  FIND_FIELD(json, length, "length");                                                                          \
  PAIR_TO_SIZET(json, length, lengthStr, track_info->length, 16);                                                             \
                                                                                                                              \
  FIND_FIELD(json, isStream, "isStream");                                                                      \
  if (json[isStream->v.pos] == 't') track_info->isStream = true;                                                              \
  else track_info->isStream = false;                                                                                          \
                                                                                                                              \
  FIND_FIELD(json, position, "position");                                                                      \
  PAIR_TO_SIZET(json, position, positionStr, track_info->position, 16);                                                       \
                                                                                                                              \
  FIND_FIELD(json, title, "title");                                                                            \
  snprintf(track_info->title, sizeof(track_info->title), "%.*s", (int)title->v.len, json + title->v.pos);                     \
                                                                                                                              \
  FIND_FIELD(json, uri, "uri");                                                                                \
  snprintf(track_info->uri, sizeof(track_info->uri), "%.*s", (int)uri->v.len, json + uri->v.pos);                             \
                                                                                                                              \
  FIND_FIELD(json, isrc, "isrc");                                                                              \
  snprintf(track_info->isrc, sizeof(track_info->isrc), "%.*s", (int)isrc->v.len, json + isrc->v.pos);                         \
                                                                                                                              \
  FIND_FIELD(json, artworkUrl, "artworkUrl");                                                                  \
  snprintf(track_info->artworkUrl, sizeof(track_info->artworkUrl), "%.*s", (int)artworkUrl->v.len, json + artworkUrl->v.pos); \
                                                                                                                              \
  FIND_FIELD(json, sourceName, "sourceName");                                                                  \
  snprintf(track_info->sourceName, sizeof(track_info->sourceName), "%.*s", (int)sourceName->v.len, json + sourceName->v.pos);

void *coglink_parse_websocket_data(int *type, const char *json, size_t length);

void *coglink_parse_load_tracks_response(struct coglink_load_tracks_response *response, const char *json, size_t length);

void coglink_free_load_tracks_response(struct coglink_load_tracks_response *response);

struct coglink_voice_state *coglink_parse_voice_state(const char *json, size_t length);

void coglink_free_voice_state(struct coglink_voice_state *voiceState);

struct coglink_voice_server_update *coglink_parse_voice_server_update(const char *json, size_t length);

void coglink_free_voice_server_update(struct coglink_voice_server_update *voiceServerUpdate);

void *coglink_parse_node_info(struct coglink_node_info *response, const char *json, size_t length);

void coglink_free_node_info(struct coglink_node_info *node_info);

void *coglink_parse_stats(struct coglink_stats_payload *response, const char *json, size_t length);

#endif
