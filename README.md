# Coglink

## About it

Coglink (not defined yet, name idea by `ap8650#5478`, AKA HackerSmacker), the firsts [Concord](https://github.com/Cogmasters/concord)'s specific Lavalink wrapper, still trying to be low-level, where you can control a lot of things, and still with the amazing Concord's function design.

## Why?

Coglink provides simple, still really customizable, functions for you to control whatever you want, and still with ease.
Also uses a lot of Concord's pre-installed libraries, being considered fully standard (as you would need Concord in any type of way to run it).

## Performance

Even without proof, it's fast, built with performance in mind, always. Any type of performance issue can be reported and will be fixed in the best way possible, but we aren't going to sacrifice stability to get more performance, don't worry.

## Status

Sadly, it's not recommended for production for now, in some days, it's expected to get more functions and better stability, for now, you can use it to have some fun, but **DO NOT USE IT IN PRODUCTION**.

## Credits

This library wouldn't be possible without [Cogmaster](https://discord.gg/YcaK3puy49)'s guild members, they helped me all the way through, and special thanks to `müller#1001`, Concord's lead developer, without him, nothing of this would be possible, thank you! ❤️

## Dependencies

* `Concord (master/dev)` 2.2.0
* `jsmn-find` included on Concord 2.2.0
* `jsmn` included on Concord 2.2.0
* `Concord's websocket` 2.2.0
* `libcurl` latest
* `libssl` latest
* `libcrypto` latest
* `libpthread` latest

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
```

And then, compile your project with the additional flag: `-lcoglink`.

## Documentation/Guides (Quick start)

We don't have a documentation, but you can take on the guides and see what is the need since there will have everything briefly explained.