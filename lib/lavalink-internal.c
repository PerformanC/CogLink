#include <stdio.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>

#include <coglink/lavalink.h>
#include <coglink/definations.h>

void __coglink_sendPayload(struct lavaInfo *lavaInfo, char payload[], char *payloadOP) {
  if (ws_send_text(lavaInfo->ws, NULL, payload, strlen(payload)) == false) {
    if (lavaInfo->debug) log_fatal("[coglink:libcurl] Something went wrong while sending a payload with op %s to Lavalink.", payloadOP);
    return;
  } else {
    if (lavaInfo->debug) log_debug("[coglink:libcurl] Successfully sent a payload with op %s to Lavalink.", payloadOP);
    ws_easy_run(lavaInfo->ws, 5, &lavaInfo->tstamp);
  }
}

int __coglink_checkParse(struct lavaInfo *lavaInfo, jsmnf_pair *field, char *fieldName) {
  if (!field) {
    if (lavaInfo->debug) log_error("[coglink:jsmn-find] Failed to find %s field.", fieldName);
    return COGLINK_JSMNF_ERROR_FIND;
  } else {
    return COGLINK_PROCEED;
  }
}