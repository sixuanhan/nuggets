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
* Kevin - handleKEY in server.c and visibility algorithm in grid.c
* James - client.c
* Selena - grid.c, main, parsearg, and handleMessage in server.c
* Steven - handlePLAY, handleSPECTATE, and gameOver in server.c, specs and scrum management

We will be responsible for unit testing our individual modules in order to ensure that they function as intended, but we will conduct integration and system tests together in order to check aggregate performance. Even though we are dividing up the code, each group member should be available to help another group member on their code if necessary. 

## Client

This is the outline of the data structures, functional breakdown, and pseudo code for the `client` program.

### Data structures

The client contains a `game` data structure that stores relevant factors to the operation of certain methods throughout the file. These include **NC**, the number of columns in the grid; **NR**, the number of rows in the grid; **letter**, the letter corresponding to the client; **init**, a boolean that indicates whether the game has just begun for the client or not; **spect**, a boolean indicating whether or not a client is a spectator; **r**, **p**, and **n**, the variables used in the `handleGOLD` function as described below; **log**, the location of the log file; and **ServerHost** and **ServerPort**, the host and port of the server as defined on the command line.

### Definition of function prototypes

A function to parse the command-line arguments, initialize the game struct, and initialize the message module. It returns 0 if the client is a spectator, 1 if the client is a player, and another integer if an error is found with parsing the arguments.
```c
static int parseArgs(const int argc, char* argv[]);
```

A function that reads and processes keystrokes from stdin using `ncurses`. It returns false.
```c
static bool handleInput(void* arg);
```

A function that passes the message that is received from the server to the relevant function. It returns a handleXYZ function's boolean.
```c
static bool handleMessage(void* arg, const addr_t from, const char* message);
```

A function that connects the client to the server and stores the letter that corresponds to their player. It returns false.
```c
static bool handleOK(const char* message);
```

A function that notifies the client of the size of the grid as two constant integers. It returns false.
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

	validate number of arguments
	check if the port is a positive integer
	initialize game module
	return 0 if spectator, 1 if player


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

A function to tell everyone to update their display.
```c
static void broadcastDisplay(void);
```

A function to tell everyone to update when someone collects gold.
```c
static void broadcastGold(int playerIndex, int goldCollected);
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

	check where the message came from 
	if message came from player
		if the keystroke is Q
			send a QUIT message to the player
			clean up the player's stored info
		else if the keystroke denotes a valid undercase movement command
			check if the movement was valid
				if the player steps on another player
					update both players' locations and the spots they are located on
					update the main grid
					have the player steal the other players' gold (extra credit feature)
					send a GOLD message to all clients (extra credit feature)
				else if the player moves onto a gold pile
					update the gold 
					send a GOLD message to all clients
					update the main grid
					turn the gold pile into a regular room spot
					if no gold remains
						return true to exit the message loop and end the game
				else 
					simply update the player's movement on the map
				send DISPLAY message to all clients 
				return false to keep looping in the message loop
		else if the keystroke denotes a valid uppercase movement command
			keep looping one step at a time until the player can no longer travel in the desired direction
				update the player's stored location
				if the player steps on another player
					update both players' locations and the spots they are located on
					update the main grid
					have the player steal the other players' gold (extra credit feature)
					send a GOLD message to all clients (extra credit feature)
				else if the player moves onto a gold pile
					update the gold
					send a GOLD message to all clients
					update the main grid
					turn the gold pile into a regular room spot
					if no gold reamains
						return true to exit the message loop and end the game
				else 
					simply update the player's movement on the map
				if the player was actually able to move
					send a DISPLAY message to all clients
			return false to keep looping in the message loop
		else the keystroke command must be invalid
			send an ERROR messsage to the client
			return false to keep looping in the message loop
	else the message must have come from the spectator
		if the keystroke is Q
			send a QUIT message to the spectator
			free up the memory storing the spectator's address
			return false to keep looping in the message loop
		else the keystroke command must be invalid
			send an ERROR message to the spectator
			return false to keep looping in the message loop 

#### `gameOver`:

	loop through the players array
		send a QUIT message to every client with a scoreboard
	return true

#### `broadcastDisplay`:

	allocate memory for a DISPLAY message
	loop over the players list
	if the player is not NULL
		update their localMap
		generate a DISPLAY message for them
		send the message
		log
	free the DISPLAY message
	if there is a spectator
		allocate memory for a DISPLAY message
		generate a DISPLAY message for them
		send the message
		log
		free the DISPLAY message

#### `broadcastGold`:
	loop over the players list
	if the player is not NULL
		allocate memory for a DISPLAY message
		if it is the player who collects the gold
			generate a suitable DISPLAY message for them
		else
			generate a suitable DISPLAY message for them
		send the message
		log
	free the DISPLAY message
	if there is a spectator
		allocate memory for a DISPLAY message
		generate a DISPLAY message for them
		send the message
		log
		free the DISPLAY message

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
	call updateVis



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
int grid_2dto1d(int x, int y, int NR, int NC);
```

This function checks if a point in the grid located at `end_loc` is visible from a player located at the `start_loc`.
``` c
bool grid_isVisible(char* grid, int start_loc, int end_loc, int NR, int NC);
```

This function updates the visibility of a player according to the `mainGrid`, the previous `localMap` and the new location and updates the `localMap`. Note that this function is in server.c rather than grid.c.
``` c
void updateVis(char* mainGrid, char* localMap, int loc, int NR, int NC);
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

	return loc/NC

#### `grid_2dto1d`:

	return x+NC*y

#### `grid_isVisible`:

	extract x and y components of the starting point
	extract x and y components of the ending point
	find the difference between the x components and the y components
	if the distance between the two points is greater than 2.5 (extra credit feature)
		return false
	calculate the change in y that occurs from a 1 step increment in x when tracing the ray between the two points
	determine whether we step in the positive or negative x direction to go from ending point to the starting point
	if both points do not share the same x component
		loop through each point on the ray by stepping one unit in the x direction
			calculate the exact floating-point value of y if we step one unit in the x direction
			if we land on an exact point in the grid
				check whether the ray can traverse through that point
					return false if the ray cannot
			else the ray must be passing between two points in the grid
				check whether the ray can traverse through both points
					return false if the ray cannot traverse through either point
			increment the x by one step in the correct direction
	calculate the change in x that occurs from a 1 step increment in y when tracing the ray between the two points
	determine whether we step in the positive or negative y direction to grom ending point to the start point
	if both points do not share the same y component
		loop through each point on the ray by stepping one unit in the y direction
			calculate the exact floating-point value of x if we step one unit the y direction
			if we land on an exact point in the grid
				check whether ray can traverse through that point
					return false if the ray cannot
			else the ray must be passing between two points in the grid
				check whether the ray can traverse through both points
					return false if the ray cannot traverse through either point
			increment the y by one step in the correct direction
	return true

#### `updateVis`:

	for each index in localMap
		if isVisible(mainGrid, start_loc, index) or the character/location is \n
			copy mainGrid[index] onto localMap[index]
		else if a previously marked gold spot or player is no longer visible
			copy an empty room spot '.' onto the localMap[index]

---

## Testing

### Unit Testing
Each unit/module were tested independently at first before being aggregated together. The grid module was tested independently first, and then client and the server were tested. Individual functions within each unit/module were tested to ensure that they work properly. 

### Integration Testing
We tested our client with the given server, and then tested our server with the given client. We logged the messages being sent back and forth between the client and the server to ensure that the messages were being sent properly. 
We had a few hard coded invalid command line calls of the server and the client in testingClient.sh and testingServer.sh. Run `make test` to test.

### System Testing
We ran the client and server together on the same device and on different devices, with different maps, and test all functions and edge cases. The game will be played and tested numerous times to ensure that not only do all the desired game mechanics work as intended, but also the game behaves properly in special edge cases. 

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

The end product meets the specs of the project and has no known issues.
