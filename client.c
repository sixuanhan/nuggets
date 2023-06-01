/* player.c - CS 50

James Quirk, May 22, 2023

*/

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include "counters.h"
#include "file.h"
#include "log.h"
#include "message.h"
#include "mem.h"


/*********************** structs ***********************/

typedef struct game {
    addr_t server;
    int NC;  // the number of columns in the grid
    int NR; // the number of rows in the grid
	char letter;  // the letter corresponding to this player
    bool init; // true on first loop, false otherwise
    bool spect;
    int r;
    int p;
    int n;
    FILE *log;
    char* ServerHost;
    char* ServerPort;
} game_t;

/*********************** global ***********************/

static game_t* game;

/**************** prototypes ****************/

static int parseArgs(const int argc, char* argv[]);
static bool handleInput(void* arg);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static bool handleOK(const char* message);
static bool handleGRID(const char* message);
static bool handleGOLD(const char* message);
static bool handleDISPLAY(const char* message);
static bool handleQUIT(const char* message);
static bool handleERROR(const char* message);
static void game_new(void);
static void game_delete(void);

/**************** main ***************/

int main(const int argc, char* argv[]) 
{
    char* message;

    int parsed = parseArgs(argc, argv); // 1 if player, 0 if spectaor; any other int is an error
    
    if (parsed == 1 || parsed == 0) { // if output indicates a player or spectator
        // assign host and port number
        game->ServerHost = argv[1];
        game->ServerPort = argv[2];
        
        if (parsed == 0) { // spectator case
            game->spect = true;
            message = mem_malloc_assert(9 * sizeof(char), "Error: Memory allocation failed. \n");
            sprintf(message, "SPECTATE");
        } else { // player case
            game->spect = false;
            message = mem_malloc_assert((6 + strlen(argv[3])) * sizeof(char), "Error: Memory allocation failed. \n");
            sprintf(message, "PLAY %s", argv[3]);
        }

        // initialize the message-sending modules
        message_init(stderr);
        message_setAddr(game->ServerHost, game->ServerPort, &game->server);
        message_send(game->server, message); // client speaks first
        free(message);

        // initialize the log
        FILE* fp = fopen("client.log", "w");
        flog_init(fp);
        game->server = game->server;
        game->log = fp;

        // prompt the user to take action
        printf("Enter handle input\n");

        // continue as long as the loop doesn't return true
        bool ok = message_loop(NULL, 0, NULL, handleInput, handleMessage);  

        // indicate completion, free memory
        message_done();
        flog_done(fp);
        game_delete();

        return ok? 0 : 1;
    }


    return 0;
}

/**************** functions ****************/

/* A function to ensure the arguments passed by the user are valid
 *
 * We return:
 *   an integer corresponding to the error or the type of client being used
 * 
 * Caller is responsible for:
 *   passing a valid host and port number
 */
static int parseArgs(const int argc, char* argv[]) 
{
    int port; // integer corresponding to string of port

    // if the number of arguments is incorrect
    if (argc != 3 && argc != 4) {
        // indicate failure
        fprintf(stderr, "Error: incorrect number of arguments. Format: %s hostname port [playername]\n", argv[0]);
        return 2; // return non-zero and non-one code
    }

    // turn port to an integer (0 if not an integer)
    port = atoi(argv[2]);

    // if user inputs a negative number or a string that is not converted to an integer (0)
    if (port <= 0) {
        // indicate error
        fprintf(stderr, "Error: incorrect input for port number. Must be a positive integer.\n");
        return 3;
    }

    // otherwise, initialize game
    game_new();

    // return 1 for player, 0 for spectator
    return argc-3;
}


/* A function to initialize the game structure
 *
 * We return:
 *   void
 * 
 * Caller is responsible for:
 *   nothing
 */
static void game_new(void) 
{
    game = mem_malloc(sizeof(game_t)); // allocate memory for the structure
    game->init = true; // indicate the first loop of the program
    game->n = 0; // set the value of gold collected on initialization to 0
}


/* A function that deletes and frees the game structure
 *
 * We return:
 *   void
 * 
 * Caller is responsible for:
 *   nothing
 */
static void game_delete(void) 
{
    mem_free(game);
}


/* A function that handles the user's keystrokes
 *
 * We return:
 *   false
 * 
 * Caller is responsible for:
 *   knowing the keys that correspond to actions in the game
 */
static bool handleInput(void* arg) 
{
    // store the character input by the user
    char c = getch();

    // log keystroke
    flog_c(game->log, "Received keystroke: %c\n", c);

    // send message in format "KEY <character>"
    char* KEYmessage = (char*)mem_malloc_assert(5 * sizeof(char), "Error: Memory allocation failed. \n");
    sprintf(KEYmessage, "KEY %c", c);
    message_send(game->server, KEYmessage);
    mem_free(KEYmessage);

    return false;
}


/* A function that handles the messages passed by the server
 *
 * We return:
 *   false for almost any message, true for quit message
 * 
 * Caller is responsible for:
 *   knowing the format of the messages that must be sent
 */
static bool handleMessage(void* arg, const addr_t from, const char* message) 
{
    // log message and sender
    flog_s(game->log, "Received message from %s, ", message_stringAddr(from));
    flog_s(game->log, "message: %s\n", message);

    // if message starts with this prefix, run the corresponding command
    if (strncmp(message, "OK ", strlen("OK ")) == 0) { // handleOK
        return handleOK(message);
    }

    if (strncmp(message, "GRID ", strlen("GRID ")) == 0) { // handleGRID
        return handleGRID(message);
    }

    if (strncmp(message, "GOLD ", strlen("GOLD ")) == 0) { // handleGOLD
        return handleGOLD(message);
    }

    if (strncmp(message, "DISPLAY\n", strlen("DISPLAY\n")) == 0) { // handleDISPLAY
        const char* content = message + strlen("DISPLAY\n");
        return handleDISPLAY(content);
    }

    if (strncmp(message, "QUIT ", strlen("QUIT ")) == 0) { // handleQUIT
        const char* content = message + strlen("QUIT ");

        return handleQUIT(content);
    }

    if (strncmp(message, "ERROR ", strlen("ERROR ")) == 0) { // handleERROR
        const char* content = message + strlen("ERROR ");
        return handleERROR(content);
    }

    // if nothing was returned, indicate error
    fprintf(stderr, "Error: cannot recognize message. \n");
    message_send(from, "ERROR cannot recognize message");
    return false;
}


/* A function that approves adding a new client to the game
 *
 * We return:
 *   false
 * 
 * Caller is responsible for:
 *   sending the message in the format "OK <letter>"
 */
static bool handleOK(const char* message) 
{
    // if the message format is followed
    if (sscanf(message, "OK %c", &game->letter) == 1) {
        flog_c(game->log, "OK message logs client at letter: %c\n", game->letter); // log the addition of a new client
    }

    // otherwise
    else {
        flog_s(game->log, "ERROR: handleOK cannot log '%s'\n", message); // indicate failed OK message
    }

    return false;
}


/* A function that gives the client the size of the grid
 *
 * We return:
 *   false
 * 
 * Caller is responsible for:
 *   sending the message in the format "GRID <rows> <columns>"
 */
static bool handleGRID(const char* message) 
{
    // if format matches
    if (sscanf(message, "GRID %d %d", &game->NR, &game->NC) != 2) {
        fprintf(stderr, "ERROR: Grid message could not be parsed."); // indicate grid has been stored
    }
    
    return false;
}


/* A function that handles any gold updates
 *
 * We return:
 *   false
 * 
 * Caller is responsible for:
 *   properly formatting and assigning the n, p, and r variables
 */
static bool handleGOLD(const char* message) 
{
    // if format matches
    if (sscanf(message, "GOLD %d %d %d", &game->n, &game->p, &game->r) == 3) {
        move(0,0); // set the GOLD line to be the first line on the screen
        refresh();
        if (game->n > 0) { // if some amount of gold was picked up, update message
            printw("Player %c has %d nuggets (%d nuggets unclaimed). Gold received: %d\n", game->letter, game->p, game->r, game->n);
        }
        refresh(); // update screen
    }

    // otherwise, format didn't match: print error
    else {
        flog_s(game->log, "ERROR: handleGOLD cannot log '%s'\n", message);
    }

    return false;
}


/* A function that prints the map to the screen
 *
 * We return:
 *   false
 * 
 * Caller is responsible for:
 *   properly formatting the message and the map's string
 */
static bool handleDISPLAY(const char* message) 
{
    if (game->init) { // if first loop through the game
        initscr(); // initialize the screen

        // note user screen size
        int row, col;
        getmaxyx(stdscr, row, col);

        while (row < game->NR + 1 || col < game->NC) { // as long as screen is too small
            mvprintw(0, 0, "%d: row, %d: col\n", row, col); // indicate the current size
            mvprintw(1, 0, "Window is not large enough. Enlarge to size %dx%d then press ENTER\n", game->NR + 1, game->NC); // indicate necessary size
            
            if (getch() == '\n') { // if user presses enter
                refresh();
                getmaxyx(stdscr, row, col); // scan again
            }
        }

        cbreak();  // accept keystrokes immediately
        noecho();
        game->init = false; // indicate we have initialized
    }

    // if no gold has been collected, continue
    if (game->n == 0) {
        flog_e(game->log, "No new gold\n");
        move(0, 0);
        refresh();
        if (!game->spect) { // alter gold message depending on spectator vs. player
            printw("Player %c has %d nuggets (%d nuggets unclaimed).\n", game->letter, game->p, game->r);
        } else {
            printw("Spectator: %d nuggets unclaimed.\n", game->r);
        }
    }

    // if there is new gold
    else {
        move(1, 0); // move past the first gold line in handleGOLD
        refresh();
        game->n = 0; // set collected gold to 0
    }

    // print the remainder of the message excluding 'DISPLAY'
    printw("%s", message);
    refresh();

    return false;
}


/* A function that allows a client to cleanly leave the server
 *
 * We return:
 *   true
 * 
 * Caller is responsible for:
 *   properly formatting the message and the quit reason
 */
static bool handleQUIT(const char* content) 
{
    // if the user is a player
    if (!game->spect && game->letter) {
        flog_c(game->log, "Player %c quitting... ", game->letter); // note that they are quitting
        flog_s(game->log, "Reason: %s\n", content); // note the reason why
    } else if (game->spect) { // similar process for the spectator
        flog_v(game->log, "Spectator quitting... ");
        flog_s(game->log, "Reason: %s\n", content);
    }

    // end ncurses
    endwin();

    // prints the message of reason for quitting
    printf("%s", content);
    return true;
}


/* A function that stores error messages
 *
 * We return:
 *   false
 * 
 * Caller is responsible for:
 *   passing the error message in the correct format
 */
static bool handleERROR(const char* content) 
{
    // store the error information
    flog_s(game->log, "ERROR: %s\n", content);
    move(0, 0);
    refresh();
    if (!game->spect) { // repeat the gold message despite error
        printw("Player %c has %d nuggets (%d nuggets unclaimed). %s\n", game->letter, game->p, game->r, content);
    } else {
        printw("Spectator: %d nuggets unclaimed. %s\n", game->r, content);
    }

    return false;
}
