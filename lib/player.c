#include <string.h>

#include <concord/discord.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>

void coglink_playSong(const struct lavaInfo *lavaInfo, char *track, u64snowflake guildId) {
  char payload[2048];
  snprintf(payload, sizeof(payload), "{\"op\":\"play\",\"guildId\":\"%"PRIu64"\",\"track\":\"%s\",\"noReplace\":false,\"pause\":false}", guildId, track);

  __coglink_sendPayload(lavaInfo, payload, sizeof(payload), "play");
}

void coglink_stopPlayer(const struct lavaInfo *lavaInfo, u64snowflake guildID) {
  char payload[64];
  snprintf(payload, sizeof(payload), "{\"op\":\"stop\",\"guildId\":\"%"PRIu64"\"}", guildID);
  __coglink_sendPayload(lavaInfo, payload, sizeof(payload), "stop");
}

void coglink_pausePlayer(const struct lavaInfo *lavaInfo, u64snowflake guildID, char *pause) {
  char payload[64];
  snprintf(payload, sizeof(payload), "{\"op\":\"pause\",\"guildId\":\"%"PRIu64"\",\"pause\":%s}", guildID, pause);
  __coglink_sendPayload(lavaInfo, payload, sizeof(payload), "pause");
}

void coglink_seekTrack(const struct lavaInfo *lavaInfo, u64snowflake guildID, char *position) {
  char payload[128];
  snprintf(payload, sizeof(payload), "{\"op\":\"seek\",\"guildId\":\"%"PRIu64"\",\"position\":%s}", guildID, position);
  __coglink_sendPayload(lavaInfo, payload, sizeof(payload), "seek");
}

void coglink_setPlayerVolume(const struct lavaInfo *lavaInfo, u64snowflake guildID, char *volume) {
  char payload[128];
  snprintf(payload, sizeof(payload), "{\"op\":\"volume\",\"guildId\":\"%"PRIu64"\",\"volume\":%s}", guildID, volume);
  __coglink_sendPayload(lavaInfo, payload, sizeof(payload), "volume");
}

void coglink_destroyPlayer(const struct lavaInfo *lavaInfo, u64snowflake guildID) {
  char payload[64];
  snprintf(payload, sizeof(payload), "{\"op\":\"destroy\",\"guildId\":\"%"PRIu64"\"}", guildID);
  __coglink_sendPayload(lavaInfo, payload, sizeof(payload), "destroy");
}

void coglink_setEffect(const struct lavaInfo *lavaInfo, u64snowflake guildID, int effect, char *value) {
  char payload[strnlen(value, 512) + 128];
  char effectStr[11] = "VOLUME";
  switch (effect) {
    case FILTER_VOLUME: {
      snprintf(effectStr, sizeof(effectStr), "volume");
      break;
    }
    case FILTER_EQUALIZER: {
      snprintf(effectStr, sizeof(effectStr), "equalizer");
      break;
    }
    case FILTER_KARAOKE: {
      snprintf(effectStr, sizeof(effectStr), "karaoke");
      break;
    }
    case FILTER_TIMESCALE: {
      snprintf(effectStr, sizeof(effectStr), "timescale");
      break;
    }
    case FILTER_TREMOLO: {
      snprintf(effectStr, sizeof(effectStr), "tremolo");
      break;
    }
    case FILTER_ROTATION: {
      snprintf(effectStr, sizeof(effectStr), "rotation");
      break;
    }
    case FILTER_DISTORTION: {
      snprintf(effectStr, sizeof(effectStr), "distortion");
      break;
    }
    case FILTER_CHANNELMIX: {
      snprintf(effectStr, sizeof(effectStr), "channelMix");
      break;
    }
    case FILTER_LOWPASS: {
      snprintf(effectStr, sizeof(effectStr), "lowPass");
      break;
    }
    case FILTER_REMOVE: {
      break;
    }
  }

  if (effect != FILTER_REMOVE) snprintf(payload, sizeof(payload), "{\"op\":\"filters\",\"guildId\":\"%"PRIu64"\",\"%s\":%s}", guildID, effectStr, value);
  else snprintf(payload, sizeof(payload), "{\"op\":\"filters\",\"guildId\":\"%"PRIu64"\"}", guildID);

  __coglink_sendPayload(lavaInfo, payload, sizeof(payload), "filters");  
}
