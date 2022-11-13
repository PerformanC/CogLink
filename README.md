# Coglink

## About

Coglink is a C99 Lavalink wrapper, with semi-full coverage of all Lavalink functions and features, allowing you to easily connect to a Lavalink server and use Lavalink features.

The performance of Coglink isn't a thing you should be worried about, it provides high performance with a low memory footprint.

## Status

While Coglink isn't fully stable, you can still use it in private bots and such, but I wouldn't recommend using it in a public bot since it still has many bugs that could be capable of causing a segmentation fault.

## Credits

Even thought I'm the single maintainer of Coglink, I got a lot of help, thanks [Cogmaster](https://discord.gg/YcaK3puy49)'s guild members for this. And special thanks to `müller#1001`, Concord's lead developer, without him, nothing of this would be possible, thank you! ❤️

## Installation

This library is not available on any package manager, so you will need to compile it yourself, but don't worry, it's really easy.

*This library was tested on Linux Ubuntu only, modifications on Makefile to compile on other OSes are probably needed, but the code will PROBABLY not change.*

### Compiling

First of all, you need to install the dependencies, you can do it with the following command:

```console
// Debian based OSes:
# apt install -y make clang git

// Arch linux:
# pacman -S make clang git
```

Then, you need to clone the repository:

```console
$ git clone https://github.com/ThePedroo/Coglink
```

*Coglink was made with Concord to be used with Concord, so it's already expected to you have Concord installed.*

Then, after cloning the repository, you will need to compile it:

```console
$ cd Coglink && make && make install
```

The `make` command will compile Coglink files and the `make install` will include the library on your system, so you can use it on your projects.

### Using

To use Coglink, you will need to include it on your project, you can do it with the following command:

```c
#include <coglink/lavalink.h> // Websocket related Coglink functions
#include <coglink/definitions.h> // The definitions of defines like COGLINK_SUCCESS
#include <coglink/rest-lavalink.h> // Functions from Coglink that uses Lavalink rest API
#include <coglink/player.h> // The functions related to the music player
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

* `Concord (master/dev)` 2.2.0
* `jsmn-find` included on Concord 2.2.0
* `jsmn` included on Concord 2.2.0
* `Concord's websocket` 2.2.0

*Tested on:* Ubuntu 22.04.1 x64, Arch Linux x64