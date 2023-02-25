#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include <coglink/lavalink-internal.h>
#include <coglink/lavalink.h>
#include <coglink/definitions.h>
#include <coglink/network.h>

int coglink_getRouterPlanner(struct lavaInfo *lavaInfo, struct requestInformation *res) {
  return __coglink_performRequest(lavaInfo, res, &(struct __coglink_requestConfig) {
                                                    .requestType = __COGLINK_GET_REQ,
                                                    .path = "/routeplanner/status",
                                                    .pathLength = 21,
                                                    .useVPath = true,
                                                    .getResponse = true
                                                  });
}

int coglink_parseRouterPlanner(struct lavaInfo *lavaInfo, struct requestInformation *res, char *ipPosition, struct lavalinkRouter *lavalinkRouterStruct) {
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

  jsmnf_pair *class = jsmnf_find(pairs, res->body, "class", 5);
  if (__coglink_checkParse(lavaInfo, class, "class") != COGLINK_PROCEED) return COGLINK_JSMNF_ERROR_FIND;

  snprintf(lavalinkRouterStruct->class, sizeof(lavalinkRouterStruct->class), "%.*s", (int)class->v.len, res->body + class->v.pos);

  if (lavalinkRouterStruct->class[0] == 'n') return COGLINK_ROUTERPLANNER_NOT_SET;

  char *path[] = { "details", "ipBlock", "type", NULL };
  jsmnf_pair *type = jsmnf_find_path(pairs, res->body, path, 3);

  path[2] = "size";
  jsmnf_pair *size = jsmnf_find_path(pairs, res->body, path, 3);

  path[1] = "failingAddresses";
  path[2] = ipPosition;
  path[3] = "address";
  jsmnf_pair *address = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "failingTimestamp";
  jsmnf_pair *failingTimestamp = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = "failingTime";
  jsmnf_pair *failingTime = jsmnf_find_path(pairs, res->body, path, 4);

  path[3] = NULL;
  path[2] = NULL;

  if (!type || !size || !address || !failingTimestamp || !failingTime) {
    if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !type ? "type" : !size ? "size" : !address ? "address" : !failingTimestamp ? "failingTimestamp" : !failingTime ? "failingTime" : "???");
    return COGLINK_JSMNF_ERROR_FIND;
  }

  if (lavalinkRouterStruct->class[0] == 'R') {
    path[1] = "rotateIndex";
    jsmnf_pair *rotateIndex = jsmnf_find_path(pairs, res->body, path, 2);

    path[1] = "ipIndex";
    jsmnf_pair *ipIndex = jsmnf_find_path(pairs, res->body, path, 2);

    path[1] = "currentAddress";
    jsmnf_pair *currentAddress = jsmnf_find_path(pairs, res->body, path, 2);

    if (!rotateIndex || !ipIndex || !currentAddress) {
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find %s field.", !rotateIndex ? "rotateIndex" : !ipIndex ? "ipIndex" : !currentAddress ? "currentAddress" : "???");
      return COGLINK_JSMNF_ERROR_FIND;
    }

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

    if (!currentAddressIndex) {
      if (lavaInfo->debugging->allDebugging || lavaInfo->debugging->parseErrorsDebugging || lavaInfo->debugging->jsmnfErrorsDebugging) log_fatal("[coglink:jsmnf-find] Error while trying to find currentAddressIndex field.");
      return COGLINK_JSMNF_ERROR_FIND;
    }

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

void coglink_getRouterPlannerCleanup(struct requestInformation *res) {
  free(res->body);
}

int coglink_freeFailingAddress(struct lavaInfo *lavaInfo, char *ip) {
  char payload[32];
  snprintf(payload, sizeof(payload), "{\"address\":\"%s\"}", ip);

  return __coglink_performRequest(lavaInfo, NULL, &(struct __coglink_requestConfig) {
                                                    .requestType = __COGLINK_POST_REQ,
                                                    .path = "/routeplanner/free/address",
                                                    .pathLength = 27,
                                                    .useVPath = true,
                                                    .body = payload,
                                                    .bodySize = strlen(payload)
                                                  });
}

int coglink_freeFailingAllAddresses(struct lavaInfo *lavaInfo) {
  return __coglink_performRequest(lavaInfo,  NULL, &(struct __coglink_requestConfig) {
                                                    .requestType = __COGLINK_POST_REQ,
                                                    .path = "/routeplanner/free/all",
                                                    .pathLength = 23,
                                                    .useVPath = true
                                                  });
}
