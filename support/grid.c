/* grid.c - CS 50
This module implements a grid, which is a string with NRxNC characters represending the map,
and related functions to help the server.

Sixuan Han and Kevin Cao, May 22 2023

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "../libcs50/file.h"


/**************** functions ****************/
void grid_load(FILE* fp, char* grid, int NR, int NC) 
{
    int i = 0;
    while (i < NR*NC) {
        char curr = (char)fgetc(fp);
        grid[i] = curr;
        i++;
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
bool grid_isVisible(char* grid, int start_loc, int end_loc, int NR, int NC)
{

    // converts starting location index to the 2d coordinate
    int start_x = grid_1dto2d_x(start_loc, NR, NC);
    int start_y = grid_1dto2d_y(start_loc, NR, NC);
    
    // converts ending location index to the 2d coordinate
    int end_x = grid_1dto2d_x(end_loc, NR, NC);
    int end_y = grid_1dto2d_y(end_loc, NR, NC);

    int dx = start_x - end_x;
    int dy = start_y - end_y;
    float slope;

    int start = (dx > 0) ? 1 : -1;
 
    if (dx != 0) {

        slope = ((float) dy) / ((float) dx);

        // loop and increment the ray through just its x component
        while (abs(start) < abs(dx)) {

            // update the y as we trace the ray
            float y_intercept = ((float) start) * slope + ((float) end_y);
            
            // if the ray lands directly on a cell
            if (y_intercept == round(y_intercept)) {

                // check if the ray can traverse through the point
                // char mapChar = grid[grid_2dto1d(end_x + start, (int) y_intercept, NR, NC)];
                char mapChar = grid[grid_2dto1d(end_x + start, round(y_intercept + 0.01f), NR, NC)];
                if (mapChar != '.' && mapChar != '*' && !isalpha(mapChar)) {

                    return false;

                }

            } else {

                // if the ray is passing between cells

                // check the transparency of the cells that the ray is sandwiched between
                char overChar = grid[grid_2dto1d(end_x + start, round(y_intercept - 0.5f), NR, NC)];
                char underChar = grid[grid_2dto1d(end_x + start, round(y_intercept + 0.5f), NR, NC)];

                if (overChar != '.' && overChar != '*' && !isalpha(overChar) && underChar != '.'
                    && underChar != '*' && !isalpha(underChar)) {

                    return false;

                }

            }

            // increment the x
            start += (dx > 0) ? 1 : -1;

        }

    }

    // now isolate the y component instead when raytracing

    start = (dy > 0) ? 1 : -1;

    if (dy != 0) {

        slope = ((float) dx) / ((float) dy);

        while (abs(start) < abs(dy)) {

            // update the x as we trace the ray
            float x_intercept = ((float) start) * slope + ((float) end_x);

            // if the ray lands directly on a cell
            if (x_intercept == round(x_intercept)) {

                // check if the ray can traverse through the point
                char mapChar = grid[grid_2dto1d((round(x_intercept + 0.01f), end_y + start, NR, NC)];
                if (mapChar != '.' && mapChar != '*' && !isalpha(mapChar)) {

                    return false;

                }

            } else {

                // if the ray is passing between cells

                // check the transparency of the cells that the ray is sandwiched between
                char leftChar = grid[grid_2dto1d(round(x_intercept - 0.5f), end_y + start, NR, NC)];
                char rightChar = grid[grid_2dto1d(round(x_intercept + 0.5f), end_y + start, NR, NC)];
                if (leftChar != '.' && leftChar != '*' && !isalpha(leftChar) && rightChar != '.'
                    && rightChar != '*' && !isalpha(rightChar)) {

                    return false;

                }

            }

            // increment the y
            start += (dy > 0) ? 1 : -1;

        }

    }

    return true;

    /*
    // implement modified bresenham raytracing algorithm between these 2 coordinates
    int dx = abs(start_x - end_x);
    int dy = abs(start_y - end_y);

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
        char mapChar = grid[grid_2dto1d(x, y, NR, NC)];
        if ((x != start_x || y != start_y) && (mapChar == '-' || mapChar == '|' || mapChar == '+' 
            || mapChar == ' ' || mapChar == '#')) {
            return false; 
        }

        // adjust the errors 
        int error2 = 2 * error1;
        if (error2 > -1 * dy) {
            error1 -= dy;
            x += sx;
        }
        if (error2 < dx) {
            error1 += dx;
            y += sy;
        }
    }
    */

}

/********************** grid_update_vis() *********************/
/* see grid.h for more information */
void grid_update_vis(char* mainGrid, char* localMap, int loc, int NR, int NC) 
{
    // loop through and check the visibility of each coordinate in mainGrid
    for (int i = 0; i < NR * NC; i++) {

        if (mainGrid[i] == '\n') {
            continue;
        }
        
        // if the character/location is visible, then copy that to localMap to make it visible to the client
        if (grid_isVisible(mainGrid, i, loc, NR, NC)) {

            localMap[i] = mainGrid[i];

        }
        // if an occupant location is not visible, show it as an empty room spot
        else if (localMap[i] == '*' || isalpha(localMap[i])) {

            localMap[i] = '.';

        }

    }
}
