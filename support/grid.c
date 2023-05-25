/* grid.c - CS 50
This module implements a grid, which is a string with NRxNC characters represending the map,
and related functions to help the server.

Sixuan Han and Kevin Cao, May 22 2023

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../libcs50/file.h"


/**************** functions ****************/
void grid_load(FILE* fp, char* grid, int NR, int NC) {
    int i = 0;
    while (i < NR*NC) {
        char curr = (char)fgetc(fp);
        grid[i] = curr;
        i++;
    }
}

int grid_1dto2d_x(int loc, int NR, int NC) {
    return loc%NC;
}

int grid_1dto2d_y(int loc, int NR, int NC) {
    return loc/NC;
}

int grid_2dto1d(int x, int y, int NR, int NC) {
    return x+NC*y;
}

bool grid_isVisible(char* grid, int start_loc, int end_loc) {
    return true;
}

void grid_update_vis(char* mainGrid, char* localMap, int loc, int NR, int NC) {

}