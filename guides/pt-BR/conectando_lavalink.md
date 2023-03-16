# Conectando nodes da lavalink

## Conectando

De padrão, toda Lavalink é hosteada na porta `2333`, que nesse guia, a gente vai assumir que a sua também está sendo hosteada nessa mesma porta.

Para você conectar uma node, você vai precisar de 5 informações diferentes, o hostname, a porta, a senha, se tem criptografia, quantas shards o BOT tem, e o Id do BOT. Caso você já saiba esses, prossiga para a próxima etapa, caso não, ache eles antes de continuar, já que os mesmos são necessários.

Com tudo isso, a gente pode prosseguir usando a função `coglink_connectNode`, olhe o exemplo a baixo:

```c
struct lavaInfo lavaInfo = {
  .debug = 0
};

...

struct coglink_lavalinkNode nodes[] = {
  {
    .name = "Node1",
    .hostname = "Hostname do node:port caso exista",
    .password = "você não deve passar",
    .ssl = 0
  }
};

struct coglink_lavalinkNodes nodeArray = {
  .nodes = nodes,
  .size = 1
};

struct coglink_nodeInfo nodeInfos[1];
coglink_connectNode(&lavaInfo, client, &nodeArray, nodeInfos);

...

// Depois do discord_run

coglink_connectNodeCleanup(&lavaInfo);
```

Pronto, agora você já vai estar conectando à sua Lavalink sem problemas, caso não, olhe as fields das estruturas e o firewall das duas máquinas.

## Desconectando

Para você fechar a conexão WebSocket com a Lavalink, você pode usar a função `coglink_disconnectNode`, olhe o exemplo a baixo:

```c
coglink_disconnectNode(&lavaInfo, 0);
```

Sim, é fácil assim, e pronto, a conexão WebSocket com a Lavalink já vai estar fechada.

* *Guia por*: @ThePedroo
* Feito com amor por um brasileiro! ❤️
