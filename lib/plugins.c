#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/plugins.h>

void coglink_setPluginEvents(struct lavaInfo *lavaInfo, struct pluginEvents *pluginEvents) {
  if (pluginEvents->onSearchRequest) lavaInfo->plugins->events->onSearchRequest[lavaInfo->plugins->amount] = pluginEvents->onSearchRequest;
  if (pluginEvents->onPlayRequest) lavaInfo->plugins->events->onPlayRequest[lavaInfo->plugins->amount] = pluginEvents->onPlayRequest;
  if (pluginEvents->onLavalinkPacketReceived) lavaInfo->plugins->events->onLavalinkPacketReceived[lavaInfo->plugins->amount] = pluginEvents->onLavalinkPacketReceived;
  if (pluginEvents->onLavalinkClose) lavaInfo->plugins->events->onLavalinkClose[lavaInfo->plugins->amount] = pluginEvents->onLavalinkClose;
  if (pluginEvents->onCoglinkScheduler) lavaInfo->plugins->events->onCoglinkScheduler[lavaInfo->plugins->amount] = pluginEvents->onCoglinkScheduler;
  if (pluginEvents->onDecodeTrackRequest) lavaInfo->plugins->events->onDecodeTrackRequest[lavaInfo->plugins->amount] = pluginEvents->onDecodeTrackRequest;
  if (pluginEvents->onDecodeTrackParseRequest) lavaInfo->plugins->events->onDecodeTrackParseRequest[lavaInfo->plugins->amount] = pluginEvents->onDecodeTrackParseRequest;
  if (pluginEvents->onDecodeTracksRequest) lavaInfo->plugins->events->onDecodeTracksRequest[lavaInfo->plugins->amount] = pluginEvents->onDecodeTracksRequest;
  if (pluginEvents->onDecodeTracksParseRequest) lavaInfo->plugins->events->onDecodeTracksParseRequest[lavaInfo->plugins->amount] = pluginEvents->onDecodeTracksParseRequest;
}
