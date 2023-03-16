# Applying effects

## Effects? Which effects?

The Lavalink allows you to add effects into your tracks, of all kinds, allowing you to change the bands, allowing effects like karaoke, nightcore, bassbost and etc.

## Adding effects

In this guide, the example of how to change the rotation effect will be used.

Coglink have the `coglink_setEffect` functions, that you can use to change all effects that Lavalink have, your use is deadly easy, see the example below:

```c
coglink_setEffect(&lavaInfo, message->guild_id, COGLINK_FILTER_ROTATION, "{\"rotationHz\":0.2}");
```

Hm? "Where is the rest of the code"? Yep, it's only this, incredible, isn't it?

This code makes the famous "8d" effect in your track, easy, and even cool, don't you think?

## Removendo efeitos

The last topic was about how to add an effect, but how about remove it?

The file `definitions.h` of Coglink offers you a bunch of macros to be used, one of them is `FILTER_REMOVE`, which defines the value to the functions of Coglink understand that you want to remove the effect. 

See the example below:

```c
coglink_setEffect(&lavaInfo, message->guild_id, COGLINK_FILTER_REMOVE, NULL);
```

And done! You will have the effect remove, in case you have added one.

* *Guide made by*: @ThePedroo
