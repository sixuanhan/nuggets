/* player.c - CS 50

James Quirk, May 22, 2023

*/

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
static bool handleMessage(void* arg, const addr_t from, const char* message);

int main(const int argc, char* argv[]) {
    char* serverHost;
    char* serverPort;
    addr_t serverAddress;
    char* message;
    
    if (parseArgs(argc, argv) == 1 || parseArgs(argc, argv) == 0) {
        serverHost = argv[1]; // why not assign memory?
        serverPort = argv[2];
        
        if (parseArgs(argc, argv) == 1) {
            message = "SPECTATE this";
        }

        if (parseArgs(argc, argv) == 0) {
            message = "PLAY this";
        }

        // message_setAddr(serverHost, serverPort, &serverAddress);
        // message_send(serverAddress, message); // client speaks first
        // message_loop(NULL, 0, NULL, NULL, handleMessage);
        // message_done();

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

// static bool handleInput(void* arg) {

// }

static bool handleMessage(void* arg, const addr_t from, const char* message) {
    return true;
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