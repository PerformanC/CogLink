# Aplicando efeitos

## Efeitos? Quais efeitos?

A Lavalink permite que você adicione efeitos nas suas tracks, de vários tipos, possibilitando até você alterar as bands, possibilitando efeitos como karaoke, nightcore, bassbost, entre outros.

## Adicionando efeitos

Nessa guide, um exemplo de como alterar o efeito rotation será usado.

O Coglink oferece a função `coglink_setEffect`, que você pode alterar todos os efeitos que a Lavalink oferece, seu uso é bem fácil, veja a seguir um exemplo:

```c
coglink_setEffect(&lavaInfo, message->guild_id, COGLINK_FILTER_ROTATION, "{\"rotationHz\":0.2}");
```

Oi? "Cade o resto do código"? É só isso mesmo, incrível, não?

Esse código fará o famoso efeito "8d" em sua track, além de fácil, é legal, não acha?

## Removendo efeitos

O tópico acima é de como adicionar um efeito, mas como removemos?

O arquivo `definitions.h` do Coglink te oferece várias macros para ser usadas, uma delas é a `FILTER_REMOVE`, onde que define o valor para que as funções do Coglink entenda que você quer remover o efeito.

Veja o exemplo a seguir:

```c
coglink_setEffect(&lavaInfo, message->guild_id, COGLINK_FILTER_REMOVE, NULL);
```

E pronto! Você vai ter removido o efeito, caso tenha adicionado um.

* *Guia por*: @ThePedroo
* Feito com amor por um brasileiro! ❤️
