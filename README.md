# Nuggets

This repository contains the code for the CS50 "Nuggets" game, in which players explore a set of rooms and passageways in search of gold nuggets.
The rooms and passages are defined by a *map* loaded by the server at the start of the game.
The gold nuggets are randomly distributed in *piles* within the rooms.
Up to 26 players, and one spectator, may play a given game.
Each player is randomly dropped into a room when joining the game.
Players move about, collecting nuggets when they move onto a pile.
When all gold nuggets are collected, the game ends and a summary is printed.

## Materials provided

See the [support library](support/README.md) for some useful modules.

See the [maps](maps/README.md) for some draft maps.

## Usage

``` bash
$ make
$ ./server map.txt [seed] 2>server.log
$ ./client hostname port [playername] 2>client.log
```

Examples:
``` bash
$ make
$ ./server maps/main.txt 2>server.log
$ ./client localhost 12345 selena 2>client.log
```

Provide a playername to join as a player; otherwise you will join as a spectator.