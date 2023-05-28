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

int main(const int argc, char* argv[]) {
    char* message;

    int parsed = parseArgs(argc, argv);
    
    if (parsed == 1 || parsed == 0) {
        game->ServerHost = argv[1]; // why not assign memory?
        game->ServerPort = argv[2];
        
        if (parsed == 0) {
            game->spect = true;
            message = mem_malloc_assert(9 * sizeof(char), "Error: Memory allocation failed. \n");
            sprintf(message, "SPECTATE");
        } else {
            game->spect = false;
            message = mem_malloc_assert((6 + strlen(argv[3])) * sizeof(char), "Error: Memory allocation failed. \n");
            sprintf(message, "PLAY %s", argv[3]);
        }

        message_init(stderr);
        message_setAddr(game->ServerHost, game->ServerPort, &game->server);
        message_send(game->server, message); // client speaks first
        free(message);

        FILE* fp = fopen("client.log", "w");

        flog_init(fp);
        game->server = game->server;
        game->log = fp;

        printf("Enter handle input\n");

        bool ok = message_loop(NULL, 0, NULL, handleInput, handleMessage);  

        message_done();
        flog_done(fp);
        game_delete();

        return ok? 0 : 1;
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

    game_new();
    return argc-3;
}

static void game_new(void) {
    game = mem_malloc(sizeof(game_t));
    game->init = true;
    game->n = 0;
}

static void game_delete(void) {
    mem_free(game);
}

static bool handleInput(void* arg) {
    char c = getch();
    // logs keystroke
    flog_c(game->log, "Received keystroke: %c\n", c);

    char* KEYmessage = (char*)mem_malloc_assert(5 * sizeof(char), "Error: Memory allocation failed. \n");
    sprintf(KEYmessage, "KEY %c", c);
    message_send(game->server, KEYmessage);
    mem_free(KEYmessage);

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
    if (sscanf(message, "GOLD %d %d %d", &game->n, &game->p, &game->r) == 3) {
        move(0,0);
        refresh();
        if (game->n > 0) {
            printw("Player %c has %d nuggets (%d nuggets unclaimed). Gold received: %d\n", game->letter, game->p, game->r, game->n);
        }
        refresh();
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

    if (game->n == 0) {
        move(0, 0);
        refresh();
        if (!game->spect) {
            printw("Player %c has %d nuggets (%d nuggets unclaimed).\n", game->letter, game->p, game->r);
        } else {
            printw("Spectator: %d nuggets unclaimed.\n", game->r);
        }
    }

    else {
        move(1, 0);
        refresh();
        game->n = 0;
    }

    printw("%s", message);
    refresh();

    return false;
}

static bool handleQUIT(const char* content) {
    if (!game->spect && game->letter) {
        flog_c(game->log, "Player %c quitting...", game->letter);
        flog_s(game->log, "Reason: %s", content);
    } else if (game->spect) {
        flog_v(game->log, "Spectator quitting...");
        flog_s(game->log, "Reason: %s", content);
    }
    endwin();
    printf("%s", content);
    return true;
}

static bool handleERROR(const char* content) {
    flog_s(game->log, "ERROR: %s", content);
    move(0, 0);
    refresh();
    if (!game->spect) {
        printw("Player %c has %d nuggets (%d nuggets unclaimed). %s\n", game->letter, game->p, game->r, content);
    } else {
        printw("Spectator: %d nuggets unclaimed. %s\n", game->r, content);
    }

    return false;
}