#ifndef LAVALINK_INTERNAL_H
#define LAVALINK_INTERNAL_H

#include <curl/curl.h>

#include <concord/jsmn.h>
#include <concord/jsmn-find.h>

struct coglink_lavaInfo;
struct coglink_requestInformation;
struct coglink_nodeInfo;
struct io_poller;

#define __COGLINK_GET_REQ 0
#define __COGLINK_POST_REQ 1
#define __COGLINK_DELETE_REQ 2
#define __COGLINK_PATCH_REQ 3

struct __coglink_requestConfig {
  int requestType;
  int additionalDebuggingSuccess;
  int additionalDebuggingError;
  char *path;
  int pathLength;
  int useVPath;
  char *body;
  long bodySize;
  int getResponse;
  CURL *usedCURL;
};

int _coglink_performRequest(struct coglink_lavaInfo *lavaInfo, struct coglink_nodeInfo *nodeInfo, struct coglink_requestInformation *res, struct __coglink_requestConfig *config);

int _coglink_checkParse(struct coglink_lavaInfo *lavaInfo, jsmnf_pair *field, char *fieldName);

int _coglink_selectBestNode(struct coglink_lavaInfo *lavaInfo);

int _coglink_findNode(struct coglink_lavaInfo *lavaInfo, char *hostname);

int _coglink_IOPoller(struct io_poller *io, CURLM *multi, void *user_data);

struct coglink_parsedTrack _coglink_buildTrackStruct(struct coglink_lavaInfo *lavaInfo, jsmnf_pair pairs[], const char *text);

#endif
