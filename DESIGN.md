# CS50 Nuggets
## Design Spec
### THEOhioStateUniversity, Spring 2023

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes x, y, z modules.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

In this DESIGN.md, we focus on the following for both the client and server:

* User interface
* Inputs and outputs
* Functional decomposition into modules
* Pseudocode for logic/algorithmic flow
* Major data structures
* Testing plan

## Client

The *client* acts in one of two modes:

1. *spectator*, the passive spectator mode described in the requirements spec.
2. *player*, the interactive game-playing mode described in the requirements spec.

### User interface

As required by the requirements spec, the user can connect to the server and join the game through the terminal command-line:
```bash
./client hostname port [playername]
```

When the game is running, it can also process user keyboard commands that allow the user to interact with the game. The exact list of valid keystrokes for the user depends on whether the user is a spectator or a player, and is also listed out in the requirements spec. 

See the requirements spec for more info regarding both the command-line and interactive UI.

### Inputs and outputs

When joining the game, the client needs to add as parameters the hostname/IP address which the server is running, the port number that the server expects to receive messages from, and an optional player name. If the player name is not provided, the client joins as a spectator. 

The user will input keystrokes that the function will use to determine how to update the client’s information. The specific valid keystrokes are listed out in full detail in the requirements spec. The keystrokes will be processed and sent in a message to the server. 

The client will have the map displayed to them (the parts of the map the client can see depends on whether they are a player or a spectator). The information regarding the map's details will reach the client from a message sent from the server, which is responsible for processing the map.

So that the user can access error messages, there will be an additional file or the option to use `stderr` that will store the error messages from failed inputs. In order to do that, the user can simply call:
`./server 2>server.log map.txt` 

### Functional decomposition into modules

`parseArgs`: This function will be responsible for ensuring the number of inputs and the type of inputs matches what is specified in the `Requirements Spec`. This includes a hostname that must be a string, typically `localhost`, a port that must be an integer, and a potential third argument that must be a string: `playername`. If any of the arguments do not meet these specifications, a relevant error message will be provided and the exit code will be non-zero.

`handleInput`: This function will be responsible for reading keystrokes from stdin with the help of ncurses and processing them. The keystroke will be sent in the form of a message to the server, which will then update the game based on the message it receives.

`handleMessage`: This function will be responsible handling the message that is received from the server. It will parse the messages it receives from the message module per the requirement spec which can be employed through various action functions, including `handleOK`, `handleGRID`, `handleGOLD`, `handleDISPLAY`, `handleQUIT`, and `handleERROR`.

`handleOK`: This function is called by handleMessage. It connects the client and passes the letter that corresponds to their player.

`handleGRID`: This function is called by handleMessage. It notifies the client of the size of the grid as a constant integer.

`handleGOLD`: This function is called by handleMessage. It notifies the client of three variables, `n`: the number of gold nuggets collected, `p`: the number of gold nuggets in the client’s purse, and `r`: the number of remaining gold nuggets on the map. 

`handleDISPLAY`: This function is called by handleMessage. It passes and prints out a string to a client that corresponds to the physical depiction of the map. 

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
	parse through the message that the client receives from the server
	extract the grid size from the message
	adjust the window size

#### handleGOLD
	parse through the message that the client receives from the server
	extract `n`, `p`, and `r`, as denoted in the functional decomposition
	update the top line of display regarding the game status

#### handleDISPLAY
	parse through the message that the client receives from the server
	find the part of the message string that denotes the part of the map the player should see

#### handleQUIT

#### handleERROR



### Major data structures
The client shall use the `ncurses` library to arrange its interactive display and allow the program to read one character from the keyboard when told that stdin has input ready.


---

## Server
### User interface

As denoted by the requirements spec, the server's initial interface with the user is through the terminal command-line via the following bash command:
```bash
./server map.txt [seed]
```

Any other interactions with the user is done indirectly through client messages. 

See the requirements spec for more information about the user interface.

### Inputs and outputs

The input for the server is a map file (of type .txt) that we assume to be valid. It can optionally take a seed for randomization. Moreover, outside of the command-line, the server should also receive messages from the clients. 

The outputs are the strings that are passed to the clients in the form of messsages. There should be no output directly to the terminal coming from the server.

So that the user can access error messages, there will be an additional file or the option to use `stderr` that will store the error messages from failed inputs on the server side as well. In order to do that, the user can simply call:
./server 2>server.log map.txt

### Functional decomposition into modules

`parseArgs`: Verifies that the user inputs a string that corresponds to the filename of a readable map file and that the seed is a positive integer after random number generation.

`initializeGame`: Calls `game_new` to create a new `game` data structure, calls grid_load, and randomizes the location of gold pile drops. 

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
	create the map with the desired gold drops
	initializes a new `game` data structure and loads in the starting game data
	sets up the server connection 

#### handleMessage
	listen for messages sent from the clients that are connected by the server
	update the `game` data structure accordingly from the contents of the message

#### handlePLAY
	check the validity of the player's name and whether there is still space for another player given *MaxPlayers* players
	if there is not enough space or the player's real name is invalid
		server should respond with a quit message and terminate the initialization of a new player
	else
		initialize a new `player` data structure 
		add the new `player` data struture to the array of players in the overarching `game datastructure

#### handleSPECTATE

#### handleKEY

#### gameOver
	



### Major data structures

There will be a static data structure, `player`, that holds:

`username`: their username

`ID`: their numerical ID

`letter`: their letter ID

`gold`: how many gold nuggets they have

`locX`: their x coordinate

`locY`: their y coordinate


There will be a static global data structure, `game`, which stores important variables corresponding to information relevant to the game in both the client and the server. The `game` will hold:

`grid`: the game grid

`numPlayers`: the number of players who have joined

`goldRemaining`: how many nuggets there are remaining in the game

`players`: an array of `player` structs


The `grid` struct is a two-dimensional array of size NRxNC. Each entry of the array is a `gridcell` struct. Find details in the grid module.


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


## testing

### unit testing
We will test the grid module independently at first. We will unit test small functions in both the client and the server to make sure they work.

### integration testing
We will test our client with the given server, and then test our server with the given client.

### system testing
We run the client and server together on the same device and on different devices and test all functions and edge cases.