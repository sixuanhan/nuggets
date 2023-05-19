# CS50 Nuggets
## Implementation Spec
### THE Ohio State University, Spring 2023

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes the grid module.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Plan for division of labor

> Update your plan for distributing the project work among your 3(4) team members.
> Who writes the client program, the server program, each module?
> Who is responsible for various aspects of testing, for documentation, etc?

## Player

This is the outline of the data structures, functional breakdown, and pseudo code for the `player` program.

### Data structures

This program will implement a new `player` structure corresponding to each client.

```c
typedef struct player {
    char* username;
	int ID;
	char letter;
	int gold;
	int locX;
	int locY;
	char* localMap;
} player_t;
```

### Definition of function prototypes

A function to parse the command-line arguments, initialize the game struct, and initialize the message module.

```c
static int parseArgs(const int argc, char* argv[]);
bool handleInput(char keystroke);
bool handleMessage(char* message);

```

### Detailed pseudo code


#### `parseArgs`:

	validate commandline
	initialize message module
	print assigned port number
	decide whether spectator or player

---

A function that reads and processes keystrokes from stdin using `ncurses`.

#### `handleInput`:

	read the keystroke from stdin
	check the validity of the keystroke
	send a corresponding message to the server
	return a boolean that indicates whether to exit the loop


A function that passes the message that is received from the server to the relevant function.

#### `handleMessage`:

	parse the first part of the message to identify which type of message it is
	call the `handleXYZ` function that handles that type of message specifically
	return a boolean that indicates whether to exit the loop

---

## Server

### Data structures


### Definition of function prototypes

A function to parse the command-line arguments, initialize the game struct, initialize the message module, and (BEYOND SPEC) initialize analytics module.

```c
static int parseArgs(const int argc, char* argv[]);
```

A function to create a new `game` data structure, initialize data, and randomize the location of gold pile drops. 

```c
static game_t* initializeGame();
```


An overarching function to handle incoming messages from a client and call specific handleXYZ functions to do the jobs.
```c
static bool handleMessage(void* arg, const addr_t from, const char* message)
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


#### `initializeGame`:

	call game_new and store the game struct in a variable
	call game_scatter_gold
	return the game_t pointer


#### `handleMessage`:

	if the first word of the message is the same as "PLAY "
		extract the message part
		call handlePLAY
	else if the first word of the message is the same as "SPECTATE "
		call handleSPECTATE
	else if the first word of the message is the same as "KEY "
		extract the message part
		call handleKEY
	else
		log error
		send an ERROR message to the client



---

## game module

The game module implements the `game` struct as well as related functions. Note that the module will be implemented directly in `server.c` instead of an independent file such as `game.c`. For clarity purpose, we will explain its implementation here in an independent section.

### Data structures

The `game` structure will store variables that provide information about each location on the grid.

```c
typedef struct game {
    char* mainGrid;
	int numPlayers;
	int goldRemaining;
	player_t** players;
	hashtable nuggetsInPile;
} game_t;
```

### Definition of function prototypes

This function will initialize a new game struct and return its pointer.
```c
game_t* game_new(void);
```

This function will drop at least GoldMinNumPiles and at most GoldMaxNumPiles gold piles on random room spots; each pile shall have a random number of nuggets. It will store the information in nuggetsInPile and also update the mainGrid in the game struct.
```c
void game_scatter_gold(game_t* game, int randSeed);
```

This function will clean up a game struct and everything within it.
```c
void game_delete(game_t* game);
```


### Detailed pseudo code

#### `game_new`:

	allocate memory for game_t* and exit error if failure to allocate memory with mem_malloc_assert
	allocate memory for mainGrid and exit error if failure to allocate memory with mem_malloc_assert
	allocate memory for players and exit error if failure to allocate memory with mem_malloc_assert
	initialize nuggetsInPile with hashtable_new
	intialize numPlayers to 0
	intialize goldRemaining to GoldTotal


#### `game_scatter_gold`:

	create a variable int goldRemaining and initialize it to GoldTotal
	initialize a variable int numPilesRemaining by randomly picking a number in [GoldMinNumPiles, GoldMaxNumPiles]
	while numPilesRemaining is greater than 0
		randomly pick a number in [0, NRxNC-1] to be the gold drop coordinate
		if the coordinate does not represent an empty room spot in mainGrid
			continue
		randomly pick a number in [1, goldRemaining/2] to be the number of nuggets dropped on that coordinate
		store the (coordinate, numberOfNuggets) information in nuggetsInPile
		change the empty room spot in mainGrid to '*'
			decrement goldRemaining by numberOfNuggets
			decrement numPilesRemaining by 1





#### `game_delete`:

	free the mainGrid string
	delete each player in the array
	free the array
	call hashtable_delete on the nuggetsInPile hashtable
	free the game struct itself


## grid module

### Data structures

This module implements a grid, which is a string with NRxNC characters represending the map.

### Definition of function prototypes

``` c
void grid_load(FILE* fp, char* grid);
int grid_1dto2d_x(int loc);
int grid_1dto2d_y(int loc);
int grid_2dto1d(int x, int y);
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

	return loc/NR

#### `grid_1dto2d_y`:

	return loc%NR

#### `grid_2dto1d`:

	return NC*x+y

#### `grid_update_vis`:
	

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
