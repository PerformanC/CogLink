# Lavalink

## What is it?

[Lavalink](https://github.com/freyacodes/Lavalink) is a powerful audio processing system, which allows you to play audio from videos of YouTube into Discord. Lavalink nodes are not fully connected directly to your bot, they can be run in another machine with no problem, and you can have multiple nodes either in the same machine or multiple machines, providing better stability for big bots, not overloading one single node and causing it to rate limit.

## Why?

Besides being slow and heavy, Lavalink is an easy option for playing YouTube videos in Discord, since doing it manually would require a lot of work for people who don't already have a base, and it provides effects and a lot of other features that would be hard to implement manually. It also provides an easy way to play songs from other sources, like SoundCloud, Bandcamp, Twitch, Vimeo, Mixer, and more.

## How?

Lavalink is powered by Java, together with Kotlin, when you download its binaries and run it, it will start a [WebSocket](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API) server on port 2333 (unless you change the port in [`application.yml`](https://github.com/freyacodes/Lavalink/blob/master/LavalinkServer/application.yml.example)) which you will be able to connect via a [WebSocket](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API) client, for example, [Concord](https://github.com/Cogmasters/concord)'s [WebSocket](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API) client, there are a lot of libraries which already connects into the Lavalink node and give you the functions to easily work with the Lavalink, like Coglink.

## Concord? Why should I use Concord instead of other libraries?

Concord is an asynchronous C99 [Discord](https://discord.com) API wrapper, providing low-level functions to control your bot with it, and still providing high performance, not losing the original structures of the Discord API responses, making it easy to maintain, fix, improve, update and much more.

* *Guide made by*: @ThePedroo
