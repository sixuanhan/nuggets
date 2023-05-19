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

---

A function that passes the message that is received from the server to the relevant function.

#### `handleMessage`:

	parse the first part of the message to identify which type of message it is
	call the `handleXYZ` function that handles that type of message specifically
	return a boolean that indicates whether to exit the loop

---

## Server

### Data structures

The `game` structure will store variables that provide information about each location on the grid.

```c
typedef struct game {
    char* grid;
	int numPlayers;
	int goldRemaining;
	player_t** players;
	hashtable nuggetsInPile;
} game_t;
```

### Definition of function prototypes

> For function, provide a brief description and then a code block with its function prototype.
> For example:

A function to parse the command-line arguments, initialize the game struct, initialize the message module, and (BEYOND SPEC) initialize analytics module.

```c
static int parseArgs(const int argc, char* argv[]);
```
### Detailed pseudo code

> For each function write pseudocode indented by a tab, which in Markdown will cause it to be rendered in literal form (like a code block).
> Much easier than writing as a bulleted list!
> For example:

#### `parseArgs`:

	validate commandline
	verify map file can be opened for reading
	if seed provided
		verify it is a valid seed number
		seed the random-number generator with that seed
	else
		seed the random-number generator with getpid()

---

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
