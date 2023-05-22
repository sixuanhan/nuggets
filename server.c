/* server.c - CS 50

Sixuan Han, Steven Mendley, and Kevin Cao, May 22 2023

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "grid.h"
#include "hashtable.h"
#include "file.h"
#include "log.h"
#include "message.h"
#include "mem.h"

/**************** prototypes ****************/
static void game_new(void);
static int randRange(int min, int max);
static void game_scatter_gold(void);
static void game_delete(void);
static player_t* player_new(void);

static int parseArgs(const int argc, char* argv[], char* mapFile);
static game_t* initializeGame(void);
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


/**************** main ****************/
int main(const int argc, char* argv[])
{
    char* mapFile;
    unsigned int randSeed;
}



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
	player_t* players[MaxPlayers];  // an array of players
    addr_t* spectator; // the spectator's address
	hashtable_t* nuggetsInPile;  // where all the gold is and how many nuggets there are in each pile
} game_t;



/**************** functions ****************/

/* A function to initialize a new game struct and return its pointer.
 *
 * We exit 4 if memory allocation for the hashtable fails.
 * 
 * Caller is responsible for:
 *   declaring the global game struct before calling this function.
 */
static void game_new(void) {
    game = mem_malloc_assert(sizeof(game_t), "Error: Memory allocation failed. \n");
    game->mainGrid = (char*)mem_malloc_assert((NR*NC + 1) * sizeof(char), "Error: Memory allocation failed. \n");
    game->mainGrid = 0;
    game->goldRemaining = GoldTotal;
    // I'm not sure if I nee to initialize players
    game->spectator = NULL;
    game->nuggetsInPile = hashtable_new(200); // 200 is the hashtable size by default
    if (game->nuggetsInPile == NULL) {
        fprintf(stderr, "Memory allocation failed. \n");
        exit(4);
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
    int numPilesRemaining = randRange(GoldMinNumPiles, GoldMaxNumPiles);
    int goldDropCoordinate;
    int goldDropNuggets;

    while (numPilesRemaining > 0) {
        goldDropCoordinate = randRange(0, NR*NC-1);
        if (game->mainGrid[goldDropCoordinate] != '.') {
            continue;
        }
        // grab a random amount of nuggets and drop until we're down to the last pile
        if (numPilesRemaining != 0) {
            goldDropNuggets = randRange(1, goldRemaining/2);
        }
        // drop all remaining nuggets when we are at the last pile
        else {
            goldDropNuggets = goldRemaining;
        }
        
        hashtable_insert(game->nuggetsInPile, goldDropCoordinate, goldDropNuggets);
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
        mem_free(game->players[i]);
    }
    // delete the hashtable
    hashtable_delete(game->nuggetsInPile, NULL);
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

    char letterID = 'A' + game->numPlayers-1;

    player->gold = 0;

    // randomly drop the player on an empty room spot
    player->loc = randRange(0, NR*NC-1);
    while (game->mainGrid[player->loc] != '.') {
        player->loc = randRange(0, NR*NC-1);
    }

    // update their local map according to their visibility
    grid_update_vis(game->mainGrid, player->localMap, player->loc, NR, NC);
}



/* A function to parse the command-line arguments, initialize the game struct, 
 * initialize the message module, and initialize analytics module.
 *
 * We return:
 *   0, if we do not exit non-zero.
 * 
 * Caller is responsible for:
 *   initializing char* mapFile and int randSeed before calling this function.
 */
static int parseArgs(const int argc, char* argv[], char* mapFile) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Error: incorrect number of arguments. Usage: ./server map.txt [seed] \n");
        exit(1);
    }

    mapFile = argv[1];
    FILE* fp = fopen(mapFile, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open map file. \n");
        exit(2);
    }

    // user gives the randSeed
    if (argc == 3) {
        // if the seed argument is an int
        if (atoi(argv[2]) == 0) {
            srand(atoi(argv[2]));
        }
        else {
            fprintf(stderr, "Error: seed argument must be int. \n");
            exit(3);
        }
    }
    // user does not give the randSeed
    else {
        srand(getpid());
    }
    

    // find NR and NC
    NR = file_numLines(fp);
    NC = strlen(file_readLine(fp));

    return 0;
}

static game_t* initializeGame(void) {

}

static bool handleMessage(void* arg, const addr_t from, const char* message) {

}

static bool handlePLAY(const addr_t from, const char* content) {

}

static bool handleSPECTATE(const addr_t from, const char* content) {

}

static bool handleKEY(const addr_t from, const char* content) {

}

static bool gameOver() {

}