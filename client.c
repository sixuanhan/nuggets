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
    bool init;
    FILE *log;
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

int main(const int argc, char* argv[]) {
    char* serverHost;
    char* serverPort;
    addr_t serverAddress;
    char* message;

    int parsed = parseArgs(argc, argv);
    
    if (parsed == 1 || parsed == 0) {
        serverHost = argv[1]; // why not assign memory?
        serverPort = argv[2];
        
        if (parsed == 0) {
            message = "SPECTATE";
        }

        if (parsed == 1) {
            message = "PLAY this";
        }

        message_init(stderr);
        message_setAddr(serverHost, serverPort, &serverAddress);
        message_send(serverAddress, message); // client speaks first

        FILE* fp = fopen("client.log", "w");

        flog_init(fp);
        game->server = serverAddress;
        game->log = fp;

        printf("Enter handle input\n");

        message_loop(NULL, 0, NULL, handleInput, handleMessage);
        message_done();
        flog_done(fp);
        game_delete();

    }


    return 0;
}

/**************** functions ****************/

static int parseArgs(const int argc, char* argv[]) {
    int port;

    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Error: incorrect number of arguments. Format: %s hostname port [playername]\n", argv[0]);
        return 2;
    }

    port = atoi(argv[2]);

    if (port <= 0) {
        fprintf(stderr, "Error: incorrect input for port number. Must be a positive integer.\n");
        return 3;
    }

    if (argc == 4) {
        game_new();
        return 1;
    }


    game_new();
    return 0;
}

static void game_new(void) {
    game = mem_malloc(sizeof(game_t));
    game->init = true;
}

static void game_delete(void) {
    mem_free(game);
}

static bool handleInput(void* arg) {
    int c;    
    
    while ((c = getch())) {
        // logs keystroke
        flog_c(game->log, "Received keystroke: %c\n", c);

        switch(c) {
            case 'h':   message_send(game->server, "KEY h"); break; // move cursor left
            case 'l':   message_send(game->server, "KEY l"); break; // move cursor right
            case 'j':   message_send(game->server, "KEY j"); break; // move cursor up
            case 'k':   message_send(game->server, "KEY k"); break; // move cursor down
            // case 'y':   message_send(*server, "KEY y"); break; // move cursor up left
            // case 'u':   message_send(*server, "KEY u"); break; // move cursor up right
            // case 'b':   message_send(*server, "KEY b"); break; // move cursor down left
            // case 'n':   message_send(*server, "KEY n"); break; // move cursor down right

            // case 'H':   message_send(*server, "KEY H"); break; // move cursor far left
            // case 'L':   message_send(*server, "KEY L"); break; // move cursor far right
            // case 'J':   message_send(*server, "KEY J"); break; // move cursor far up
            // case 'K':   message_send(*server, "KEY K"); break; // move cursor far down
            // case 'Y':   message_send(*server, "KEY Y"); break; // move cursor far up left
            // case 'U':   message_send(*server, "KEY U"); break; // move cursor far up right
            // case 'B':   message_send(*server, "KEY B"); break; // move cursor far down left
            // case 'N':   message_send(*server, "KEY N"); break; // move cursor far down right

            case 'Q':   message_send(game->server, "KEY Q"); break; // quit
        }

    }

    return false;
}

static bool handleMessage(void* arg, const addr_t from, const char* message) {
    // logs message and sender

    flog_s(game->log, "Received message from %s, ", message_stringAddr(from));
    flog_s(game->log, "message: %s\n", message);

    if (strncmp(message, "OK ", strlen("OK ")) == 0) {
        return handleOK(message);
    }

    if (strncmp(message, "GRID ", strlen("GRID ")) == 0) {
        return handleGRID(message);
    }

    if (strncmp(message, "GOLD ", strlen("GOLD ")) == 0) {
        return handleGOLD(message);
    }

    if (strncmp(message, "DISPLAY\n", strlen("DISPLAY\n")) == 0) {
        const char* content = message + strlen("DISPLAY\n");
        return handleDISPLAY(content);
    }

    if (strncmp(message, "QUIT ", strlen("QUIT ")) == 0) {
        const char* content = message + strlen("QUIT ");
        return handleQUIT(content);
    }

    if (strncmp(message, "ERROR ", strlen("ERROR ")) == 0) {
        const char* content = message + strlen("ERROR ");
        return handleERROR(content);
    }

    fprintf(stderr, "Error: cannot recognize message. \n");
    message_send(from, "ERROR cannot recognize message");
    return false;
}

static bool handleOK(const char* message) {
    if (sscanf(message, "OK %c", &game->letter) == 1) {
        flog_c(game->log, "OK message logs client at letter: %c", game->letter);
    }

    else {
        flog_s(game->log, "ERROR: handleOK cannot log '%s'", message);
    }

    return false;
}

static bool handleGRID(const char* message) {
    if (sscanf(message, "GRID %d %d", &game->NR, &game->NC) != 2) { // REVISIT: why problem?
        fprintf(stderr, "ERROR: Grid message could not be parsed.");
    }
    
    return false;
}

static bool handleGOLD(const char* message) {
    int n; // number collected on move
    int p; // in purse
    int r; // remaining

    if (sscanf(message, "GOLD %d %d %d", &n, &p, &r) == 3) {
        // printf("Player %c has %d nuggets (%d nuggets remaining). Gold received: %d\n", game->letter, p, r, n);
    }

    else {
        flog_s(game->log, "ERROR: handleGOLD cannot log '%s'", message);
    }

    return false;
}

static bool handleDISPLAY(const char* message) {
    if (game->init) {
        initscr(); // initialize the screen
        cbreak();  // accept keystrokes immediately
        noecho();
        game->init = false;
    }

    mvprintw(1, 0, message);

    return false;
}

static bool handleQUIT(const char* content) {
    flog_c(game->log, "Player %c quitting...", game->letter);
    flog_s(game->log, "Reason: %s", content);
    
    return true;
}

static bool handleERROR(const char* content) {
    flog_s(game->log, "ERROR: %s", content);

    return false;
}