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
#include "grid.h"
#include "counters.h"
#include "file.h"
#include "log.h"
#include "message.h"
#include "mem.h"

/**************** structs ****************/
typedef struct player {
    char* username;
	char letterID;
	int gold;  // how many nuggets they have
	int loc;
	char* localMap;  // the grid that this player can see
	addr_t* address;
} player_t;


typedef struct game {
    char* mainGrid;  // the grid that contains all information (spectator's view)
	int numPlayers;  // the number of players that have joined the games
	int goldRemaining;  // the number of unclaimed nuggets
	player_t** players;  // an array of players
    addr_t* spectator; // the spectator's address
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
static bool gameOver();


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

    printf("%s", game->mainGrid);

    FILE* logFP = fopen("server.log", "w");
    int port = message_init(logFP);
    if (port == 0) {
        fprintf(stderr, "Failure to initialize message module. \n");
        return 6;
    }
    printf("Server is ready at port %i. \n", port);

    bool ok = message_loop(NULL, 0, NULL, NULL, handleMessage);
    message_done();
    gameOver();
    game_delete();
  
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
    game->spectator = NULL;
    game->nuggetsInPile = counters_new();
    if (game->nuggetsInPile == NULL) {
        fprintf(stderr, "Memory allocation failed. \n");
        exit(5);
    }
}


/* This function will generate a random number in the range [min, max].
 */
static int randRange(int min, int max) {
    return min + rand() % (max-min+1);
}


/* This function will drop at least GoldMinNumPiles and at most GoldMaxNumPiles gold piles 
 * on random room spots; each pile shall have a random number of nuggets. 
 * It will store the information in nuggetsInPile and also update the mainGrid in the game struct.
 */
static void game_scatter_gold(void) {
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
            // the nuggets in a pile is 50%~150% the average size.
            goldDropNuggets = randRange(GoldTotal/numPiles*0.5, GoldTotal/numPiles*1.5);
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

    // make sure this algorithm is correct. Will delete this once we're done debugging
    if (goldRemaining != 0 || numPilesRemaining != 0) {
        printf("bug!! bug!! \n");
    }
}

/* This function will clean up a game struct and everything within it.
 */
static void game_delete(void) {
    // free the mainGrid
    mem_free(game->mainGrid);
    // free the each player struct
    for (int i = 0; i < MaxPlayers+1; i++) {
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
 *   incrementing game->numPlayers before calling player_new
 *   only calling this function for players, not the spectator
 */
static player_t* player_new(void) {
    player_t* player = mem_malloc_assert(sizeof(player_t), "Error: Memory allocation failed. \n");

    player->localMap = (char*)mem_malloc_assert((NR*NC + 1) * sizeof(char), "Error: Memory allocation failed. \n");
    // initializing localMap to all ' '
    for (int i = 0; i < NR*NC; i++) {
            player->localMap[i] = ' ';
        }
        player->localMap[NR*NC] = '\0';

    player->letterID = 'A' + game->numPlayers-1;

    player->gold = 0;

    // randomly drop the player on an empty room spot
    player->loc = randRange(0, NR*NC-1);
    while (game->mainGrid[player->loc] != '.') {
        player->loc = randRange(0, NR*NC-1);
    }

    // update their local map according to their visibility
    grid_update_vis(game->mainGrid, player->localMap, player->loc, NR, NC);

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
static int parseArgs(const int argc, char* argv[], char* mapFile) {
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
    NC = strlen(file_readLine(fp));
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
static bool handleMessage(void* arg, const addr_t from, const char* message) {
    if (strncmp(message, "PLAY ", strlen("PLAY ")) == 0) {
        const char* content = message + strlen("PLAY ");
        return handlePLAY(from, content);
    }
    if (strncmp(message, "SPECTATE", strlen("SPECTATE")) == 0) {
        const char* content = message + strlen("SPECTATE");
        return handleSPECTATE(from, content);
    }
    if (strncmp(message, "KEY ", strlen("KEY ")) == 0) {
        const char* content = message + strlen("KEY ");
        return handleKEY(from, content);
    }
    fprintf(stderr, "Error: cannot recognize message. \n");
    message_send(from, "ERROR cannot recognize message");
    return false;
}

/* A function to handle incoming PLAY messages from the client
 * and add a player to the game if there is space.
 *
 * We return:
 *   true if we want to exit the loop (too many players or invalid name)
 *   false otherwise
 * 
 * Caller is responsible for:
 *   passing a valid address
 */
static bool handlePLAY(const addr_t from, const char* content) {
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
        message_send(from, "QUIT Sorry - you must provide player's name.");
        return false;
    }

    if (game->numPlayers >= MaxPlayers) {
        message_send(from, "QUIT Game is full: no more players can join.");
        return true;
    }

    game->numPlayers++;
    player_t* player = player_new();
    game->players[game->numPlayers-1] = player;
    player->username = username;
    player->address = &from;

    // allocate memory for message strings
    char* OKmessage = (char*)mem_malloc_assert(6 * sizeof(char), "Error: Memory allocation failed. \n");
    char* GRIDmessage = (char*)mem_malloc_assert((6 + ((int)(ceil(log10(NC))+1)) + ((int)(ceil(log10(NR))+1))) * sizeof(char), "Error: Memory allocation failed. \n");
    char* GOLDmessage = (char*)mem_malloc_assert((6 + ((int)(ceil(log10(1))+1)) + ((int)(ceil(log10(1))+1)) + ((int)(ceil(log10(game->goldRemaining))+1))) * sizeof(char), "Error: Memory allocation failed. \n");
    char* DISPLAYmessage = (char*)mem_malloc_assert((9 + (strlen(player->localMap))) * sizeof(char), "Error: Memory allocation failed. \n");

    // write to message strings
    sprintf(OKmessage, "OK %c", player->letterID);
    sprintf(GRIDmessage, "GRID %d %d", NC, NR);
    sprintf(GOLDmessage, "GOLD 0 0 %d", game->goldRemaining);
    sprintf(DISPLAYmessage, "DISPLAY\n%s", player->localMap);

    // send messages
    message_send(from, OKmessage);
    message_send(from, GRIDmessage);
    message_send(from, GOLDmessage);
    message_send(from, DISPLAYmessage);

    // free memory
    free(OKmessage);
    free(GRIDmessage);
    free(GOLDmessage);
    free(DISPLAYmessage);
    
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
static bool handleSPECTATE(const addr_t from, const char* content) {
    if (strlen(content)>1) {
        fprintf(stderr, "Error: improper SPECTATE message. \n");
        message_send(from, "ERROR improper SPECTATE message");
    }
    if (game->spectator != NULL) {
        message_send(*game->spectator, "QUIT You have been replaced by a new spectator.");
    }
    game->spectator = &from;

    // allocate memory for message strings
    char* GRIDmessage = (char*)mem_malloc_assert((6 + ((int)(ceil(log10(NC))+2)) + ((int)(ceil(log10(NR))+2))) * sizeof(char), "Error: Memory allocation failed. \n");
    char* GOLDmessage = (char*)mem_malloc_assert((6 + ((int)(ceil(log10(1))+2)) + ((int)(ceil(log10(1))+2)) + ((int)(ceil(log10(game->goldRemaining))+2))) * sizeof(char), "Error: Memory allocation failed. \n");
    char* DISPLAYmessage = (char*)mem_malloc_assert((9 + (strlen(game->mainGrid))) * sizeof(char), "Error: Memory allocation failed. \n");

    // write to message strings
    sprintf(GRIDmessage, "GRID %d %d", NC, NR);
    sprintf(GOLDmessage, "GOLD 0 0 %d", game->goldRemaining);
    sprintf(DISPLAYmessage, "DISPLAY\n%s", game->mainGrid);

    // send messages
    message_send(from, GRIDmessage);
    message_send(from, GOLDmessage);
    message_send(from, DISPLAYmessage);

    // free memory
    free(GRIDmessage);
    free(GOLDmessage);
    free(DISPLAYmessage);

    return false;
}

static bool handleKEY(const addr_t from, const char* content) {
    return false;
}

/* A function that sends a QUIT message to every client 
* with a scoreboard when the game is over.
*
* We return:
*   true, as the loop should end after this
*/
static bool gameOver() {
    char* QUITmessage = (char*)mem_malloc_assert((17+game->numPlayers*(13+MaxNameLength)) * sizeof(char), "Error: Memory allocation failed. \n");
    strcpy(QUITmessage, "QUIT GAME OVER:\n");

    for (int i = 0; i < game->numPlayers; i++) {
        sprintf(QUITmessage, "%c %12d %s\n", game->players[i]->letterID, game->players[i]->gold, game->players[i]->username);
    }

    if (game->spectator != NULL) {
        message_send(*game->spectator, QUITmessage);
    }

    for (int i = 0; i < game->numPlayers; i++) {
        message_send(*game->players[i]->address, QUITmessage);
    }

    free(QUITmessage);

    return true;
}