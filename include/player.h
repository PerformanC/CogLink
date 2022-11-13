#ifndef PLAYER_H
#define PLAYER_H

void coglink_playSong(const struct lavaInfo *lavaInfo, char *track, u64snowflake guildId);

void coglink_stopPlayer(const struct lavaInfo *lavaInfo, u64snowflake guildID);

void coglink_pausePlayer(const struct lavaInfo *lavaInfo, u64snowflake guildID, char *pause);

void coglink_seekTrack(const struct lavaInfo *lavaInfo, u64snowflake guildID, char *position);

void coglink_setPlayerVolume(const struct lavaInfo *lavaInfo, u64snowflake guildID, char *volume);

void coglink_destroyPlayer(const struct lavaInfo *lavaInfo, u64snowflake guildID);

#endif