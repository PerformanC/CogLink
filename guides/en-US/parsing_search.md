# Parsing song search

## Why parsing?

Parsing the search request is important since it will allow you to get the song name, the author, the duration, the thumbnail, the song URL, and much more. If you didn't, you won't be able to play the song.

## Parsing

WARNING: **THIS GUIDE WAS MADE FOR PARSING TRACKS.**

To parse the song search JSON, we will be using the `coglink_parseSearch` function, which will fill the last parameter with the information of the music you requested for it to parse.

For example, you will mostly want it to parse the first song, so follow the example above to parse the first song:

```c
struct lavaMusic music;

/*
  the second parameter, res, should be the struct httpRequest that you got from the coglink_searchSong function.
  the third parameter, songPos, should be the position of the song you want to parse, if you want to parse the first song, it should be 0.
  The last parameter, the library will fill with the information of the requested song.
*/
int parseRes = coglink_parseTrack(&lavaInfo, res, "0", &song);

if (parseRes != 0) {
  log_fatal("Error parsing song: %d", parseRes);
}

log_info("Song name: %s", song.name);
```

Done, now the `lavaSong` struct will be filled with the information of the song.

## Parsing + loop

Let's be fancy here, in case you want to parse all the 10 first song founds, you can use the for loop, see the example below:

```c
struct lavaSong song;

for (int i = 0; i < 10; i++) {
  char iString[16];
  snprintf(isString, sizeof(isString), "%d", i); 

  int parseRes = coglink_parseTrack(&lavaInfo, res, isString, &song);

  if (parseRes != 0) {
    log_fatal("Error parsing song: %d", parseRes);
  }

  log_info("Song name: %s", song.name);
}
```

Done, it will print the name of the first 10 songs found.

* *Guide made by*: @ThePedroo
