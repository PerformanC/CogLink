# Coglink

[![Discord Server](https://img.shields.io/discord/1036045973039890522?color=5865F2&logo=discord&logoColor=white)](https://discord.gg/YcaK3puy49) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/acbabb99b4354f5ab182e511dd35aee4)](https://www.codacy.com/gh/ThePedroo/Coglink/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=ThePedroo/Coglink&amp;utm_campaign=Badge_Grade)

## About

Coglink is a C99 Lavalink wrapper, with full coverage of all Lavalink functions and features, allowing you to easily connect to a Lavalink server and use Lavalink features.

The performance of Coglink isn't a thing you should be worried about, it provides high performance with a low memory footprint.

## Status

In Ubuntu, Arch Linux, Termux, if properly compiled, it is considered as stable, at least the common functions. Other OSes are not considered stable and needs testing. 

If any bug, please make an issue at GitHub and I'll try to fix ASAP.

## Credits

Even thought I'm the single maintainer of Coglink, I got a lot of help, thanks [Cogmaster](https://discord.gg/YcaK3puy49)'s guild members for this. And special thanks to `müller#1001`, Concord's lead developer, without him, nothing of this would be possible, thank you! ❤️

## Installation

This library is not available on any package manager, so you will need to compile it yourself, but don't worry, it's deadly easy.

*This library was compiled and tested (with no modifications to any file to work) in Ubuntu and Arch Linux, in FreeBSD and Termux requires you to change the PREFIX or include the /usr/local/..., in Termux for you need to change PREFIX and include it. This may be required to other OSes.*

### Compiling

First of all, you need to install the dependencies, you can do it with the following command:

```console
// Debian based OSes:
# apt install -y make clang git

// Arch linux:
# pacman -S make clang git

// FreeBSD/Termux:
# pkg install make clang git
```

Then, you need to clone the repository:

```console
$ git clone https://github.com/ThePedroo/Coglink
```

*Coglink was made with Concord to be used with Concord, so it's already expected to you have Concord installed.*

Then, after cloning the repository, you will need to compile it:

*Note that on FreeBSD systems, you will need to add to the compiler default directories /usr/local/..., since by default, it is not added into it on FreeBSD.*

```console
# cd Coglink && make && make install
```

The `make` command will compile Coglink files and the `make install` will include the library on your system, so you can use it on your projects.

### Using

To use Coglink, you will need to include it on your project, you can do it with the following command:

```c
#include <coglink/lavalink.h> // Websocket related Coglink functions
#include <coglink/definitions.h> // The definitions of defines like COGLINK_SUCCESS
#include <coglink/rest-lavalink.h> // Functions from Coglink that uses Lavalink rest API
#include <coglink/player.h> // The functions related to the music player
#include <coglink/miscellaneous.h> // Other functions, like decode track, get router planner and etc
```

And then, compile your project with the additional flag: `-lcoglink`.

## Documentation/Guides (Quickstart)

We don't have documentation, but you can take on the guides and see what is the need since there will have everything briefly explained.

## Why this library? Why have I created it?

Concord, an awesome Discord wrapper, with not have the attention it should deserve, so I saw that no Lavalink wrappers were made for it, and neither were made with C, so since I already created a Lavalink powered bot with Concord, I decided that a lot of people should meet Concord, and also see how powerful C can be, blazingly fast and with its awesome low-level control.

I'm not an expert at C, but I do what I can do, I'll help if I'm able to, and if I can make Concord deserve what it should get, I'll do it.

I also know that this library is far away from being as famous as some JS libraries like erela.js, which was archived, but being famous here it's not the point, the point here is to help.

If you are currently using Coglink, please let me know, would be an honor to see people use this little project. :)

## Support

If you need any support, feel absolutely free to ask for help on our [Discord server](https://discord.gg/uPveNfTuCJ). No one starts knowing everything, and we are here to help you. ^^

## Dependencies

* `libcurl` >= 7.56.1
* `Concord (master/dev)` 2.2.0
* `Concord's websocket` 2.2.0
* `jsmn-find` (included on Concord 2.2.0)
* `jsmn` (included on Concord 2.2.0)
* `chash` (included on Concord 2.2.0)

*Tested on:* Ubuntu 22.04.1 x64, Arch Linux x64, Termux Aarch64 (Android 12), Void Linux latest x64, FreeBSD 13.1 x64