# Coglink

## About it

Coglink (not defined yet, name idea by `ap8650#5478`, AKA HackerSmacker), the firsts [Concord](https://github.com/Cogmasters/concord)'s specific Lavalink wrapper, still trying to be low-level, where you can control a lot of things, and still with the amazing Concord's function design.

## Why?

Coglink provides simple, still really customizable, functions for you to control whatever you want, and still with ease.
Also uses a lot of Concord's pre-installed libraries, being considered fully standard (as you would need Concord in any type of way to run it).

## Performance

Even without proof, it's fast, built with performance in mind, always. Any type of performance issue can be reported and will be fixed in the best way possible, but we aren't going to sacrifice stability to get more performance, don't worry.

## Status

Coglink can be used in production, what so ever it's not recommended for big bots since stability is the #1. Coglink is still in development and major updates are still expected to improve user experience. If you want to contribute, feel free to open a pull request. And would be awesome if you could let me know if you already made a bot with Coglink, please let me know at my Discord, `Pedro.js#9446`.

## Credits

This library wouldn't be possible without [Cogmaster](https://discord.gg/YcaK3puy49)'s guild members, they helped me all the way through, and special thanks to `müller#1001`, Concord's lead developer, without him, nothing of this would be possible, thank you! ❤️

Yep, me, the lead developer, or the single developer of this library, what so ever I got a lot of help from other people to make possible this library, thanks.

## Dependencies

* `Concord (master/dev)` 2.2.0
* `jsmn-find` included on Concord 2.2.0
* `jsmn` included on Concord 2.2.0
* `Concord's websocket` 2.2.0

*Tested on:* Ubuntu 22.04.1 x64

## Installation

This library is not available on any package manager, so you will need to compile it yourself, but don't worry, it's really easy.

*This library was tested on Linux Ubuntu only, modifications on Makefile to compile on other OSes are probably needed, but the code will PROBABLY not change.*

### Compiling

First, you will need to install the dependencies, you can do it with the following command (Debian based OSes):

```console
$ apt install -y make clang git
```

*This is supposed you already have Concord installed on your machine, this library was made and optimized for Concord.*

Then, you will clone this repository, and compile it:

```console
$ git clone https://github.com/ThePedroo/Coglink && cd Coglink && make && make install
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

## Documentation/Guides (Quick start)

We don't have documentation, but you can take on the guides and see what is the need since there will have everything briefly explained.

## Why this library? Why have I created it?

Concord, an awesome Discord wrapper, with not have the attention it should deserve, so I saw that no Lavalink wrappers were made for it, and neither were made with C, so since I already created a Lavalink powered bot with Concord, I decided that a lot of people should meet Concord, and also see how powerful C can be, blazingly fast and with its awesome low-level control.

I'm not an expert at C, but I do what I can do, I'll help if I'm able to, and if I can make Concord deserve what it should get, I'll do it.

I also know that this library is far away from being as famous as some JS libraries like erela.js, which was archived, but being famous here it's not the point, the point here is to help.

If you are currently using Coglink, please let me know, would be an honor to see people use this little project. :)
