# Procurando uma música

## Procurando de ONDE?

A Lavalink permite que você procure uma música de várias fontes, e nesse guia, você vai poder customizar a função de search para procurar de uma fonte específica ou de todas as fontes.

## Searching

Para procurar por músicas, a gente vai usar a função `coglink_searchSong`, que vai preencher o último parâmetro, a estrutura `struct requestInformation *` com o body (JSON) da resposta da Lavalink.

### Por que o Coglink não da parse automaticamente

O Coglink foi feito pensando em performance, se fosse para dar parse no JSON inteiro, causaria uma perda de performance enorme, então a gente decidiu criar uma função que você pode escolher qual música você quer dar parse, a primeira, a segunda, a terceira, e assim por diante, fazendo ficar mais ágil e melhor.

Olhe o exemplo abaixo da função `coglink_searchSong` sendo usada:

```c
struct requestInformation res;

int resStatus = coglink_searchSong(&lavaInfo, "Very ordinary life", &res);

if (resStatus != 0) {
  log_fatal("Erro dando parse na música: %d", resStatus);
}

log_debug("Body da resposta da Lavalink: %s", res.body);
    
/* ... */

coglink_searchCleanup(res);
```

Pronto, o código acima vai dar print na resposta da search da Lavalink, e caso ele falhe, irá retornar um valor int de o que deu errado.

Você pode prosseguir com o próximo guia, o guia [`dando parse na resposta do search`](/guides/pt-BR/parse_no_search.md)

* *Guia por*: @ThePedroo
* Feito com amor por um brasileiro! ❤️
