#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "websocket.h"

#include "utils.h"

size_t _coglink_write_cb(void *data, size_t size, size_t nmemb, void *userp) {
  size_t write_size = size * nmemb;
  struct coglink_response *mem = userp;

  char *ptr = realloc(mem->body, mem->size + write_size + 1);
  if (!ptr) {
    FATAL("[coglink:libcurl] Not enough memory to realloc.\n");
  }

  mem->body = ptr;
  memcpy(&(mem->body[mem->size]), data, write_size);
  mem->size += write_size;
  mem->body[mem->size] = 0;

  return write_size;
}

size_t _coglink_fake_write_cb(void *data, size_t size, size_t nmemb, void *userp) {
  (void)data; (void)size; (void)nmemb; (void)userp;

  return size * nmemb;
}

int _coglink_perform_request(struct coglink_node *node_info, struct coglink_request_params *req, struct coglink_response *res) {
  CURL *curl = curl_easy_init();

  size_t url_size = (node_info->ssl ? 1 : 0) + (sizeof("http://:") - 1) + (req->unversioned ? 0 : (sizeof("/v4") - 1)) + strlen(node_info->hostname) + 4 + strlen(req->endpoint);
  char *full_url = malloc(url_size + 1);

  url_size = snprintf(full_url, url_size + 1, "http%s://%s:%d%s%s", node_info->ssl ? "s" : "", node_info->hostname, node_info->port, req->unversioned ? "" : "/v4", req->endpoint);

  CURLcode c_res = curl_easy_setopt(curl, CURLOPT_URL, full_url);

  struct curl_slist *chunk = NULL;
  char *authorization = NULL;
    
  if (node_info->password) {
    authorization = malloc(17 + strlen(node_info->password) + 1);
    snprintf(authorization, 17 + strlen(node_info->password) + 1, "Authorization: %s", node_info->password);

    chunk = curl_slist_append(chunk, authorization);
  }

  if (req->body) {
    chunk = curl_slist_append(chunk, "Content-Type: application/json");

    c_res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->body);
    c_res = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, req->body_length);
  }

  chunk = curl_slist_append(chunk, "User-Agent: libcurl");

  c_res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
  c_res = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req->method);

  /* NodeLink compression compliance */
  c_res = curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

  if (req->get_response) {
    (*res).body = malloc(1);
    (*res).size = 0;

    c_res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _coglink_write_cb);
    c_res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)res);
  } else {
    /* avoids libcurl to write to stdout */
    c_res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _coglink_fake_write_cb);
  }

  c_res = curl_easy_perform(curl);
  if (c_res != CURLE_OK) {
    ERROR("[coglink:libcurl] curl_easy_perform failed: %s", curl_easy_strerror(c_res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);
    free(authorization);
    free(full_url);

    return COGLINK_FAILED;
  }

  curl_easy_cleanup(curl);
  curl_slist_free_all(chunk);
  free(authorization);
  free(full_url);
  
  DEBUG("[coglink:libcurl] Performed request successfully.");

  return COGLINK_SUCCESS;
}

