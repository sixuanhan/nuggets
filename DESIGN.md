# CS50 Nuggets
## Design Spec
### THEOhioStateUniversity, Spring 2023

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes x, y, z modules.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.


## Client

The *client* acts in one of two modes:

1. *spectator*, the passive spectator mode described in the requirements spec.
2. *player*, the interactive game-playing mode described in the requirements spec.


### User interface

See the requirements spec for both the command-line and interactive UI.


### Inputs and outputs

The user will input keystrokes that the function will use to determine how to update the client’s information. This will be used to change the display on the server side.

So that the user can access error messages, there will be an additional file or the option to use `stderr` that will store the error messages from failed inputs. In order to do that, the user can simply call:
`./server 2>server.log map.txt`


### Functional decomposition into modules


`parseArgs`: This function will be responsible for ensuring the number of inputs and the type of inputs matches what is specified in the `Requirements Spec`. This includes a hostname that must be a string, typically `localhost`, a port that must be an integer, and a potential third argument that must be a string: `playername`. If any of the arguments do not meet these specifications, a relevant error message will be provided and the exit code will be non-zero.


`handleInput`: This function will be responsible for reading keystrokes from stdin with the help of ncurses and processing them.


`handleMessage`: This function will be responsible for taking the string that is passed between the client and the server and  It will parse messages it receives from the message module per the requirement spec that can be employed through various action functions, including `handleOK`, `handleGRID`, `handleGOLD`, `handleDISPLAY`, `handleQUIT`, and `handleERROR`.


`handleOK`: This function is called by handleMessage. It connects the client and passes the letter that corresponds to their player.


`handleGRID`: This function is called by handleMessage. It notifies the client of the size of the grid as a constant integer.


`handleGOLD`: This function is called by handleMessage. It notifies the client of three variables, `n`: the number of gold nuggets collected, `p`: the number of gold nuggets in the client’s purse, and `r`: the number of remaining gold nuggets on the map. 


`handleDISPLAY`: This function is called by handleMessage. It passes a string to a client that corresponds to the physical depiction of the map.


`handleQUIT`: This function is called by handleMessage. It removes a client from the server, it outputs the message corresponding to the removal, and exits the program.


`handleERROR`: This function is called by handleMessage. It is employed when the client attempts an invalid action and stores it in the stderr or log file.




### Pseudo code for logic/algorithmic flow

#### main
The client will run as follows:

	execute from a command line per the requirement spec
	parse the command line, validate parameters
	initialize the 'message' module
	send a message to the server, requesting connection
	call message_loop(), to await the server
	shut down when the client tells it to
	clean up


#### handleInput
	read the keystroke from stdin
	check the validity of the keystroke
	send a corresponding message to the server
	return a boolean that indicates whether to exit the loop


#### handleMessage
	parse the first part of the message to identify which type of message it is
	call the `handleXYZ` function that handles that type of message specifically
	return a boolean that indicates whether to exit the loop

#### handleOK

#### handleGRID

#### handleGOLD

#### handleDISPLAY

#### handleQUIT

#### handleERROR



### Major data structures
The client shall use the `ncurses` library to arrange its interactive display and allow the program to read one character from the keyboard when told that stdin has input ready.


---


## Server
### User interface

See the requirements spec for the command-line interface.
Any other interactions with the user is done indirectly through client messages. 

### Inputs and outputs

The input for the server is a map file (of type .txt) that we assume to be valid. It can optionally take a seed for randomization. Moreover, outside of the command-line, the server should also receive messages from the clients. 

The outputs are the strings that are passed to the clients. There should be no output directly to the terminal coming from the server.

So that the user can access error messages, there will be an additional file or the option to use `stderr` that will store the error messages from failed inputs on the server side as well. In order to do that, the user can simply call:
./server 2>server.log map.txt

### Functional decomposition into modules

`parseArgs`: Verifies that the user inputs a string that corresponds to the filename of a readable map file and that the seed is a positive integer after random number generation.


`initializeGame`: Calls `game_new` to create a new `game` data structure, calls grid_load, and randomizes the location of gold pile drops. It calls initializeMessage.


`handleMessage`: Processes the message sent from the client and determines what the server should do based on the type of input received from the message. It will parse messages it receives from the message module per the requirement spec.


`handlePLAY`: This function is called by handleMessage. Initializes a player game with the name given by the client message.


`handleSPECTATE`: This function is called by handleMessage. Initializes a spectator game.


`handleKEY`: This function is called by handleMessage. It will process any client messages representing keystrokes (KEY k).


`gameOver`: Prints “Game Over” to all clients and prints the score table.



### Pseudo code for logic/algorithmic flow


#### main
The server will run as follows:

   	execute from a command line per the requirement spec
	parse the command line, validate parameters
	call initializeGame() to set up data structures
	initialize the 'message' module
	print the port number on which we wait
	call message_loop(), to await clients
	call gameOver() to inform all clients the game has ended
	clean up



#### initializeGame

#### handleMessage

#### handlePLAY

#### handleSPECTATE

#### handleKEY

#### gameOver


### Major data structures


There will be a static global data structure, `game`, which stores important variables corresponding to information relevant to the game in both the client and the server. The `game` will hold:

`grid`: the game grid

`numPlayers`: the number of players who have joined

`usernames`: an array, to store the real name for each player

`spectator`: the address of the spectator, if there is one

`goldRemaining`: how many nuggets there are remaining in the game

`locEachPlayer`: an array, to store how the coordinate of each player at the moment

`nuggetsEachPlayer`: an array, to store how many nuggets each player has at the moment


The `grid` struct is a two-dimensional array of size NRxNC. Each entry of the array is a `gridcell` struct. 


---


## the Grid module

**grid.c** provides a module that helps with the grid part for both the client and the server. There will be a `grid` struct that is a two-dimensional array of size NRxNC. Each entry of the array is a `gridcell` struct. 

### Functional decomposition

`gridcell_new`:  this function initializes a new gridcell.

`grid_new`: this function initializes a new grid of the given size.

`grid_load`: this function takes the map file and loads the information into a `grid` struct.

`grid_get_location_spot`: this function takes a coordinate (i, j) and returns the spot type on the coordinate in the grid.

`grid_get_location_nuggets`: this function takes a coordinate (i, j) and returns the number of nuggets on the coordinate in the grid.

`grid_get_location_vis`: this function takes a coordinate (i, j) and returns the visibility on the coordinate in the grid.

`grid_set_location_spot`: this function takes a coordinate (i, j) and a spot type, and replaces the gridcell on the coordinate in the grid.

`grid_set_location_nuggets`: this function takes a coordinate (i, j) and a spot type, and replaces the gridcell on the coordinate in the grid.

`grid_set_location_vis`: this function takes a coordinate (i, j) and a spot type, and replaces the gridcell on the coordinate in the grid.

`grid_out`: this function takes a `grid` and a client as input and outputs a string representing the `grid` that the client should display. This function should also implement visibility. It should utilize line-tracing algorithms (e.g. Bresenham) in order to calculate and help update the set of which grid cells are visible to the player. 


### Pseudo code for logic/algorithmic flow

#### grid_load

	reads the input file line by line
	for each line
		for each character
			initialize a gridcell for that coordinate
			stores the character in the gridcell

#### grid_out

	initialize a string buffer for output
	for each coordinate in the grid
		if the gridcell is visible to the player
			write the character of the gridcell to the string
		else
			if the gridcell is an empty room spot or an occupant character
				write an empty room spot to the string
			else
				write a solid rock spot to the string
	return the string


### Major data structures

The `grid` struct is basically a wrapper for a two-dimensional array of `gridcell` structs. 
The `gridcell` struct contains the following information:

`spot`: the character of the spot
`nugs`: the amount of gold value of the spot
`vis`: an array the visibility of the spot for each player