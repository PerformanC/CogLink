#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>
#include <coglink/network.h>

int coglink_getRouterPlanner(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, struct coglink_requestInformation *res) {
  int node = _coglink_findPlayerNode(guildId);

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], res, 
                                 &(struct __coglink_requestConfig) {
                                   .requestType = __COGLINK_GET_REQ,
                                   .path = "/routeplanner/status",
                                   .pathLength = 21,
                                   .getResponse = 1
                                 });
}

int coglink_parseRouterPlanner(struct coglink_lavaInfo *lavaInfo, struct coglink_requestInformation *res, char *ipPosition, struct coglink_lavalinkRouter *lavalinkRouterStruct) {
  jsmn_parser parser;
  jsmntok_t tokens[128];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, res->body, res->size, tokens, sizeof(tokens));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to parse JSON.");
    return COGLINK_JSMNF_ERROR_PARSE;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[jsmn-find] Successfully parsed JSON.");

  jsmnf_loader loader;
  jsmnf_pair pairs[128];

  jsmnf_init(&loader);
  r = jsmnf_load(&loader, res->body, tokens, parser.toknext, pairs, sizeof(pairs) / sizeof(*pairs));

  if (r < 0) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_error("[coglink:jsmn-find] Failed to load jsmn-find.");
    return COGLINK_JSMNF_ERROR_LOAD;
  }
  if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfSuccessDebugging) log_debug("[coglink:jsmn-find] Successfully loaded jsmn-find.");

  jsmnf_pair *class = jsmnf_find(pairs, res->body, "class", sizeof("class") - 1);
  if (_coglink_checkParse(lavaInfo, class, "class") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  snprintf(lavalinkRouterStruct->class, sizeof(lavalinkRouterStruct->class), "%.*s", (int)class->v.len, res->body + class->v.pos);

  if (lavalinkRouterStruct->class[0] == 'n') return COGLINK_ROUTERPLANNER_NOT_SET;

  char *path[] = { "details", "ipBlock", "type", NULL };
  jsmnf_pair *type = jsmnf_find_path(pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, type, "type") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[2] = "size";
  jsmnf_pair *size = jsmnf_find_path(pairs, res->body, path, 3);
  if (_coglink_checkParse(lavaInfo, size, "size") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[1] = "failingAddresses";
  path[2] = ipPosition;
  path[3] = "address";
  jsmnf_pair *address = jsmnf_find_path(pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, address, "address") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[3] = "failingTimestamp";
  jsmnf_pair *failingTimestamp = jsmnf_find_path(pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, failingTimestamp, "failingTimestamp") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  path[3] = "failingTime";
  jsmnf_pair *failingTime = jsmnf_find_path(pairs, res->body, path, 4);
  if (_coglink_checkParse(lavaInfo, failingTime, "failingTime") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  if (!type || !size || !address || !failingTimestamp || !failingTime) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !type ? "type" : !size ? "size" : !address ? "address" : !failingTimestamp ? "failingTimestamp" : !failingTime ? "failingTime" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  if (lavalinkRouterStruct->class[0] == 'R') {
    path[1] = "rotateIndex";
    jsmnf_pair *rotateIndex = jsmnf_find_path(pairs, res->body, path, 2);
    if (_coglink_checkParse(lavaInfo, rotateIndex, "rotateIndex") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_FIND;

    path[1] = "ipIndex";
    jsmnf_pair *ipIndex = jsmnf_find_path(pairs, res->body, path, 2);
    if (_coglink_checkParse(lavaInfo, ipIndex, "ipIndex") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_FIND;

    path[1] = "currentAddress";
    jsmnf_pair *currentAddress = jsmnf_find_path(pairs, res->body, path, 2);
    if (_coglink_checkParse(lavaInfo, currentAddress, "currentAddress") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_FIND;

    snprintf(lavalinkRouterStruct->details->ipBlock->type, sizeof(lavalinkRouterStruct->details->ipBlock->type), "%.*s", (int)type->v.len, res->body + type->v.pos);
    snprintf(lavalinkRouterStruct->details->ipBlock->size, sizeof(lavalinkRouterStruct->details->ipBlock->size), "%.*s", (int)size->v.len, res->body + size->v.pos);
    snprintf(lavalinkRouterStruct->details->failingAddress->address, sizeof(lavalinkRouterStruct->details->failingAddress->address), "%.*s", (int)address->v.len, res->body + address->v.pos);
    snprintf(lavalinkRouterStruct->details->failingAddress->failingTimestamp, sizeof(lavalinkRouterStruct->details->failingAddress->failingTimestamp), "%.*s", (int)failingTimestamp->v.len, res->body + failingTimestamp->v.pos);
    snprintf(lavalinkRouterStruct->details->failingAddress->failingTime, sizeof(lavalinkRouterStruct->details->failingAddress->failingTime), "%.*s", (int)failingTime->v.len, res->body + failingTime->v.pos);
    snprintf(lavalinkRouterStruct->details->rotateIndex, sizeof(lavalinkRouterStruct->details->rotateIndex), "%.*s", (int)rotateIndex->v.len, res->body + rotateIndex->v.pos);
    snprintf(lavalinkRouterStruct->details->ipIndex, sizeof(lavalinkRouterStruct->details->ipIndex), "%.*s", (int)ipIndex->v.len, res->body + ipIndex->v.pos);
    snprintf(lavalinkRouterStruct->details->currentAddress, sizeof(lavalinkRouterStruct->details->currentAddress), "%.*s", (int)currentAddress->v.len, res->body + currentAddress->v.pos);
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> class: %s\n> type: %s\n> size: %s\n> address: %s\n> failingTimestamp: %s\n> failingTime: %s\n> rotateIndex: %s\n> ipIndex: %s\n> currentAddress: %s\n", lavalinkRouterStruct->class, lavalinkRouterStruct->details->ipBlock->type, lavalinkRouterStruct->details->ipBlock->size, lavalinkRouterStruct->details->failingAddress->address, lavalinkRouterStruct->details->failingAddress->failingTimestamp, lavalinkRouterStruct->details->failingAddress->failingTime, lavalinkRouterStruct->details->rotateIndex, lavalinkRouterStruct->details->ipIndex, lavalinkRouterStruct->details->currentAddress);
  } else if (lavalinkRouterStruct->class[0] == 'N') {
    path[1] = "currentAddressIndex";
    jsmnf_pair *currentAddressIndex = jsmnf_find_path(pairs, res->body, path, 2);
    if (_coglink_checkParse(lavaInfo, currentAddressIndex, "currentAddressIndex") != COGLINK_SUCCESS) return COGLINK_JSMNF_ERROR_FIND;

    snprintf(lavalinkRouterStruct->details->ipBlock->type, sizeof(lavalinkRouterStruct->details->ipBlock->type), "%.*s", (int)type->v.len, res->body + type->v.pos);
    snprintf(lavalinkRouterStruct->details->ipBlock->size, sizeof(lavalinkRouterStruct->details->ipBlock->size), "%.*s", (int)size->v.len, res->body + size->v.pos);
    snprintf(lavalinkRouterStruct->details->failingAddress->address, sizeof(lavalinkRouterStruct->details->failingAddress->address), "%.*s", (int)address->v.len, res->body + address->v.pos);
    snprintf(lavalinkRouterStruct->details->failingAddress->failingTimestamp, sizeof(lavalinkRouterStruct->details->failingAddress->failingTimestamp), "%.*s", (int)failingTimestamp->v.len, res->body + failingTimestamp->v.pos);
    snprintf(lavalinkRouterStruct->details->failingAddress->failingTime, sizeof(lavalinkRouterStruct->details->failingAddress->failingTime), "%.*s", (int)failingTime->v.len, res->body + failingTime->v.pos);
    snprintf(lavalinkRouterStruct->details->currentAddressIndex, sizeof(lavalinkRouterStruct->details->currentAddressIndex), "%.*s", (int)currentAddressIndex->v.len, res->body + currentAddressIndex->v.pos);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> class: %s\n> type: %s\n> size: %s\n> address: %s\n> failingTimestamp: %s\n> failingTime: %s\n> currentAddressIndex: %s\n", lavalinkRouterStruct->class, lavalinkRouterStruct->details->ipBlock->type, lavalinkRouterStruct->details->ipBlock->size, lavalinkRouterStruct->details->failingAddress->address, lavalinkRouterStruct->details->failingAddress->failingTimestamp, lavalinkRouterStruct->details->failingAddress->failingTime, lavalinkRouterStruct->details->currentAddressIndex);
  } else {
    path[1] = "blockIndex";
    jsmnf_pair *blockIndex = jsmnf_find_path(pairs, res->body, path, 2);

    path[1] = "currentAddressIndex";
    jsmnf_pair *currentAddressIndex = jsmnf_find_path(pairs, res->body, path, 2);

    if (!currentAddressIndex) {
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find currentAddressIndex field.");
      return COGLINK_JSMNF_ERROR_FIND;
    }

    snprintf(lavalinkRouterStruct->details->ipBlock->type, sizeof(lavalinkRouterStruct->details->ipBlock->type), "%.*s", (int)type->v.len, res->body + type->v.pos);
    snprintf(lavalinkRouterStruct->details->ipBlock->size, sizeof(lavalinkRouterStruct->details->ipBlock->size), "%.*s", (int)size->v.len, res->body + size->v.pos);
    snprintf(lavalinkRouterStruct->details->failingAddress->address, sizeof(lavalinkRouterStruct->details->failingAddress->address), "%.*s", (int)address->v.len, res->body + address->v.pos);
    snprintf(lavalinkRouterStruct->details->failingAddress->failingTimestamp, sizeof(lavalinkRouterStruct->details->failingAddress->failingTimestamp), "%.*s", (int)failingTimestamp->v.len, res->body + failingTimestamp->v.pos);
    snprintf(lavalinkRouterStruct->details->failingAddress->failingTime, sizeof(lavalinkRouterStruct->details->failingAddress->failingTime), "%.*s", (int)failingTime->v.len, res->body + failingTime->v.pos);
    snprintf(lavalinkRouterStruct->details->blockIndex, sizeof(lavalinkRouterStruct->details->blockIndex), "%.*s", (int)blockIndex->v.len, res->body + blockIndex->v.pos);
    snprintf(lavalinkRouterStruct->details->currentAddressIndex, sizeof(lavalinkRouterStruct->details->currentAddressIndex), "%.*s", (int)currentAddressIndex->v.len, res->body + currentAddressIndex->v.pos);

    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseSuccessDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_debug("[coglink:jsmn-find] Parsed error search json, results:\n> class: %s\n> type: %s\n> size: %s\n> address: %s\n> failingTimestamp: %s\n> failingTime: %s\n> blockIndex: %s\n> currentAddressIndex: %s\n", lavalinkRouterStruct->class, lavalinkRouterStruct->details->ipBlock->type, lavalinkRouterStruct->details->ipBlock->size, lavalinkRouterStruct->details->failingAddress->address, lavalinkRouterStruct->details->failingAddress->failingTimestamp, lavalinkRouterStruct->details->failingAddress->failingTime, lavalinkRouterStruct->details->blockIndex, lavalinkRouterStruct->details->currentAddressIndex);
  }

  return COGLINK_SUCCESS;
}

void coglink_getRouterPlannerCleanup(struct coglink_requestInformation *res) {
  free(res->body);
}

int coglink_freeFailingAddress(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId, char *ip) {
  int node = _coglink_findPlayerNode(guildId);

  char payload[16];
  int payloadLen = snprintf(payload, sizeof(payload), "{\"address\":\"%s\"}", ip);

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL, 
                                 &(struct __coglink_requestConfig) {
                                   .requestType = __COGLINK_POST_REQ,
                                   .path = "/routeplanner/free/address",
                                   .pathLength = 27,
                                   .body = payload,
                                   .bodySize = payloadLen
                                 });
}

int coglink_freeFailingAllAddresses(struct coglink_lavaInfo *lavaInfo, u64snowflake guildId) {
  int node = _coglink_findPlayerNode(guildId);

  return _coglink_performRequest(lavaInfo, &lavaInfo->nodes[node], NULL,
                                  &(struct __coglink_requestConfig) {
                                    .requestType = __COGLINK_POST_REQ,
                                    .path = "/routeplanner/free/all",
                                    .pathLength = 23,
                                    .useVPath = 1
                                  });
}
