# CS50 Nuggets
## Implementation Spec
### Team name, term, year

> This **template** includes some gray text meant to explain how to use the template; delete all of them in your document!

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

> Teams of 3 students should delete this section.

### Data structures

> For each new data structure, describe it briefly and provide a code block listing the `struct` definition(s).
> No need to provide `struct` for existing CS50 data structures like `hashtable`.

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
	initialize message module
	print assigned port number
	decide whether spectator or player

---

## Server

### Data structures

> For each new data structure, describe it briefly and provide a code block listing the `struct` definition(s).
> No need to provide `struct` for existing CS50 data structures like `hashtable`.

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

The gridcell struct represents each cell (or spot) in the grid. It contains key information related to the spot.

```c
typedef struct gridcell {
  char spot;                 // the character of the spot
  int nugs;       // the amount of gold value of the spot
  bool* vis;	// an array the visibility of the spot for each player
} gridcell_t;
```

The grid struct is a wrapper for a two dimensional array of pointers to gridcell structs. It contains all the information regarding the grid in the nuggets game for the server.

```c
typedef struct grid {
  gridcell_t* grid[NR][NC];
} grid_t;
```

### Definition of function prototypes

```c
static gridcell_t* gridcell_new(char spot);
grid_t* grid_new();
void grid_load(grid_t* grid, FILE* fp);
char grid_get_location_spot(grid_t* grid, int x, int y);
int grid_get_location_nuggets(grid_t* grid, int x, int y);
bool* grid_get_location_vis(grid_t* grid, int x, int y);
void grid_set_location_spot(grid_t* grid, int x, int y, char newSpot);
void grid_set_location_nuggets(grid_t* grid, int x, int y, char newNugs);
void grid_set_location_vis_for_player(grid_t* grid, int x, int y, bool vis);
void grid_update_vis_for_player(grid_t* grid, int x, int y, int clientID);
char* grid_out(grid_t* grid, int clientID);
void grid_iterate(grid_t* grid, void* arg, void (*itemfunc)(void* arg, gridcell_t gridcell));
void grid_delete(grid_t* grid);
```


### Detailed pseudo code

Pseudocode for `gridcell_new`:

	allocate memory for a pointer to a gridcell struct
	initialize spot to the parameter spot
	initialize nugs to 0
	initialize vis to a boolean array of 1 true and 26 false with length of 27 (26 players and 1 spectator)
	return the pointer

Pseudocode for `grid_new`:

	initialize an array of pointers to gridcells of size NR x NC
	return the pointer


Pseudocode for `grid_load`:

	for each row
		for each column
			call gridcell_new and pass the character as the parameter
			store the gridcell in the corresponding place

Pseudocode for `grid_get_location_spot`:

	find the gridcell at grid[x][y]
	return the spot character of the gridcell

Pseudocode for `grid_get_location_nuggets`:

	find the gridcell at grid[x][y]
	return the nuggets of the gridcell

Pseudocode for `grid_get_location_vis`:

	find the gridcell at grid[x][y]
	return the visibility array of the gridcell

Pseudocode for `grid_set_location_spot`:

	find the gridcell at grid[x][y]
	set the spot character of the gridcell to the passed parameter

Pseudocode for `grid_set_location_spot`:

	find the gridcell at grid[x][y]
	set the spot character of the gridcell to the passed parameter

Pseudocode for `grid_set_location_nuggets`:

	find the gridcell at grid[x][y]
	set the nuggets of the gridcell to the passed parameter

Pseudocode for `grid_set_location_vis_for_player`:

	find the gridcell at grid[x][y]
	set the visibility boolean of the indicated player in the visibility array of the gridcell to the passed parameter

Pseudocode for `grid_update_vis_for_player`:

Pseudocode for `grid_out`:

	initialize a string buffer for output
	use grid_iterate to loop over each cell
			if the gridcell is visible to the player
				write the character of the gridcell to the string
			else
				if the gridcell is an empty room spot or an occupant character
					write an empty room spot to the string
				else
					write a solid rock spot to the string
		write a '\n' character to the string
	return the string


Pseudocode for `grid_iterate`:

	for each row in the grid
		for each column/entry in the row
			call itemfunc to the gridcell with the argument


Pseudocode for `grid_delete`:

	for each row in the grid
			for each column/entry in the row
				delete the boolean array of the gridcell
				delete the gridcell
	delete the gridcell array of the grid
	delete the grid

---

## Testing plan

### unit testing
We will test the grid module independently at first. We will unit test small functions in both the client and the server to make sure they work.

### integration testing
We will test our client with the given server, and then test our server with the given client.

### system testing
We run the client and server together on the same device and on different devices and test all functions and edge cases.

---

## Limitations

> Bulleted list of any limitations of your implementation.
> This section may not be relevant when you first write your Implementation Plan, but could be relevant after completing the implementation.
