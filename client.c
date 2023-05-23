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

static int parseArgs(const int argc, char* argv[]);
static bool handleInput(void* arg);
static bool handleMessage(void* arg, const addr_t from, const char* message);

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
        message_loop(NULL, 0, NULL, handleInput, handleMessage);
        message_done();

    }


    return 0;
}

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
    int c;
    printf("Enter handle input\n");
    initscr(); // initialize the screen
    cbreak();  // accept keystrokes immediately
    noecho();
    
    while ((c = getch()) != 'z') {    // read one 
    printf("Got input\n");
        switch(c) {
            case 'h':   printf("move left\n"); break; // move cursor left
            case 'l':   printf("move right\n"); break; // move cursor right
            case 'j':   printf("move up\n"); break; // move cursor up
            case 'k':   printf("move down\n"); break; // move cursor down
            case 'g':   printf("move far left\n"); break; // move cursor far left
            case 'q':   return true; break; // move cursor far left

            default: if (isprint(c)) { printf("Idk\n"); }  // add character at cursor
        }
    }

    return false;
}

static bool handleMessage(void* arg, const addr_t from, const char* message) {
    printf("Handled");
    return false;
}

// static bool handleOK(const char* message) {

// }

// static bool handleGRID(const char* message) {

// }

// static bool handleGOLD(const char* message) {

// }

// static bool handleDISPLAY(const char* message) {

// }

// static bool handleQUIT(const char* message) {

// }

// static bool handleERROR(const char* message) {

// }