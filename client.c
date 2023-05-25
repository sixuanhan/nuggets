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

        printf("Enter handle input\n");
        initscr(); // initialize the screen
        cbreak();  // accept keystrokes immediately
        noecho();

        message_loop(NULL, 0, NULL, handleInput, handleMessage);
        message_done();

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
        return 1;
    }



    return 0;
}

static bool handleInput(void* arg) {
    // const addr_t* server = arg;
    int c;
    // char message[6];
    

    // // base cases modeled from miniclient
    // if (server == NULL) {
    //     fprintf(stderr, "handleInput called with arg=NULL");
    //     return true;
    // }

    // if (!message_isAddr(*server)) {
    //     fprintf(stderr, "handleInput called without a correspondent.");
    //     return true;
    // }
    
    while ((c = getch())) {
        // logs keystroke
        log_c("Received keystroke: %c\n", c);

        switch(c) {
            case 'h':   printf("move left\n"); break; // move cursor left
            case 'l':   printf("move right\n"); break; // move cursor right
            case 'j':   printf("move up\n"); break; // move cursor up
            case 'k':   printf("move down\n"); break; // move cursor down
            case 'y':   printf("move up left\n"); break; // move cursor up left
            case 'u':   printf("move up right\n"); break; // move cursor up right
            case 'b':   printf("move down left\n"); break; // move cursor down left
            case 'n':   printf("move down right\n"); break; // move cursor down right

            case 'H':   printf("move far left\n"); break; // move cursor far left
            case 'L':   printf("move far right\n"); break; // move cursor far right
            case 'J':   printf("move far up\n"); break; // move cursor far up
            case 'K':   printf("move far down\n"); break; // move cursor far down
            case 'Y':   printf("move far up left\n"); break; // move cursor far up left
            case 'U':   printf("move far up right\n"); break; // move cursor far up right
            case 'B':   printf("move far down left\n"); break; // move cursor far down left
            case 'N':   printf("move far down right\n"); break; // move cursor far down right

            case 'Q':   return true; // quit
        }
    }

    return false;
}

static bool handleMessage(void* arg, const addr_t from, const char* message) {
    // logs message and sender
    log_s("Received message from %s, ", message_stringAddr(from));
    log_s("message: %s\n", message);

    if (strncmp(message, "OK ", strlen("OK ")) == 0) {
        const char* content = message + strlen("OK ");
        return handleOK(content);
    }

    if (strncmp(message, "GRID ", strlen("GRID ")) == 0) {
        const char* content = message + strlen("SPECTATE");
        return handleGRID(content);
    }

    if (strncmp(message, "GOLD ", strlen("GOLD ")) == 0) {
        const char* content = message + strlen("KEY ");
        return handleGOLD(content);
    }

    if (strncmp(message, "DISPLAY ", strlen("DISPLAY ")) == 0) {
        const char* content = message + strlen("KEY ");
        return handleDISPLAY(content);
    }

    if (strncmp(message, "QUIT ", strlen("QUIT ")) == 0) {
        const char* content = message + strlen("KEY ");
        return handleQUIT(content);
    }

    if (strncmp(message, "ERROR ", strlen("ERROR ")) == 0) {
        const char* content = message + strlen("KEY ");
        return handleERROR(content);
    }

    fprintf(stderr, "Error: cannot recognize message. \n");
    message_send(from, "ERROR cannot recognize message");
    return false;
}

static bool handleOK(const char* message) {
    printf("%s", message);

    return false;
}

static bool handleGRID(const char* message) {
    return false;
}

static bool handleGOLD(const char* message) {
    return false;
}

static bool handleDISPLAY(const char* message) {
    return false;
}

static bool handleQUIT(const char* message) {
    return true;
}

static bool handleERROR(const char* message) {
    return false;
}