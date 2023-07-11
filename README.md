# Coglink

[![Discord Server](https://img.shields.io/discord/1036045973039890522?color=5865F2&logo=discord&logoColor=white)](https://discord.gg/YcaK3puy49) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/acbabb99b4354f5ab182e511dd35aee4)](https://www.codacy.com/gh/ThePedroo/Coglink/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=ThePedroo/Coglink&amp;utm_campaign=Badge_Grade)

![Coglink logo](guides/images//CoglinkLogo.png "Coglink logo")

Coglink, the C Lavalink client, with the most performance above all Lavalink wrappers.

## About

Coglink is a C99 Lavalink client, which has full coverage of Lavalink API, allowing easy low-level control of LavaLink.

Made to be portable, Coglink is the least resource-consuming client, allowing you to run easily anywhere. (It can use less than 10MB of RAM)

## Why?

One of the main purposes of Coglink is to be the most performant LavaLink client, while still being light-weight and portable, especially with a low overhead.

In comparison with other NodeJs clients, even against FastLink, Coglink is way more portable, which makes it a good option for big bots, which needs to have the least resource consumption possible.

Coglink is from PerformanC, and our missions is to make our softwares stable enough that it could survive months without issues, and while this is hard for a big project, we are trying to make it possible, which for now, it's working.

Performance, stability and portability, that's what Coglink is about.

## Stability

Stability on this branch cannot be confirmed, LavaLink v4 is still in development with some pending features to be added.

While this branch is more stable than the main, LavaLink is not, so be aware when using this branch.

*If marked, considered stable, if not, bugs may be found.*

- [x] Search (Highly stable)
- [x] loadType/track/playlist/error parsing (loadType/track highly stable)
- [x] Play (Highly stable)
- [x] Join voice channel (Highly stable)
- [x] All cleanups (Highly stable)
- [x] DecodeTrack(s) & ParseTrack(s)
- [x] Parse track, pause track, stop track, seek track, set volume
- [x] Set filter
- [x] Get Players & parse get Players
- [x] Get/parse Info
- [ ] Get/parse RouterPlanner
- [x] Get/parse Lavalink Stats (Event included)
- [x] Get Lavalink Version
- [x] Connect/disconnect Node (Highly stable)
- [x] Set event (Highly stable)
- [x] Websocket (**Known bugs with close event**)
- [x] IO poller (Highly stable)

## Compiling

Before starting, you will need to install the required libraries for Coglink, this can be done by the package manager of your OS, you will need to install the mentioned libraries below:

```text
make clang git
```

**Warning**: Remember that their names can change depending on your OS.

After installing the libraries, you will need to [compile Concord](https://github.com/Cogmasters/concord).

After compiling Concord, you can proceed with the installation of Coglink, first, we will need to clone the repository:

```console
$ git clone https://github.com/PerformanC/Coglink
```

When the repository is cloned, enter in the folder and run the `make` command, this will generate the header files, which will be used to compile Coglink, and after it, install into the system.

```console
# cd Coglink && make && make install
```

**Warning**: When compiling your bot, remember to use the `-lcoglink` flag.

## Using

See below the purpose of Coglink header files:

```c
#include <coglink/lavalink.h> // Websocket related Coglink functions
#include <coglink/definitions.h> // File with the definitions of macros, like COGLINK_SUCCESS
#include <coglink/information.h> // Functions which get information from Lavalink
#include <coglink/network.h> // Functions related to network, like the router planner
#include <coglink/player.h> // The functions related to the music player
#include <coglink/plugins.h> // For the use of plugins
#include <coglink/track.h> // Functions related to tracks, like decoding, searching, etc
```

As for the usage, you can see the `guides/example` folder, with a complete example of Coglink usage.

## Documentation

You can see [here](https://performanc.github.io/CoglinDocs/) the documentation for Coglink, made with Doxygen.

In case you want to build the documentation yourself, you can use the `make gen_docs` command, but be sure that you have Doxygen installed.

## Coglink plugins

For most functions, Coglink allows plugins to interfere with it, allowing the community to add more features to Coglink, without the need to modify the source code, but for the protection of the users, Coglink has some security measures to avoid malicious plugins.

The security measures are:

- Plugins are not allowed to change the members of the struct `lavaInfo`/`client`
- Concord's client struct has hidden members (OPTIONAL)
- Plugins are not allowed to read neither the bot token nor the Concord websocket struct by default.

### Setting up a plugin

Plugins can't straight ahead work in Coglink, you need to pass the functions to Coglink from the plugin, so it can execute when some function from Coglink is used, see the function below that allow this to happen:

```c
#include <coglink/plugins.h>
#include "somePlugin.h"

struct coglink_pluginEvents event = {
  .onSearchRequest = &functionToBeExecuted
}

coglink_setPluginEvents(&lavaInfo, &event);
```

Done, now when the function coglink_searchSong is executed, this will be executed first before Coglink processes it.

**Warning**: The first plugins have the priority, so if you want to make a plugin that will be executed first, you will need to set it first.

## Plugin list

For now, there are no plugins for Coglink, but if you want to make one, you can see the [Setting up a plugin](#setting-up-a-plugin) section, and after it, you can add it to the list below.

## Support

In case of any issue using it (except bugs, that should be reported on GitHub Issues), you are free to ask on PerformanC's [Discord server](https://discord.gg/uPveNfTuCJ).

## Credits

Although Coglink is made only by one person, many people contributed to it, thanks [Cogmaster](https://discord.gg/YcaK3puy49) members for this. And special thanks to `müller#1001`, without you, we wouldn't be here.

Some people that helped on Coglink related things:

- müller
- HackerSmacker
- Goo

Thanks you all for the help! ^^

## Dependencies

- `libcurl` >= 7.56.1
- `Concord (master/dev)` 2.2.0
- `jsmn-find` latest (included on Concord 2.2.0)
- `jsmn` latest (included on Concord 2.2.0)
- `tablec` v2.2.3 (Open-addressing)
