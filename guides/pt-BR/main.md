# Lavalink

## O que é isso?

[Lavalink](https://github.com/freyacodes/Lavalink) é um sistema de processamento de áudio poderoso, que permite que você toque áudios de vídeos do YouTube no Discord. Lavalink nodes não são totalmente conectadas diretamente com seu BOT, elas podem ser hosteadas em outas máquinas sem problemas, e você pode ter várias nodes em uma máquina ou em várias, providenciando melhor estabilidade para BOTs grandes, não deixando uma node específica sobrecarregar e causar dela tomar rate limit.

## Por quê?

Mesma sendo lenta e pesada, a Lavalink é uma opção fácil para tocar vídeos do YouTube no Discord, já que fazer isso manualmente daria muito trabalho para pessoas que já não tem uma base, e ela providencia efeitos e muitos outras funções que seria difíceis de serem adicionadas manualmente. Ela também providencia uma maneira fácil de tocar músicas de outras fontes, tipo, SoundCloud, Bandcamp, Twitch, Vimeo, Mixer, e mais.

## Como?

A Lavalink é feita com Java, junto com Kotlin. Quando você da download nos seus binários, e executa eles, eles vão criar um servidor WebSocket](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API) na porta 2333 (só caso você não trocar a porta no arquivo [`application.yml`](https://github.com/freyacodes/Lavalink/blob/master/LavalinkServer/application.yml.example)), que você irá conseguir acessar com um cliente WebSocket](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API), por exemplo, o cliente [WebSocket](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API) do [Concord](https://github.com/Cogmasters/concord). Tem várias livrarias que conectam na Lavalink node e providencia à você funções para gerenciar a Lavalink, tipo o Coglink.

## Concord? Por que eu deveria usar o Concord invés de outras livrarias?

Concord é uma livraria assincrónica do [Discord](https://discord.com) usando o C99, providenciando funções low-level para contrar seu bot com elas, e mesmo assim, providenciando uma performance alta, não perdendo as estruturas originais das repostas da Discord API, fazendo ser fácil de se manter, ajeitar, melhor, atualizar, e muito mais.

* *Guia por*: @ThePedroo
* Feito com amor por um brasileiro! ❤️
