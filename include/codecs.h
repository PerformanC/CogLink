#ifndef COGLINK_CODECS
#define COGLINK_CODECS

#include "utils.h"

#include <concord/types.h>

/* coglink_parse_websocket_data */

#define COGLINK_PARSE_ERROR -1

struct coglink_ready {
  char *session_id;
  bool resumed;
};

#define COGLINK_READY 0

struct coglink_player_state {
  int time;
  int position;
  bool connected;
  int ping;
};

struct coglink_player_update {
  u64snowflake guildId;
  struct coglink_player_state *state;
};

#define COGLINK_PLAYER_UPDATE 2

struct coglink_stats_memory {
  int free;
  int used;
  int allocated;
  int reservable;
};

struct coglink_stats_cpu {
  int cores;
  int systemLoad;
  int lavalinkLoad;
};

struct coglink_stats_frame_stats {
  int sent;
  int nulled;
  int deficit;
};

struct coglink_stats {
  int players;
  int playingPlayers;
  int uptime;
  struct coglink_stats_memory *memory;
  struct coglink_stats_cpu *cpu;
  struct coglink_stats_frame_stats *frameStats;
};

#define COGLINK_STATS 3

struct coglink_track_info {
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
  char *encoded;
  struct coglink_track_info *info;
};

struct coglink_tracks {
  struct coglink_track **array;
  size_t size;
};

struct coglink_track_start {
  u64snowflake guildId;
  struct coglink_track *track;
};

#define COGLINK_TRACK_START 41

enum coglink_track_end_reason {
  COGLINK_TRACK_END_REASON_FINISHED,
  COGLINK_TRACK_END_REASON_LOAD_FAILED,
  COGLINK_TRACK_END_REASON_STOPPED,
  COGLINK_TRACK_END_REASON_REPLACED,
  COGLINK_TRACK_END_REASON_CLEANUP
};

struct coglink_track_end {
  u64snowflake guildId;
  struct coglink_track *track;
  enum coglink_track_end_reason reason;
};

#define COGLINK_TRACK_END 42

enum coglink_exception_severity {
  COGLINK_EXCEPTION_SEVERITY_COMMON,
  COGLINK_EXCEPTION_SEVERITY_SUSPICIOUS,
  COGLINK_EXCEPTION_SEVERITY_FAULT
};

struct coglink_exception {
  char *message;
  enum coglink_exception_severity severity;
  char *cause;
};

struct coglink_track_exception {
  u64snowflake guildId;
  struct coglink_track *track;
  struct coglink_exception *exception;
};

#define COGLINK_TRACK_EXCEPTION 43

struct coglink_track_stuck {
  u64snowflake guildId;
  struct coglink_track *track;
  int thresholdMs;
};

#define COGLINK_TRACK_STUCK 44

struct coglink_websocket_closed {
  int code;
  char *reason;
  bool byRemote;
};

#define COGLINK_WEBSOCKET_CLOSED 45

/* coglink_parse_load_tracks */

enum coglink_load_type {
  COGLINK_LOAD_TYPE_TRACK,
  COGLINK_LOAD_TYPE_PLAYLIST,
  COGLINK_LOAD_TYPE_SEARCH,
  COGLINK_LOAD_TYPE_EMPTY,
  COGLINK_LOAD_TYPE_ERROR
};

struct coglink_load_tracks {
  enum coglink_load_type type;
  void *data;
};

/* same as coglink_track */
struct coglink_load_tracks_track {
  char *encoded;
  struct coglink_track_info *info;
};

/* todo: change name (?) */
struct coglink_playlist_info {
  char name[256];
  int selectedTrack;
};

struct coglink_load_tracks_playlist {
  struct coglink_playlist_info *info;
  struct coglink_tracks *tracks;
  /* plugins are not supported. Use NodeLink instead. */
};

struct coglink_load_tracks_search {
  struct coglink_track **array;
  size_t size;
};

/* Empty will not allocate data */

enum coglink_load_tracks_error_severity {
  COGLINK_LOAD_TRACKS_ERROR_SEVERITY_COMMON,
  COGLINK_LOAD_TRACKS_ERROR_SEVERITY_SUSPICIOUS,
  COGLINK_LOAD_TRACKS_ERROR_SEVERITY_FAULT
};

struct coglink_load_tracks_error {
  char *message;
  enum coglink_load_tracks_error_severity severity;
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

/* coglink_parse_guild_create */

struct coglink_guild_create {
  u64snowflake guild_id;
  jsmnf_pair *pairs;
};

/* coglink_parse_update_player */

struct coglink_update_player_state {
  int time;
  int position;
  bool connected;
  int ping;
};

struct coglink_update_player {
  struct coglink_track *track;
  int volume;
  bool paused;
  struct coglink_update_player_state *state;
  struct coglink_update_player_filters_params *filters;
};

/* coglink_parse_single_user_guild_create */

struct coglink_single_user_guild_create {
  int type;
  u64snowflake user_id;
  u64snowflake vc_id;
  char *session_id;
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

struct coglink_node_version {
  int major;
  int minor;
  int patch;
  char *preRelease;
};

#define coglink_parse_track(track_info, pairs, json)                                                                                      \
  char *path[] = { "encoded", NULL };                                                                                                     \
  FIND_FIELD_PATH(json, pairs, encoded, "encoded", 1);                                                                                    \
  track_info->encoded = malloc(encoded->v.len + 1);                                                                                       \
  snprintf(track_info->encoded, encoded->v.len + 1, "%.*s", (int)encoded->v.len, json + encoded->v.pos);                                  \
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

int coglink_parse_websocket_data(const char *json, size_t json_length, void **response, int *event_type);

void coglink_free_track(struct coglink_track *track);

void coglink_free_tracks(struct coglink_tracks *tracks);

int coglink_parse_load_tracks(struct coglink_load_tracks *response, const char *json, size_t json_length);

void coglink_free_load_tracks(struct coglink_load_tracks *response);

int coglink_parse_voice_state(const char *json, size_t json_length, struct coglink_voice_state *response);

void coglink_free_voice_state(struct coglink_voice_state *voice_state);

int coglink_parse_voice_server_update(const char *json, size_t json_length, struct coglink_voice_server_update *response);

void coglink_free_voice_server_update(struct coglink_voice_server_update *voice_server_update);

int coglink_parse_guild_create(const char *json, size_t json_length, struct coglink_guild_create *response);

void coglink_free_guild_create(struct coglink_guild_create *guild_create);

int coglink_parse_single_user_guild_create(jsmnf_pair *pairs, const char *json, char *i_str, u64snowflake bot_id, struct coglink_single_user_guild_create *response);

int coglink_parse_update_player(struct coglink_update_player *response, const char *json, size_t json_length);

void coglink_free_update_player(struct coglink_update_player *response);

int coglink_parse_node_info(struct coglink_node_info *response, const char *json, size_t json_length);

void coglink_free_node_info(struct coglink_node_info *node_info);

int coglink_parse_stats(struct coglink_stats *response, const char *json, size_t json_length);

int coglink_parse_version(struct coglink_node_version *response, const char *version, size_t version_length);

void coglink_free_node_version(struct coglink_node_version *version);

#endif
