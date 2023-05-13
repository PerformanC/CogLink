# Parsing song search

## Why parsing?

Parsing the search request is important since it will allow you to get the song name, the author, the duration, the thumbnail, the song URL, and much more. If you didn't, you won't be able to play the song.

## Parsing

To parse the song search JSON, we will be using the `coglink_parseSearch` function, which will fill the last parameter with the information of the music you requested for it to parse.

For example, you will mostly want it to parse the first song, so follow the example above to parse the first song:

```c
struct coglink_parsedTrackStruct parsedStruct;
coglink_initParseTrack(&lavaInfo, &parsedStruct, &res);

struct parsedTrack song;
int parseRes = coglink_parseTrack(&lavaInfo, &parsedStruct, &res, "0", &song);

if (parseRes != 0) 
  log_fatal("Error parsing song: %d", parseRes);

log_info("Song name: %s", song->name);
```

Done, now the `parsedTrack` struct will be filled with the information of the song.

## Parsing + loop

Let's be fancy here, in case you want to parse all the 10 first song founds, you can use the for loop, see the example below:

```c
struct parsedTrack song;

for (int i = 0; i < 10; i++) {
  char iString[16];
  char iString[16];
  snprintf(iString, sizeof(iString), "%d", i); 

  int parseRes = coglink_parseTrack(&lavaInfo, &parsedStruct, &res, iString, &song);

  if (parseRes != 0)
    log_fatal("Error parsing song: %d", parseRes);

  log_info("Song name: %s", song->name);
}
```

Done, it will print the name of the first 10 songs found.

* *Guide made by*: @ThePedroo
