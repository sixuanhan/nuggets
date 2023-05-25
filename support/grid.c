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
void grid_load(FILE* fp, char* grid, int NR, int NC) 
{
    int i = 0;
    while (i < NR*NC) {
        char curr = (char)fgetc(fp);
        if (curr != '\n') {
            grid[i] = curr;
            i++;
        }
    }
}

int grid_1dto2d_x(int loc, int NR, int NC) 
{
    return loc%NC;
}

int grid_1dto2d_y(int loc, int NR, int NC) 
{
    return loc/NC;
}

int grid_2dto1d(int x, int y, int NR, int NC) 
{
    return x+NC*y;
}

/****************** grid_isVisible() ***********************/
/* see grid.h for more information */
bool grid_isVisible(char* grid, int start_loc, int end_loc)
{

    // converts starting location index to the 2d coordinate
    int start_x = grid_1dto2d_x(start_loc);
    int start_y = grid_1dto2d_y(start_loc);
    
    // converts ending location index to the 2d coordinate
    int end_x = grid_1dto2d_x(end_loc);
    int end_y = grid_1dto2d_y(end_loc);


    // implement modified bresenham raytracing algorithm between these 2 coordinates
    int dx = abs(start_x - end_x);
    int dy = abs(start_y - end_x);

    // sx and sy take into account which quadrant the end point is with respect to the start point
    int sx;
    int sy; 

    if (start_x > end_x) {

        sx = -1;

    } else {

        sx = 1;

    }

    if (start_y > end_y) {

        sy = -1;

    } else {

        sy = 1;

    }

    int error1 = dx - dy;

    int x = start_x; 
    int y = start_y; 
    while (true) {

        // if the ray reaches the end point then mark the point as visible by returning true
        if (x == end_x && y == end_y) {

            return true;

        }

        // if the ray reaches one of the specified characters and is not at the starting loc
        // then mark the point as not visible by returning false
        char mapChar = grid[grid_2dto1d(start_x, start_y)];
        if (x != start_x && y != start_y && (strcmp(mapChar, "-") == 0 || strcmp(mapChar, "|") == 0 || strcmp(mapChar, "+") == 0
            strcmp(mapChar, " ") == 0 || strcmp(mapChar, "#"))) {

                return false; 

        }

        // adjust the errors 
        error2 = 2 * error1;
        if (error2 > -1 * dy) {

            error1 -= dy;
            x += sx;

        }

        if (error2 < dx) {

            error1 += dx;
            y -= sy;

        }

    }

}

/********************** grid_update_vis() *********************/
/* see grid.h for more information */
void grid_update_vis(char* mainGrid, char* localMap, int loc) {

    // loop through and check the visibility of each coordinate in mainGrid
    for (int i = 0; i < strlen(mainGrid); i++) {

        // if the character/location is visible, then copy that to localMap to make it visible to the client
        if isVisible(mainGrid, i, loc) {

            strcpy(mainGrid[i], localMap[i]);

        }

    }


}