/* server.c - CS 50

Sixuan Han, Steven Mendley, and Kevin Cao, May 22 2023

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "file.h"
#include "log.h"
#include "message.h"

/**************** prototypes ****************/
static int parseArgs(const int argc, char* argv[]);
static game_t* initializeGame();
static bool handleMessage(void* arg, const addr_t from, const char* message);
static bool handlePLAY(const addr_t from, const char* content);
static bool handleSPECTATE(const addr_t from, const char* content);
static bool handleKEY(const addr_t from, const char* content);
static bool gameOver();


/**************** main ****************/
int main(const int argc, char* argv[])
{

}


/*********************** global ***********************/
/**************** constants ****************/
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // amount of gold in the game
static const int GoldMinNumPiles = 10; // minimum number of gold piles
static const int GoldMaxNumPiles = 30; // maximum number of gold piles
static const int NR; // number of rows in the grid
static const int NC; // number of columns in the grid

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
	player_t* players[MaxPlayers+1];  // an array of players
	hashtable_t* nuggetsInPile;  // where all the gold is and how many nuggets there are in each pile
} game_t;



/**************** functions ****************/
static int parseArgs(const int argc, char* argv[]) {

}

static game_t* initializeGame() {

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