# Dando parse na resposta do search

## Parsing? Por quê?

Dar parse no search é algo muito importate, já que permite você pegar o nome da música, a duração dela, sua thumbnail, seu URL, e muito mais. Caso você não de, você nem vai conseguir tocar a música.

## Parsing

Para dar parse, você vai precisar usar a função `coglink_parseSearch`, que vai preencher o último parâmetro com as informações da música que tu pediu para dar parse.

Por exemplo, você vai provavelmente querer dar parse na primeira música, não é? Então siga o exemplo abaixo para fazer isso:

```c
struct coglink_parsedTrackStruct parsedStruct;
coglink_initParseTrack(&lavaInfo, &parsedStruct, &res);

struct parsedTrack song;
int parseRes = coglink_parseTrack(&lavaInfo, &parsedStruct, &res, "0", &song);

if (parseRes != 0)
  log_fatal("Erro dando parse: %d", parseRes);

log_info("Nome da música: %s", song->name);
```

Pronto, agora a estrutura `parsedTrack` vai estar com as informações da música.

## Parsing + loop

Vamos fazer algo top aqui, caso você queira dar parse nas primeiras 10 músicas achadas, você pode usar o loop for, olhe o exemplo abaixo:

```c
struct parsedTrack song;

for (int i = 0; i < 10; i++) {
  char iString[16];
  snprintf(iString, sizeof(iString), "%d", i); 

  int parseRes = coglink_parseTrack(&lavaInfo, &parsedStruct, &res, iString, &song);

  if (parseRes != 0)
    log_fatal("Erro dando parse na música: %d", parseRes);

  log_info("Nome da música: %s", song->name);
}
```

Pronto, isso vai dar print nas primeiras 10 músicas achadas.

* *Guia por*: @ThePedroo
* Feito com amor por um brasileiro! ❤️
