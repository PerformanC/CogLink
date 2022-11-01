#ifndef PLAYER_H
#define PLAYER_H

void coglink_stopPlayer(struct lavaInfo *lavaInfo, u64snowflake guildID);

void coglink_pausePlayer(struct lavaInfo *lavaInfo, u64snowflake guildID, char *pause);

void coglink_seekTrack(struct lavaInfo *lavaInfo, u64snowflake guildID, char *position);

void coglink_volumePlayer(struct lavaInfo *lavaInfo, u64snowflake guildID, char *volume);

void coglink_destroyPlayer(struct lavaInfo *lavaInfo, u64snowflake guildID);

#endif