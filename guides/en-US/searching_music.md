# Searching song

## Search from WHERE?

Lavalink allows you to search from a lot of sources, and in this guide, you will be able to customize the search to search from a specific source, or from all sources.

## Searching

For searching for songs, we will be using the `coglink_searchSong` function, it will fill the last parameter, `struct requestInformation *` with the body (JSON) of the Lavalink response.

### Why Coglink doesn't parse it automatically

Coglink was made with performance in mind, if it was parsing the whole JSON, it would cause a lot of performance loss, so we decided to create a function that you can choose to select to parse the first, second, and third on any song in that position, so it will be faster and better.

Follow the example above of the `coglink_searchSong` function:

```c
struct requestInformation res;

int resStatus = coglink_searchSong(&lavaInfo, "Very ordinary life", &res);

if (resStatus != 0) {
  log_fatal("Error searching song: %d", resStatus);
}

log_debug("Body response of Lavalink: %s", res.body);
    
/* ... */

coglink_searchCleanup(res);
```

Done, it will print the response of the Lavalink search request, but in case it fails, it will return the int value of what happened wrong.

You can proceed following the next guide, the [`parsing search song`](/guides/en-US/parsing_search.md) guide.

* *Guide made by*: @ThePedroo
