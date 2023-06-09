/* server.c - CS 50

Sixuan Han, Steven Mendley, and Kevin Cao, May 22 2023

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include "support/grid.h"
#include "libcs50/counters.h"
#include "libcs50/file.h"
#include "support/log.h"
#include "support/message.h"
#include "libcs50/mem.h"

/**************** structs ****************/
typedef struct player {
    char* username;
	char letterID;
	int gold;  // how many nuggets they have
	int loc;    // a 1d coordinate of the current location
    char currSpot;  // the spot type that they player's standing on
	char* localMap;  // the grid that this player can see
	addr_t address;
} player_t;


typedef struct game {
    char* mainGrid;  // the grid that contains all information (spectator's view)
	int numPlayers;  // the number of players that have joined the games
	int goldRemaining;  // the number of unclaimed nuggets
	player_t** players;  // an array of players
    addr_t spectator; // the spectator's address
	counters_t* nuggetsInPile;  // where all the gold is and how many nuggets there are in each pile
} game_t;


/**************** prototypes ****************/
static void game_new(void);
static int randRange(int min, int max);
static void game_scatter_gold(void);
static void game_delete(void);
static player_t* player_new(void);

static int parseArgs(const int argc, char* argv[], char* mapFile);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static bool handlePLAY(const addr_t from, const char* content);
static bool handleSPECTATE(const addr_t from, const char* content);
static bool handleKEY(const addr_t from, const char* content);
static bool gameOver(void);
static void broadcastDisplay(void);
static void broadcastGold(int playerIndex, int goldCollected);
static void updateVis(char* localMap, int loc);

/*********************** global ***********************/
/**************** constants ****************/
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // amount of gold in the game
static const int GoldMinNumPiles = 10; // minimum number of gold piles
static const int GoldMaxNumPiles = 30; // maximum number of gold piles
static int NR; // number of rows in the grid
static int NC; // number of columns in the grid
static game_t* game; // stores variables that provide information about each location on the grid


/**************** main ***************
 * Returns:
 *  0: all good
 *  1: fatal error occurred in message_loop
 *  2: incorrect number of arguments
 *  3: cannot read map file
 *  4: seed argument is not int
 *  5: memory allocation for the counters fails
 *  6: failure to initialize message module
*/
int main(const int argc, char* argv[])
{
    char* mapFile = NULL;

    int parseArgsReturn = parseArgs(argc, argv, mapFile);

    if (parseArgsReturn != 0) {
        return parseArgsReturn;
    }

    FILE* logFP = fopen("server.log", "w");
    int port = message_init(logFP);
    if (port == 0) {
        fprintf(stderr, "Failure to initialize message module. \n");
        fclose(logFP);
        return 6;
    }
    printf("\nServer is ready at port %i \n", port);

    bool ok = message_loop(NULL, 0, NULL, NULL, handleMessage);
    gameOver();
    message_done();
    game_delete();
    fclose(logFP);

    return ok? 0 : 1;
}






/**************** functions ****************/

/* A function to initialize a new game struct and return its pointer.
 *
 * We exit 5 if memory allocation for the counters fails.
 * 
 * Caller is responsible for:
 *   declaring the global game struct before calling this function.
 */
static void game_new(void) {
    game = mem_malloc_assert(sizeof(game_t), "Error: Memory allocation failed. \n");
    game->mainGrid = (char*)mem_malloc_assert((NR*NC + 1) * sizeof(char), "Error: Memory allocation failed. \n");
    game->numPlayers = 0;
    game->goldRemaining = GoldTotal;
    game->players = mem_malloc_assert(sizeof(player_t)*26, "Error: Memory allocation failed. \n");
    for (int i = 0; i < 26; i++) {
        game->players[i] = NULL;
    }
    game->spectator = message_noAddr();
    game->nuggetsInPile = counters_new();
    if (game->nuggetsInPile == NULL) {
        fprintf(stderr, "Memory allocation failed. \n");
        exit(5);
    }
}


/* This function will generate a random number in the range [min, max].
 */
static int randRange(int min, int max) 
{
    return min + rand() % (max-min+1);
}


/* This function will drop at least GoldMinNumPiles and at most GoldMaxNumPiles gold piles 
 * on random room spots; each pile shall have a random number of nuggets. 
 * It will store the information in nuggetsInPile and also update the mainGrid in the game struct.
 */
static void game_scatter_gold(void) 
{
    int goldRemaining = GoldTotal;
    int numPiles = randRange(GoldMinNumPiles, GoldMaxNumPiles);
    int numPilesRemaining = numPiles;
    int goldDropCoordinate;
    int goldDropNuggets;

    while (numPilesRemaining > 0) {
        goldDropCoordinate = randRange(0, NR*NC-1);
        if (game->mainGrid[goldDropCoordinate] != '.') {
            continue;
        }
        // grab a random amount of nuggets and drop until we're down to the last pile
        if (numPilesRemaining != 1) {
            // the nuggets in a pile is 50%~150% the average size or the remaining the average size
            // whichever is less
            goldDropNuggets = (randRange(GoldTotal/numPiles*0.5, GoldTotal/numPiles*1.5) < goldRemaining/numPilesRemaining) ? randRange(GoldTotal/numPiles*0.5, GoldTotal/numPiles*1.5) : goldRemaining/numPilesRemaining;
        }
        // drop all remaining nuggets when we are at the last pile
        else {
            goldDropNuggets = goldRemaining;
        }
        counters_set(game->nuggetsInPile, goldDropCoordinate, goldDropNuggets);
        game->mainGrid[goldDropCoordinate] = '*';
        goldRemaining-=goldDropNuggets;
        numPilesRemaining--;
    }
}

/* This function will clean up a game struct and everything within it.
 */
static void game_delete(void) 
{
    // free the mainGrid
    mem_free(game->mainGrid);
    // free the each player struct

    for (int i = 0; i < game->numPlayers; i++) {
        mem_free(game->players[i]->username);
        mem_free(game->players[i]->localMap);
        mem_free(game->players[i]);
    }
    // free the players array
    mem_free(game->players);
    // delete the counters
    counters_delete(game->nuggetsInPile);
    // free the game struct
    mem_free(game);
}


/* A function to initialize a new player struct and return its pointer.
 *
 * We return:
 *   a pointer to the player struct
 * 
 * Caller is responsible for:
 *   incrementing game->numPlayers after calling player_new
 *   only calling this function for players, not the spectator
 */
static player_t* player_new(void) 
{
    player_t* player = mem_malloc_assert(sizeof(player_t), "Error: Memory allocation failed. \n");

    player->localMap = (char*)mem_malloc_assert((NR*NC + 1) * sizeof(char), "Error: Memory allocation failed. \n");
    // initializing localMap to all ' '
    for (int i = 0; i < NR*NC; i++) {
            player->localMap[i] = ' ';
        }
        player->localMap[NR*NC] = '\0';

    player->letterID = 'A' + game->numPlayers;

    player->gold = 0;

    // randomly drop the player on an empty room spot
    player->loc = randRange(0, NR*NC-1);
    while (game->mainGrid[player->loc] != '.') {
        player->loc = randRange(0, NR*NC-1);
    }

    // remember the current spot that the player is standing on (in this case, '.')
    player->currSpot = game->mainGrid[player->loc];
    // update the player's map to show the player's letterID
    game->mainGrid[player->loc]=player->letterID;

    // update their local map according to their visibility
    updateVis(player->localMap, player->loc);

    return player;
}



/* A function to parse the command-line arguments, initialize the game struct, 
 * initialize the message module, and initialize analytics module.
 *
 * We return:
 *   0: all good
 *   2: incorrect number of arguments
 *   3: cannot read map file
 *   4: seed argument is not int
 * 
 * Caller is responsible for:
 *   initializing char* mapFile and int randSeed before calling this function.
 */
static int parseArgs(const int argc, char* argv[], char* mapFile)
{
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Error: incorrect number of arguments. Usage: %s map.txt [seed] \n", argv[0]);
        return 2;
    }

    mapFile = argv[1];
    FILE* fp = fopen(mapFile, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot read map file. \n");
        return 3;
    }

    // user gives the randSeed
    if (argc == 3) {
        // if the seed argument is an int
        for (int i = 0; i < strlen(argv[2]); i++) {
            if (isdigit(argv[2][i]) == 0) {
                fprintf(stderr, "Error: seed argument is not int. \n");
                return 4;
            }
        }
        srand(atoi(argv[2]));
    }
    // user does not give the randSeed
    else {
        srand(getpid());
    }
    
    // find NR and NC
    NR = file_numLines(fp);
    char* line = file_readLine(fp);
    NC = strlen(line)+1;
    free(line);
    fclose(fp);

    // initialize game
    game_new();
    fp = fopen(mapFile, "r");
    grid_load(fp, game->mainGrid, NR, NC);
    fclose(fp);
    game_scatter_gold();

    return 0;
}


/* An overarching function to handle incoming messages from a client
 * and call specific handleXYZ functions to do the jobs.
 *
 * We return:
 *   true if we want to exit message_loop; false otherwise
 * 
 * Caller is responsible for:
 *   passing a valid address
 */
static bool handleMessage(void* arg, const addr_t from, const char* message) 
{
    // logs message and sender
    log_s("Received message from %s, ", message_stringAddr(from));
    log_s("message: %s\n", message);

    if (strncmp(message, "PLAY ", strlen("PLAY ")) == 0) {
        const char* content = message + strlen("PLAY ");
        return handlePLAY(from, content);
    }
    if (strncmp(message, "SPECTATE", strlen("SPECTATE")) == 0) {
        const char* content = message + strlen("SPECTATE");
        return handleSPECTATE(from, content);
    }
    if (strncmp(message, "KEY ", strlen("KEY ")) == 0) {
        return handleKEY(from, message);
    }
    fprintf(stderr, "Error: cannot recognize message. \n");
    message_send(from, "ERROR cannot recognize message.\n");
    log_s("Sending error message to %s. \n", message_stringAddr(from));
    return false;
}


/* A function to handle incoming PLAY messages from the client
 * and add a player to the game if there is space.
 *
 * We return:
 *   false
 * 
 * Caller is responsible for:
 *   passing a valid address
 */
static bool handlePLAY(const addr_t from, const char* content) 
{
    bool isempty = true;
    char* username = (char*)mem_malloc_assert((MaxNameLength) * sizeof(char), "Error: Memory allocation failed. \n");
    for (int i = 0; i < MaxNameLength; i++) {
        if (i == strlen(content)) {
            username[i] = '\0';
            break;
        }
        if (!isspace(content[i])) {
            isempty = false;
            if (!(isblank(content[i]) || isgraph(content[i]))) {
                username[i] = '_';
            } else {
                username[i] = content[i];
            }
        } else {
            username[i] = content[i];
        }
    }
    if (isempty) {
        message_send(from, "ERROR Sorry - you must provide player's name.");
        log_s("Sending error message to %s. \n", message_stringAddr(from));
        return false;
    }

    if (game->numPlayers >= MaxPlayers) {
        message_send(from, "ERROR Game is full: no more players can join.");
        log_s("Sending error message to %s. \n", message_stringAddr(from));
        return false;
    }

    
    player_t* player = player_new();
    player->username = username;
    player->address = from;
    game->players[game->numPlayers] = player;
    game->numPlayers++;

    // allocate memory for message strings
    char* OKmessage = (char*)mem_malloc_assert(6 * sizeof(char), "Error: Memory allocation failed. \n");
    char* GRIDmessage = (char*)mem_malloc_assert((6 + ((int)(ceil(log10(NC))+1)) + ((int)(ceil(log10(NR))+1))) * sizeof(char), "Error: Memory allocation failed. \n");
    char* GOLDmessage = (char*)mem_malloc_assert((6 + ((int)(ceil(log10(2))+2)) + ((int)(ceil(log10(2))+2)) + ((int)(ceil(log10(game->goldRemaining))+2))) * sizeof(char), "Error: Memory allocation failed. \n");

    // write to message strings
    sprintf(OKmessage, "OK %c", player->letterID);
    sprintf(GRIDmessage, "GRID %d %d", NR, NC);
    sprintf(GOLDmessage, "GOLD 0 0 %d", game->goldRemaining);

    // send messages
    log_s("Sending message to %s,", message_stringAddr(from));
    message_send(from, OKmessage);
    log_s("message: %s\n", OKmessage);
    message_send(from, GRIDmessage);
    log_s("message: %s\n", GRIDmessage);
    message_send(from, GOLDmessage);
    log_s("message: %s\n", GOLDmessage);
    broadcastDisplay();

    // free memory
    mem_free(OKmessage);
    mem_free(GRIDmessage);
    mem_free(GOLDmessage);
    
    return false;
}


/* A function to handle incoming SPECTATE messages from the client
 * and add a spectator to the game if there is space.
 *
 * We return:
 *   false
 * 
 * Caller is responsible for:
 *   passing a valid address
 */
static bool handleSPECTATE(const addr_t from, const char* content) 
{
    if (strlen(content)>0) {
        fprintf(stderr, "Error: improper SPECTATE message. \n");
        message_send(from, "ERROR improper SPECTATE message");
        log_s("Sending error message to %s. \n", message_stringAddr(from));
        return false;
    }
    if (message_isAddr(game->spectator)) {
        message_send(game->spectator, "QUIT You have been replaced by a new spectator.\n");
        log_s("Sending quit message to %s \n", message_stringAddr(game->spectator));
    }
    game->spectator = from;

    // allocate memory for message strings
    char* GRIDmessage = (char*)mem_malloc_assert((6 + ((int)(ceil(log10(NC))+2)) + ((int)(ceil(log10(NR))+2))) * sizeof(char), "Error: Memory allocation failed. \n");
    char* GOLDmessage = (char*)mem_malloc_assert((6 + ((int)(ceil(log10(1))+2)) + ((int)(ceil(log10(1))+2)) + ((int)(ceil(log10(game->goldRemaining))+2))) * sizeof(char), "Error: Memory allocation failed. \n");
    char* DISPLAYmessage = (char*)mem_malloc_assert((9 + (strlen(game->mainGrid))) * sizeof(char), "Error: Memory allocation failed. \n");

    // write to message strings
    sprintf(GRIDmessage, "GRID %d %d", NR, NC);
    sprintf(GOLDmessage, "GOLD 0 0 %d", game->goldRemaining);
    sprintf(DISPLAYmessage, "DISPLAY\n%s", game->mainGrid);

    // send messages
    log_s("Sending message to %s,", message_stringAddr(from));
    message_send(from, GRIDmessage);
    log_s("message: %s\n", GRIDmessage);
    message_send(from, GOLDmessage);
    log_s("message: %s\n", GOLDmessage);
    message_send(from, DISPLAYmessage);
    log_s("message: %s\n", DISPLAYmessage);

    // free memory
    free(GRIDmessage);
    free(GOLDmessage);
    free(DISPLAYmessage);

    return false;
}


/*
*   A function to handle KEY messages and updates the game based on the 
*   the contents of the KEY message
*/
static bool handleKEY(const addr_t from, const char* content)
{
    // check to see which player the message is from
    // if playerIndex is still -1, then it must be spectator; else is player
    int playerIndex = -1;
    for (int i = 0; i < 26; i++) {
        if(game->players[i] != NULL && message_eqAddr(from, game->players[i]->address)) {
            playerIndex = i;
            break;
        }
    }

    // extract the actual key command from the contents of the message
    char key;
    sscanf(content, "KEY %c", &key);

    if (playerIndex != -1) {
        if (key == 'Q') {
            // send a QUIT message to the player who quit 
            message_send(game->players[playerIndex]->address, "QUIT Thanks for playing!\n");
            log_s("Sending quit message to %s \n", message_stringAddr(game->players[playerIndex]->address));
            
            game->mainGrid[game->players[playerIndex]->loc] = game->players[playerIndex]->currSpot;

            // free up the memory storing the player information
            mem_free(game->players[playerIndex]->username);
            mem_free(game->players[playerIndex]->localMap);
            mem_free(game->players[playerIndex]);
            game->players[playerIndex] = NULL;

            broadcastDisplay();

            // return false to keep looping
            return false; 
        }
        else if (strchr("ykuhlbjn", key) != NULL) {
            int new_loc;
            switch(key) {
                case 'y':
                    
                    // move the player up and to the left
                    new_loc = game->players[playerIndex]->loc - NC - 1;
                    break;

                case 'k':
                    // move the player upwards
                    new_loc = game->players[playerIndex]->loc - NC;
                    break;

                case 'u':

                    // move the player up and to the right
                    new_loc = game->players[playerIndex]->loc - NC + 1;
                    break;

                case 'h':

                    // move the player to the left
                    new_loc = game->players[playerIndex]->loc - 1;
                    break;

                case 'l':

                    // move the player to the right
                    new_loc = game->players[playerIndex]->loc + 1;
                    break;

                case 'b':

                    // move the player down and to the left
                    new_loc = game->players[playerIndex]->loc + NC - 1;
                    break;

                case 'j':

                    // move the player downwards
                    new_loc = game->players[playerIndex]->loc + NC;
                    break;

                case 'n':

                    // move the player down and to the right
                    new_loc = game->players[playerIndex]->loc + NC + 1;
                    break;

            }

            // check if the movement was valid
            if (game->mainGrid[new_loc] == '.' || game->mainGrid[new_loc] == '*' || game->mainGrid[new_loc] == '#'
                || isalpha(game->mainGrid[new_loc])) {

                int old_loc = game->players[playerIndex]->loc;

                // update the player's stored location
                game->players[playerIndex]->loc = new_loc;

                // check if the player steps on another player
                if (isalpha(game->mainGrid[game->players[playerIndex]->loc]))  {

                    int otherIndex = game->mainGrid[game->players[playerIndex]->loc] - 'A';

                    // update the players' locations and currSpots 
                    game->players[otherIndex]->loc = old_loc;
                    char temp = game->players[otherIndex]->currSpot;
                    game->players[otherIndex]->currSpot = game->players[playerIndex]->currSpot;
                    game->players[playerIndex]->currSpot = temp;

                    // update the visualization
                    game->mainGrid[game->players[playerIndex]->loc] = game->players[playerIndex]->letterID;
                    game->mainGrid[old_loc] = game->players[otherIndex]->letterID;

                    /* Extra Credit (Stealing nuggets) */
                    // steal nuggets from the other player
                    int stolenGold = (game->players[otherIndex]->gold < 20) ? game->players[otherIndex]->gold : 20;
                    game->players[playerIndex]->gold += stolenGold;
                    game->players[otherIndex]->gold -= stolenGold;

                    broadcastGold(playerIndex, stolenGold);
                    /* Extra Credit ends here */

                } else if (game->mainGrid[game->players[playerIndex]->loc] == '*') {
                    // check if the player moves onto a gold pile
                    // update gold
                    int goldCollected = counters_get(game->nuggetsInPile, game->players[playerIndex]->loc);
                    counters_set(game->nuggetsInPile, game->players[playerIndex]->loc, 0);

                    game->players[playerIndex]->gold += goldCollected;
                    game->goldRemaining -= goldCollected;
                    counters_set(game->nuggetsInPile, game->players[playerIndex]->loc, 0);

                    broadcastGold(playerIndex, goldCollected);

                    // update the visualization
                    game->mainGrid[game->players[playerIndex]->loc] = game->players[playerIndex]->letterID;
                    game->mainGrid[old_loc] = game->players[playerIndex]->currSpot;

                    // turn the gold pile into a regular room spot
                    game->players[playerIndex]->currSpot = '.';

                    if (game->goldRemaining == 0) {

                        // return true to exit message loop and end the game
                        return true;

                    }

                } else {

                    // otherwise just update the player's movement on the map
                    char newSpot = game->mainGrid[game->players[playerIndex]->loc];
                    game->mainGrid[game->players[playerIndex]->loc] = game->players[playerIndex]->letterID;
                    game->mainGrid[old_loc] = game->players[playerIndex]->currSpot;
                    game->players[playerIndex]->currSpot = newSpot;

                }

                broadcastDisplay();

                // return false to keep looping
                return false; 
            }
            else {

                // ignore and return false to keep looping
                return false;

            }

        } else if (strchr("YKUHLBJN", key) != NULL) {

            // tracks whether the player actually moved at least once
            bool didMove = false; 

            // keep looping (loop breaks once player can no longer go in specified direction)
            while (true) {
                int old_loc = game->players[playerIndex]->loc;
                int new_loc;
                switch(key) {

                    case 'Y':
                        
                        // move the player up and to the left
                        new_loc = game->players[playerIndex]->loc - NC - 1;
                        break;

                    case 'K':

                        // move the player upwards
                        new_loc = game->players[playerIndex]->loc - NC;
                        break;

                    case 'U':

                        // move the player up and to the right
                        new_loc = game->players[playerIndex]->loc - NC + 1;
                        break;

                    case 'H':

                        // move the player to the left
                        new_loc = game->players[playerIndex]->loc - 1;
                        break;

                    case 'L':

                        // move the player to the right
                        new_loc = game->players[playerIndex]->loc + 1;
                        break;

                    case 'B':

                        // move the player down and to the left
                        new_loc = game->players[playerIndex]->loc + NC - 1;
                        break;

                    case 'J':

                        // move the player downwards
                        new_loc = game->players[playerIndex]->loc + NC;
                        break;

                    case 'N':

                        // move the player down and to the right
                        new_loc = game->players[playerIndex]->loc + NC + 1;
                        break;

                }

                // if the player cannot move to the spot, break out of the loop
                if (game->mainGrid[new_loc] != '.' && game->mainGrid[new_loc] != '*' && game->mainGrid[new_loc] != '#'
                    && !isalpha(game->mainGrid[new_loc])) {
                    break;
                }

                // update the player's stored location
                game->players[playerIndex]->loc = new_loc;
                didMove = true;

                // check if the player steps on another player
                if (isalpha(game->mainGrid[game->players[playerIndex]->loc]))  {

                    int otherIndex = game->mainGrid[game->players[playerIndex]->loc] - 'A';

                    // update the players' locations and currSpots 
                    game->players[otherIndex]->loc = old_loc;
                    char temp = game->players[otherIndex]->currSpot;
                    game->players[otherIndex]->currSpot = game->players[playerIndex]->currSpot;
                    game->players[playerIndex]->currSpot = temp;

                    // update the visualization
                    game->mainGrid[game->players[playerIndex]->loc] = game->players[playerIndex]->letterID;
                    game->mainGrid[old_loc] = game->players[otherIndex]->letterID;

                    /* Extra Credit (Stealing nuggets) */
                    // steal nuggets from the other player
                    int stolenGold = (game->players[otherIndex]->gold < 20) ? game->players[otherIndex]->gold : 20;
                    game->players[playerIndex]->gold += stolenGold;
                    game->players[otherIndex]->gold -= stolenGold;

                    broadcastGold(playerIndex, stolenGold);
                    /* Extra Credit ends here */

                } else if (game->mainGrid[game->players[playerIndex]->loc] == '*') {

                    // check if the player moves onto a gold pile

                    // update gold
                    int goldCollected = counters_get(game->nuggetsInPile, game->players[playerIndex]->loc);
                    counters_set(game->nuggetsInPile, game->players[playerIndex]->loc, 0);

                    game->players[playerIndex]->gold += goldCollected;
                    game->goldRemaining -= goldCollected;
                    counters_set(game->nuggetsInPile, game->players[playerIndex]->loc, 0);

                    broadcastGold(playerIndex, goldCollected);

                    // update the visualization
                    game->mainGrid[game->players[playerIndex]->loc] = game->players[playerIndex]->letterID;
                    game->mainGrid[old_loc] = game->players[playerIndex]->currSpot;

                    // turn the gold pile into a regular room spot
                    game->players[playerIndex]->currSpot = '.';

                    if (game->goldRemaining == 0) {

                        // return true to exit message loop and end the game
                        return true;

                    }

                } else {

                    // otherwise simply update the player's movement on the map
                    char newSpot = game->mainGrid[game->players[playerIndex]->loc];
                    game->mainGrid[game->players[playerIndex]->loc] = game->players[playerIndex]->letterID;
                    game->mainGrid[old_loc] = game->players[playerIndex]->currSpot;
                    game->players[playerIndex]->currSpot = newSpot;

                }
            
            // only send messages if the player actually moved
                if (didMove) {
                    broadcastDisplay();
                }
            }   

            // return false to keep looping
            return false;
            
        } else {

            // if invalid keystroke command then send error message
            message_send(game->players[playerIndex]->address, "ERROR Invalid player keystroke");
            log_s("Sending error message to %s. \n", message_stringAddr(game->players[playerIndex]->address));
            
            // return false to keep looping
            return false;

        }
        
        
    } else {

        if (key == 'Q') {

            // send a QUIT message to the player who quit 
            message_send(game->spectator, "QUIT Thanks for watching!\n");
            log_s("Sending error message to %s. \n", message_stringAddr(game->spectator));

            // free up the memory storing the spectator's address
            game->spectator = message_noAddr();
            
            // return false to keep looping
            return false; 

        } else {
            // if any other keystroke send error message
            message_send(game->spectator, "ERROR Invalid spectator keystroke");
            log_s("Sending error message to %s. \n", message_stringAddr(game->spectator));

            // return false to keep looping
            return false; 
        }
    }
    return false;
}


/* A function that sends a QUIT message to every client 
* with a scoreboard when the game is over.
*
* We return:
*   true, as the loop should end after this
*/
static bool gameOver(void) 
{
    char* QUITmessage = (char*)mem_malloc_assert((17+game->numPlayers*(15+MaxNameLength)) * sizeof(char), "Error: Memory allocation failed. \n");
    int len = 0;
    len += sprintf(QUITmessage, "QUIT GAME OVER:\n");

    for (int i = 0; i < game->numPlayers; i++) {
        len += sprintf(QUITmessage + len, "%c %12d %s\n", game->players[i]->letterID, game->players[i]->gold, game->players[i]->username);
    }

    if (message_isAddr(game->spectator)) {
        message_send(game->spectator, QUITmessage);
        log_s("Sending QUIT message to %s. \n", message_stringAddr(game->spectator));
    }

    for (int i = 0; i < game->numPlayers; i++) {
        message_send(game->players[i]->address, QUITmessage);
        log_s("Sending QUIT message to %s. \n", message_stringAddr(game->players[i]->address));
    }

    free(QUITmessage);

    return true;
}


/* A function that is called when the mainGrid is updated. It updates the local maps of all players and send display messages
 * and send updated complete map to spectator if there is one
 */
static void broadcastDisplay(void) 
{
    // update the local maps of all players and send display message
    char *displayMessage = (char *)mem_malloc(8 + NR * NC + 1);
    for (int i = 0; i < 26; i++) {
        if (game->players[i] != NULL) {
            updateVis(game->players[i]->localMap, game->players[i]->loc);
            sprintf(displayMessage, "DISPLAY\n%s", game->players[i]->localMap);

            // replace the player's letterID with '@'
            displayMessage[8 + game->players[i]->loc] = '@';

            message_send(game->players[i]->address, displayMessage);
            log_s("Sending message to %s, ", message_stringAddr(game->players[i]->address));
            log_s("message: %s\n", displayMessage);
        }  
    }
    mem_free(displayMessage);

    // send updated complete map to spectator if there is one
    if (message_isAddr(game->spectator)) {
        char *displayMessage = (char *)mem_malloc(8 + NR * NC + 1);
        sprintf(displayMessage, "DISPLAY\n%s", game->mainGrid);
        message_send(game->spectator, displayMessage);
        log_s("Sending message to %s, ", message_stringAddr(game->spectator));
        log_s("message: %s\n", displayMessage);
        mem_free(displayMessage);
    }
}


/* A function that is called when someone collects gold. It sends GOLD messages to all players
 * and to spectator if there is one
 */
static void broadcastGold(int playerIndex, int goldCollected) 
{
    // send a gold message to all players
    for (int i = 0; i < 26; i++) {
        if (game->players[i] != NULL) {
            char* goldMessage = (char *)mem_malloc(25);
            if (i == playerIndex) {
                sprintf(goldMessage, "GOLD %d %d %d", goldCollected, game->players[i]->gold, game->goldRemaining);                    
            } else {
                sprintf(goldMessage, "GOLD 0 %d %d", game->players[i]->gold, game->goldRemaining);
            }

            message_send(game->players[i]->address, goldMessage);
            log_s("Sending message to %s, ", message_stringAddr(game->players[i]->address));
            log_s("message: %s\n", goldMessage);
            mem_free(goldMessage);
        }
    }

    // send a gold message if there is a spectator
    if (message_isAddr(game->spectator)) {
        char* goldMessage = (char *)mem_malloc(25);
        sprintf(goldMessage, "GOLD 0 0 %d", game->goldRemaining);
        message_send(game->spectator, goldMessage);
        log_s("Sending message to %s, ", message_stringAddr(game->spectator));
        log_s("message: %s\n", goldMessage);
        mem_free(goldMessage);
    }
}


/* update the localmap according to visibility
 */
static void updateVis(char* localMap, int loc) 
{
    // loop through and check the visibility of each coordinate in mainGrid
    for (int i = 0; i < NR * NC; i++) {
        // if the character/location is visible, then copy that to localMap to make it visible to the client
        if (game->mainGrid[i] == '\n' || grid_isVisible(game->mainGrid, i, loc, NR, NC)) {
            localMap[i] = game->mainGrid[i];
        }

        // if an occupant location is not visible, show it as an empty room spot
        else if (localMap[i] == '*' || isalpha(localMap[i])) {
            if (isalpha(localMap[i])) {
                localMap[i] = game->players[localMap[i]-'A']->currSpot;
            }
            else {
                localMap[i] = '.';
            }
        }
    }
}
