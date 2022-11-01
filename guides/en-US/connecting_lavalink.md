# Connecting Lavalink Nodes

## Connecting

By default, every Lavalink is being hosted on port `2333`, we will assume in this guide that yours will as well be hosted on that port.

For you to connect a node, you will need to know 5 different pieces of information, the hostname, the port, the password, has SSL enabled, the shard count of the bot and the bot Id. If you know those, you can proceed to the next step, if you don't, find them before proceeding, you will need those.

Now with all this, we can proceed using the `coglink_connectNode` function, see the example below:

```c
struct lavaInfo lavaInfo = {
  .debug = 0
};

void on_cycle(struct discord *client) {
  coglink_wsLoop(&lavaInfo);
}

...

struct lavaNode params = {
    .name = "Node 1",
    .hostname = "localhost:2333",
    .password = "youshallnotpass",
    .shards = "1",
    .botId = "1234567898765431",
    .ssl = 0
};

coglink_connectNode(&lavaInfo, &params);

...

discord_set_on_cycle(client, &on_cycle);

...

// After discord_run

coglink_connectNodeCleanup(&lavaInfo);
```

Done, you will be now connecting to the Lavalink successfully, if not, check the values and check your firewall from both machines.

## Disconnecting

This is still not supported, sadly, but in the future, we hope we will be adding it, soonly, very soonly!

* *Guide made by*: @ThePedroo
