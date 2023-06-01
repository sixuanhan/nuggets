# CS50 Nuggets
## Design Spec
### THEOhioStateUniversity, Spring 2023

**Group Members**:
* Kevin Cao
* James Quirk
* Selena Han
* Steven Mendley

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes the grid module.
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
./client hostname port [playername] 2>client.log
```

When the game is running, it can also process user keyboard commands that allow the user to interact with the game. The exact list of valid keystrokes for the user depends on whether the user is a spectator or a player, and is also listed out in the requirements spec. While the game can still technically run without logging our stderr output, the game should be run with it.

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
	shut down when the client tells it to
	clean up

#### handleInput
	read the keystroke from stdin
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

`broadcastDisplay`: Tells everyone to update their display.

`broadcastGold`: Tells everyone to update when someone collects gold.



### Pseudo code for logic/algorithmic flow

#### main
The server will run as follows:

   	execute from a command line per the requirement spec
	parse the command line, validate parameters
		if invalid, then send an error message and exit
	set up data structures
	initialize the 'message' module
	print the port number on which we wait
	call message_loop(), to await clients
		handleMessage() to continuously listen for message sent by clients
	clean up

#### other functions

We have detailed pseudocodes for other functions in [Implementation Spec](IMPLEMENTATION.md).


### Major data structures

There will be a static data structure, `player`, that holds:

`username`: their username

`letterID`: their letter ID

`gold`: how many gold nuggets they have

`loc`: their one dimensional coordinate

`localMap`: the grid that this player sees

`address`: their address

There will be a static global data structure, `game`, which stores important variables corresponding to information relevant to the game in both the client and the server. The `game` will hold:

`mainGrid`: the main game grid that sees everything

`numPlayers`: the number of players who have joined

`goldRemaining`: how many nuggets there are remaining in the game

`players`: an array of `player` structs

`spectator`: the address of the spectator

`nuggetsInPile`: a counters that holds the one dimensional coordinate of all piles and the number of nuggets in each pile

---

## the Grid module

**grid.c** provides a module that helps with the grid part for both the client and the server. 


### Functional decomposition

`grid_load`: this function takes the map file and loads the information into a grid.

`grid_1dto2d_x`: this function takes a one dimensional coordinate, transforms it to a two dimensional coordinate according to the size of the grid, and returns the x value.

`grid_1dto2d_y`: this function takes a one dimensional coordinate, transforms it to a two dimensional coordinate according to the size of the grid, and returns the y value.

`grid_2dto1d`: this function takes a two dimensional coordinate and transforms it to a one dimensional coordinate according to the size of the grid.

`grid_isVisible`: This function checks if a point in the grid located at `end_loc` is visible from a player located at the `start_loc`.

`grid_update_vis`: this function is called when a player moves. It takes a coordinate, a player's local grid, and the main game grid, and rewrites the player's new local grid given their updated position. *Extra credit*: We set a range limit on vision of a diameter of five spots.


### Pseudo code for logic/algorithmic flow

See [Implementation Spec](IMPLEMENTATION.md).



### Major data structures

The grid itself is just a string with NRxNC characters represending the map.


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

## Group Roles & Division of Work
While all group members are responsible for the entirety of the project, we assign certain tasks for certain group members to prioritize and complete first:
* Kevin - handleKEY in server.c and visibility algorithm in grid.c
* James - client.c
* Selena - grid.c, main, parsearg, and handleMessage in server.c
* Steven - handlePLAY, handleSPECTATE, and gameOver in server.c, specs and scrum management

We will be responsible for unit testing our individual modules in order to ensure that they function as intended, but we will conduct integration and system tests together in order to check aggregate performance. Even though we are dividing up the code, each group member should be available to help another group member on their code if necessary. 
