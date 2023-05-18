# CS50 Nuggets
## Implementation Spec
### THE Ohio State University, Spring 2023

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes x, y, z modules.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Plan for division of labor

> Update your plan for distributing the project work among your 3(4) team members.
> Who writes the client program, the server program, each module?
> Who is responsible for various aspects of testing, for documentation, etc?

## Player

This is the outline of the data structures, functional breakdown, and pseudo code for the `player` module.

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
```
### Detailed pseudo code


#### `parseArgs`:

	validate commandline
	initialize message module
	print assigned port number
	decide whether spectator or player

---

A function that reads and processes keystrokes from stdin using `ncurses`.

```c
bool handleInput(char keystroke);
```
### Detailed pseudo code


#### `handleInput`:

	read the keystroke from stdin
	check the validity of the keystroke
	send a corresponding message to the server
	return a boolean that indicates whether to exit the loop

---

A function that passes the message that is received from the server to the relevant function.

```c
bool handleMessage(char* message);
```
### Detailed pseudo code


#### `handleMessage`:

	parse the first part of the message to identify which type of message it is
	call the `handleXYZ` function that handles that type of message specifically
	return a boolean that indicates whether to exit the loop

---

## Server

### Data structures

There will be a `gridcell` structure, which contains the information relevant to each location on the grid.

```c
typedef struct gridcell {
    char spot;
	int nugs;
	char** vis;
} gridcell_t;
```

It will also define a new `grid` structure, which stores the position of each coordinate on the game as a 2D array.

```c
typedef struct grid {
    int NR;
	int NC;
} grid_t;
```

The `game` structure will implement the `grid` structure and store variables that provide information about each location on the grid.

```c
typedef struct game {
    grid_t* grid;
	int numPlayers;
	int goldRemaining;
	player_t** players;
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

## XYZ module

> For each module, repeat the same framework above.

### Data structures

### Definition of function prototypes

### Detailed pseudo code

---

## Testing plan

### unit testing

> How will you test each unit (module) before integrating them with a main program (client or server)?

### integration testing

> How will you test the complete main programs: the server, and for teams of 4, the client?

### system testing

> For teams of 4: How will you test your client and server together?

---

## Limitations

> Bulleted list of any limitations of your implementation.
> This section may not be relevant when you first write your Implementation Plan, but could be relevant after completing the implementation.
