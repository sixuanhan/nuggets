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


/*************** grid_load() *******************/
/* see grid.h for more information */
void grid_load(FILE* fp, char* grid, int NR, int NC) 
{
    int i = 0;
    while (i < NR*NC) {
        char curr = (char)fgetc(fp);
        grid[i] = curr;
        i++;
    }
}

/*************** grid_1dto2d_x() *******************/
/* see grid.h for more information */
int grid_1dto2d_x(int loc, int NR, int NC) 
{
    return loc%NC;
}

/*************** grid_1dto2d_y() *******************/
/* see grid.h for more information */
int grid_1dto2d_y(int loc, int NR, int NC) 
{
    return loc/NC;
}

/*************** grid_2dto1d() *******************/
/* see grid.h for more information */
int grid_2dto1d(int x, int y, int NR, int NC) 
{
    return x+NC*y;
}

/*************** grid_isVisible() *******************/
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

    /* Extra Credit (Vision Limits) */
    // sets a range limit on vision (a diameter of five spots)
    if (sqrt(dx * dx + dy * dy) > 2.5) {

        return false;
            
    }
    /* Extra Credit ends here */

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
                char mapChar = grid[grid_2dto1d(end_x + start, round(y_intercept), NR, NC)];
                if (mapChar != '.' && mapChar != '*' && !isalpha(mapChar)) {
                    return false;
                }
            } else {
                // if the ray is passing between cells
                // check the transparency of the cells that the ray is sandwiched between
                char overChar = grid[grid_2dto1d(end_x + start, floor(y_intercept), NR, NC)];
                char underChar = grid[grid_2dto1d(end_x + start, ceil(y_intercept), NR, NC)];

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
                char mapChar = grid[grid_2dto1d(round(x_intercept), end_y + start, NR, NC)];
                if (mapChar != '.' && mapChar != '*' && !isalpha(mapChar)) {
                    return false;
                }
            } else {
                // if the ray is passing between cells
                // check the transparency of the cells that the ray is sandwiched between
                char leftChar = grid[grid_2dto1d(floor(x_intercept), end_y + start, NR, NC)];
                char rightChar = grid[grid_2dto1d(ceil(x_intercept), end_y + start, NR, NC)];
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
}
