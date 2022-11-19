#include <string.h>

#include <concord/discord.h>
#include <concord/discord-internal.h>

#include <coglink/lavalink.h>
#include <coglink/definitions.h>

void __coglink_sendPayload(struct lavaInfo *lavaInfo, char payload[], char *payloadOP) {
  if (ws_send_text(lavaInfo->ws, NULL, payload, strnlen(payload)) == false) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadErrorsDebugging) log_fatal("[coglink:libcurl] Something went wrong while sending a payload with op %s to Lavalink.", payloadOP);
    return;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->sendPayloadSuccessDebugging) log_debug("[coglink:libcurl] Successfully sent a payload with op %s to Lavalink.", payloadOP);
    ws_easy_run(lavaInfo->ws, 5, &lavaInfo->tstamp);
  }
}

int __coglink_checkParse(struct lavaInfo *lavaInfo, jsmnf_pair *field, char *fieldName) {
  if (!field) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->checkParseErrorsDebugging) log_error("[coglink:jsmn-find] Failed to find %s field.", fieldName);
    return COGLINK_JSMNF_ERROR_FIND;
  } else {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->checkParseSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully found %s field.", fieldName);
    return COGLINK_PROCEED;
  }
}
