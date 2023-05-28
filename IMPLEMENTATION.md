# CS50 Nuggets
## Implementation Spec
### THE Ohio State University, Spring 2023

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes the grid module.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Plan for division of labor

While all group members are responsible for the entirety of the project, we assign certain tasks for certain group members to prioritize and complete first:
* Kevin - Server module + Bresenham algorithm in Grid module
* James - Client module
* Selena - Grid module
* Steven - Server module & Grid module (particularly where both modules intersect)

We will be responsible for unit testing our individual modules in order to ensure that they function as intended, but we will conduct integration and system tests together in order to check aggregate performance. Even though we are dividing up the code, each group member should be available to help another group member on their code if necessary. 

## Client

This is the outline of the data structures, functional breakdown, and pseudo code for the `client` program.

### Data structures

The client is a bare-bones receiver and communicator of strings as inputs and ouputs, so no additional data structures are required.

### Definition of function prototypes

A function to parse the command-line arguments, initialize the game struct, and initialize the message module. It returns 0.
```c
static int parseArgs(const int argc, char* argv[]);
```

A function that reads and processes keystrokes from stdin using `ncurses`. It returns false.
```c
static bool handleInput(void* arg);
```

A function that passes the message that is received from the server to the relevant function. It returns a handleXYZ function's return.
```c
static bool handleMessage(void* arg, const addr_t from, const char* message);
```

A function that connects the client to the server and returns the letter that corresponds to their player. It returns false.
```c
static bool handleOK(const char* message);
```

A function that notifies the client of the size of the grid as a constant integer. It returns false.
```c
static bool handleGRID(const char* message);
```

A function that notifies the client of three variables, `n`: the number of gold nuggets collected, `p`: the number of gold nuggets in the client’s purse, and `r`: the number of remaining gold nuggets on the map. It returns false.
```c
static bool handleGOLD(const char* message);
```

A function that passes and prints out a string to a client that corresponds to the physical depiction of the map. It returns false.
```c
static bool handleDISPLAY(const char* message);
```

A function that removes a client from the server, it outputs the message corresponding to the removal, and exits the program. It returns true.
```c
static bool handleQUIT(const char* message);
```

A function that informs the client of an invalid action and stores it in the stderr or log file. It returns false.
```c
static bool handleERROR(const char* message);
```


### Detailed pseudo code

A function to parse the command-line arguments, initialize the game struct, and initialize the message module.

#### `parseArgs`:

	validate commandline
	initialize message module
	print assigned port number
	decide whether spectator or player


#### `handleInput`:

	read the keystroke from stdin
	check the validity of the keystroke
	send a corresponding message to the server
	return a boolean that indicates whether to exit the loop


#### `handleMessage`:

	parse the first part of the message to identify which type of message it is
	return the `handleXYZ` function that handles that type of message specifically
	return a boolean that indicates whether to exit the loop


#### `handleOK`:
	prints out the OK message
	return false


#### `handleGRID`:
	parse through the GRID message that the client receives from the server
	extract the grid size from the message
	if the current window size is too small
		inform the user to about the minimum required window size
		wait for the user to enlarge the window
	return false


#### `handleGOLD`:
	parse through the GOLD message that the client receives from the server
	extract `n`, `p`, and `r`, as denoted in the functional decomposition
	update the top line of display regarding the game status
	return false


#### `handleDISPLAY`:
	parse through the DISPLAY message that the client receives from the server
	output the updated map that is displayed to the client 
	return false


#### `handleQUIT`:
	take the QUIT message and inform the user of the quit and the reason for the quit
	break out of the message_loop() (break command may appear outside of function but called immediately after this function is called)
	return true


#### `handleERROR`:
	print out the ERROR message received from the client
	log the error 
	return false


## Server

### Data structures

There is a static global `game` struct and a `player` struct in *server.c*. They will be explained in the next section: "the game module". Note that this is not an individual module like a `game.c`.

### Definition of function prototypes

A function to parse the command-line arguments, initialize the game struct, initialize the message module, and (BEYOND SPEC) initialize analytics module.

```c
static int parseArgs(const int argc, char* argv[], char* mapFile);
```

An overarching function to handle incoming messages from a client and call specific handleXYZ functions to do the jobs.
```c
static bool handleMessage(void* arg, const addr_t from, const char* message);
```

A function to handle PLAY messages and add a new player to the game, initializing the display for that client.
```c
static bool handlePLAY(const addr_t from, const char* content);
```

A function to handle SPECTATE messages and add a spectator to the game, initializing the display for that client.
```c
static bool handleSPECTATE(addr_t from, const char* content);
```

A function to handle KEY messages and updates the game based on the message that it receives from a client.
```c
static bool handleKEY(const addr_t from, const char* content);
```

A function to handle the end of the game and sends a message to each client with a QUIT GAME OVER message.
```c
static bool gameOver();
```

A function to update the main grid when a player moves.
```c
static char* updateMainGrid(const int playerIndex, const int old_loc);
```

### Detailed pseudo code

#### `parseArgs`:

	if the number of arguments is not 2 or 3
		print error message to stderr
		exit non-zero
	store the path to the map file
	if we cannot open the file for reading
		print error message to stderr
		exit non-zero
	if the number of arguments is 3
		if the third argument (the seed) is not an int
			print error message to stderr
			exit non-zero
		store the seed in a variable
	else
		seed the random-number generator with getpid()
	find out how many lines there are and store it in NR
	find out how many characters are in each line and store it in NC
	call game_new
	call grid_load on the mainGrid
	call game_scatter_gold


#### `handleMessage`:

	if the first word of the message is the same as "PLAY "
		extract the message part
		return handlePLAY
	else if the first word of the message is the same as "SPECTATE "
		return handleSPECTATE
	else if the first word of the message is the same as "KEY "
		extract the message part
		return handleKEY
	else
		log error
		send an ERROR message to the client
		return false

#### `handlePLAY`:

	if non-empty name and have not reached maxPlayers
		call player_new
		send an OK, GOLD, GRID, and DISPLAY message to client with player display
	else
		send QUIT message to the client
		return true
	return false

#### `handleSPECTATE`:

	if there is another spectator
		send a QUIT message to previous spectator
	send an GOLD, GRID, and DISPLAY message to new client with spectator display

#### `handleKEY`:

	if message came from player
		if the keystroke denotes a valid movement command
			update the grid for the player
			send updated DISPLAY message back to all players (not sure if this is correct)
			if goldRemaining == 0
				return gameOver()
		else if the keystroke is Q
			send a QUIT message to the client 
			close communication socket with client
			delete the corresponding player struct of the client
		else
			send an ERROR message to the client
	else if message came from spectator
		if the keystroke is Q
			send a QUIT message to the client
			close communication socket with the client
			delete the corresponding player struct of the client
		else
			send an ERROR Message to the client 
	return false

#### `gameOver`:

	loop through the players array
		send a QUIT message to every client with a scoreboard
	return true

#### `updateMainGrid`:

	if the player is stepping onto another player
		swap the two players, updating their stored spot type data
	else if the player is stepping onto gold
		update the gold and send a gold message to everyone
	else if the player is stepping on to a spot
		move the player and restore the spot they're stepping from to its previous character


---

## game module

The game module implements the `game` struct, the `player` struct, as well as related functions. Note that the module will be implemented directly in `server.c` instead of an independent file such as `game.c`. For clarity purpose, we will explain its implementation here in an independent section.

### Data structures

The `game` structure will store variables that provide information about each location on the grid.

```c
typedef struct game {
    char* mainGrid;  // the grid that contains all information (spectator's view)
	int numPlayers;  // the number of players that have joined the games
	int goldRemaining;  // the number of unclaimed nuggets
	player_t** players;  // an array of players
    addr_t* spectator; // the spectator's address
	counters_t* nuggetsInPile;  // where all the gold is and how many nuggets there are in each pile
} game_t;
```

The `player` structure corresponds to each client, storing information about them.

```c
typedef struct player {
    char* username;
	char letterID;
	int gold;  // how many nuggets they have
	int loc;    // a 1d coordinate of the current location
    char currSpot;  // the spot type that they player's standing on
	char* localMap;  // the grid that this player can see
	addr_t* address;
} player_t;
```

### Definition of function prototypes

This function will initialize a new game struct and return its pointer.
```c
static game_t* game_new(void);
```

This function will generate a random number in the range [min, max].
```c
static int randRange(int min, int max);
```

This function will drop at least GoldMinNumPiles and at most GoldMaxNumPiles gold piles on random room spots; each pile shall have a random number of nuggets. It will store the information in nuggetsInPile and also update the mainGrid in the game struct.
```c
static void game_scatter_gold(void);
```

This function will clean up a game struct and everything within it.
```c
static void game_delete(game_t* game);
```

This function will initialize a new player struct and return its pointer.
```c
static player_t* player_new(void);
```


### Detailed pseudo code

#### `game_new`:

	allocate memory for game_t* and exit error if failure to allocate memory with mem_malloc_assert
	allocate memory for mainGrid and exit error if failure to allocate memory with mem_malloc_assert
	initialize nuggetsInPile with counters_new
	initialize numPlayers to 0
	initialize goldRemaining to GoldTotal


#### `randRange`:

	return min + rand() % (max-min+1)


#### `game_scatter_gold`:

	create a variable int goldRemaining and initialize it to GoldTotal
	initialize a variable int numPilesRemaining by randomly picking a number in [GoldMinNumPiles, GoldMaxNumPiles]
	while numPilesRemaining is greater than 0
		randomly pick a number in [0, NRxNC-1] to be the gold drop coordinate
		if the coordinate does not represent an empty room spot in mainGrid
			continue
		if we are not down to the last pile
			randomly pick a number in [GoldTotal/numPiles*0.5, GoldTotal/numPiles*1.5] to be the number of nuggets dropped on that coordinate
		else
			drop all remaining nuggets
		store the (coordinate, numberOfNuggets) information in nuggetsInPile
		change the empty room spot in mainGrid to '*'
		decrement goldRemaining by numberOfNuggets
		decrement numPilesRemaining by 1


#### `game_delete`:

	free the mainGrid string
	free each player_t* in the players array
	call counters_delete on the nuggetsInPile counters
	free the game struct itself


#### `player_new`:

	allocate memory for player_t* and exit error if failure to allocate memory with mem_malloc_assert
	initialize localMap to all ' '
	initialize letterID to 'A'+ID
	initialize gold to 0
	initialize loc to a random number in [0, NRxNC-1]
	while the coordinate does not represent an empty room spot in mainGrid
		set loc to a random number in [0, NRxNC-1] to be the gold drop coordinate again
	update the player's map to show @
	remember the current spot that the player is standing on
	update the player's map to show the player's letterID
	call grid_update_vis



## grid module

### Data structures

This module implements a grid, which is a string with NRxNC characters represending the map.

### Definition of function prototypes

This function loads the information from a file (map.txt) to a grid.
``` c
void grid_load(FILE* fp, char* grid, int NR, int NC);
```

This function converts a one dimensional coordinate to a two dimensional coordinate and returns the x coordinate.
``` c
int grid_1dto2d_x(int loc, int NR, int NC);
```

This function converts a one dimensional coordinate to a two dimensional coordinate and returns the x coordinate.
``` c
int grid_1dto2d_y(int loc, int NR, int NC);
```

This function converts a two dimensional coordinate to a one dimensional coordinate and returns it.
``` c
int grid_2dto1d(int x, int y);
```

This function checks if a point in the grid located at `end_loc` is visible from a player located at the `start_loc`.
``` c
bool grid_isVisible(char** grid, int start_loc, int end_loc);
```

This function updates the visibility of a player according to the `mainGrid`, the previous `localMap` and the new location and updates the `localMap`.
``` c
void grid_update_vis(char** mainGrid, char** localMap, int loc);
```

### Detailed pseudo code

#### `grid_load`:

	initialize a variable int i to 0
	as long as i is less than NRxNC
		we read a character from fp
		if the character is not '\n'
			set grid[i] to be that character
			increment i by 1

#### `grid_1dto2d_x`:

	return loc%NC

#### `grid_1dto2d_y`:

	return int(loc/NC)

#### `grid_2dto1d`:

	return x+NC*y

#### `isVisible`:

	start_x = grid_1dto2d_x(start_loc)
	start_y = grid_1dto2d_y(start_loc)
	end_x = grid_1dto2d_x(end_loc)
	end_y = grid_1dto2d_y(end_loc)
	set dx to the magnitude of the difference between the x coordinates
	set dy to the magnitude of the difference between the y coordinates
	sx = -1 if start_x > end_x; sx = 1 otherwise
	sy = -1 if start_y > end_y; sy = 1 otherwise
	error1 = dx - dy
	loop continuously
		if start_x reached end_x and start_y reached end_y
			return true
		if grid[grid_2dto1d(start_x, start_y)] is "-", "|", "+", " ", or "#"
			return false 
		error2 = 2 * error1
		if error2 > -dy:
			decrement error1 by dy
			increment start_x by sx
		if error2 < dx:
			increment error1 by dx
			decrement start_y by sy

#### `grid_update_vis`:

	for each index in localMap
		if isVisible(mainGrid, start_loc, index)
			copy mainGrid[index] onto localMap[index]

---

## Testing

### Unit Testing
Each unit/module will be tested independently at first before being aggregated together. The grid module will be tested independently first, and then client and the server will be tested. Individual functions within each unit/module will be tested to ensure that they work properly. 

### Integration Testing
We will test our client with the given server, and then test our server with the given client. We can again log the messages being sent back and forth between the client and the server to ensure that the messages are being sent properly. 

### System Testing
We run the client and server together on the same device and on different devices and test all functions and edge cases. The game will be played and tested numerous times to ensure that not only do all the desired game mechanics work as intended, but also the game behaves properly in special edge cases. 

The following are some examples of "special" cases that we consider testing:
* Invalid keystrokes
* Collisions between players and with walls 
* Having too many players join 
* Initializing a spectator session with an already existing specator 
* Players leaving/rejoining 
* Starting with too small of a window

Moreover, it should be noted that we also use valgrind to test for memory leaks throughout testing to ensure that there are no problems memory-wise.

---

## Limitations

> Bulleted list of any limitations of your implementation.
> This section may not be relevant when you first write your Implementation Plan, but could be relevant after completing the implementation.
