# CS50 Nuggets
## Design Spec
### THEOhioStateUniversity, Spring 2023

**Group Members**:
* Kevin Cao
* James Quirk
* Selena Han
* Steven Mendley

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

In addition, at the end we also provide insight about group roles and the division of work for this project.

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
`./client 2>player.log hostname port playername` or `./client 2>spectator.log hostname port` 

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
		if they are invalid, send error message and exit
	initialize the 'message' module
	send a message to the server, requesting connection
	call message_loop(), to await the server
		handleInput()
		handleMessage()
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
	prints out the OK message

#### handleGRID
	parse through the GRID message that the client receives from the server
	extract the grid size from the message
	if the current window size is too small
		inform the user to about the minimum required window size
		wait for the user to enlarge the window 

#### handleGOLD
	parse through the GOLD message that the client receives from the server
	extract `n`, `p`, and `r`, as denoted in the functional decomposition
	update the top line of display regarding the game status

#### handleDISPLAY
	parse through the DISPLAY message that the client receives from the server
	output the updated map that is displayed to the client 

#### handleQUIT
	take the QUIT message and inform the user of the quit and the reason for the quit
	break out of the message_loop() (break command may appear outside of function but called immediately after this function is called)

#### handleERROR
	print out the ERROR message received from the client
	log the error 

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
		if invalid, then send an error message and exit
	call initializeGame() to set up data structures
	initialize the 'message' module
	print the port number on which we wait
	call message_loop(), to await clients
		handleMessage() to continuously listen for message sent by clients
	call gameOver() to inform all clients the game has ended
	clean up

#### initializeGame
	create the map with the desired gold drops
	initializes a new `game` data structure and loads in the starting game data
	sets up the server connection 

#### handleMessage
	listen for messages sent from the clients that are connected by the server
	update the `game` data structure accordingly from the contents of the message
	if none of handleXYZ functions are able to process the client message
		log an error
		send back an ERROR message to the client 

#### handlePLAY
	check the validity of the player's name and whether there is still space for another player given *MaxPlayers* players
	if there is not enough space or the player's real name is invalid
		server should respond with a quit message and terminate the initialization of a new player
	else
		initialize a new `player` data structure with a random room spot location in the grid
		add the new `player` data struture to the array of players in the overarching `game` data structure
		send ok message, grid message, gold message, and display message to the new player

#### handleSPECTATE
	if there already is a spectator spectating the game
		send a a quit message to the current spectator 
		replace the current spectator with the new spectator
	else
		initialize a new `player` data structure with NULL username
		add this new `player` data structure to the array of players in the overarching `game` data structure
		send grid message, gold message, and display message to the new player

#### handleKEY
	if the keystroke came from a player
		if the keystroke is a valid keystroke for a player
			if the keystroke is a movement command
				if the player is allowed to move to the desired location
					move the player to the location and update the game accordingly (which includes updating the stored `grid` and the amount of gold remaining if the player finds a nugget)
					send a message updating all clients including the spectator
					if the goldRemaining in the game is 0
						call gameOver
			else the keystroke is to quit `Q`
				send quit message to the player
				close their port and handle their exit accordingly
		else
			send ERROR message to client 
	else the keystroke came from the spectator
		if the keystroke is `Q` (the only valid spectator keystroke)
			send quit message to the spectator 
			close spectator's port and handle spectator's exit accordingly
		else
			send ERROR message to client

#### gameOver
	server should prepare a tabular summary of the end results of the game
	send a quit message to all clients
	close the server connection

### Major data structures

There will be a static data structure, `player`, that holds:

`username`: their username

`ID`: their numerical ID

`letter`: their letter ID

`gold`: how many gold nuggets they have

`locX`: their x coordinate

`locY`: their y coordinate

`localMap`: the grid that this player sees

There will be a static global data structure, `game`, which stores important variables corresponding to information relevant to the game in both the client and the server. The `game` will hold:

`mainGrid`: the main game grid that sees everything

`numPlayers`: the number of players who have joined

`goldRemaining`: how many nuggets there are remaining in the game

`players`: an array of `player` structs

---

## the Grid module

**grid.c** provides a module that helps with the grid part for both the client and the server. 


### Functional decomposition

`grid_load`: this function takes the map file and loads the information into a grid.

`grid_1dto2d`: this function takes a one dimensional coordinate and transforms it to a two dimensional coordinate according to the size of the grid.

`grid_2dto1d`: this function takes a two dimensional coordinate and transforms it to a one dimensional coordinate according to the size of the grid.

`grid_get_loc`: this function takes a coordinate (x, y) and returns the character on the coordinate in the grid.

`grid_set_loc`: this function takes a coordinate (x, y) and a character, and replaces the character on the coordinate in the grid.




### Pseudo code for logic/algorithmic flow

#### grid_load

	read the input file line by line
	for each line
		for each character
			store the character in the string



### Major data structures

The grid itself is just a string with NRxNC characters represending the map.


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

## Group Roles & Division of Work
While all group members are responsible for the entirety of the project, we assign certain tasks for certain group members to prioritize and complete first:
* Kevin - Server module + Bresenham algorithm in Grid module
* James - Client module
* Selena - Grid module
* Steven - Server module & Grid module (particularly where both modules intersect)

We will be responsible for unit testing our individual modules in order to ensure that they function as intended, but we will conduct integration and system tests together in order to check aggregate performance. Even though we are dividing up the code, each group member should be available to help another group member on their code if necessary. 
