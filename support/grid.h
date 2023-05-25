/* 
 * grid.h - header file for CS50 'grid' module
 * 
 * A 'grid' is a string with NRxNC characters represending the map
 *
 * Sixuan Han and Kevin Cao, May 22 2023
 */

#include <stdbool.h>
#include "file.h"

/**************** functions ****************/

/**************** grid_load ****************/
/* This function loads the information from a file (map.txt) to a grid.
 *
 * Caller is responsible for:
 *   providing a readable file and an allocated grid;
 *   making sure that the map file is valid;
 *   the size of the file and the grid matches.
 */
void grid_load(FILE* fp, char* grid, int NR, int NC);

/**************** grid_1dto2d_x ****************/
/* This function converts a one dimensional coordinate to a two dimensional coordinate.
 *
 * We return:
 *   the x coordinate.
 * 
 * Caller is responsible for:
 *   providing a one dimensional location within the range of the grid.
 */
int grid_1dto2d_x(int loc, int NR, int NC);


/**************** grid_1dto2d_y ****************/
/* This function converts a one dimensional coordinate to a two dimensional coordinate.
 *
 * We return:
 *   the y coordinate.
 * 
 * Caller is responsible for:
 *   providing a one dimensional location within the range of the grid.
 */
int grid_1dto2d_y(int loc, int NR, int NC);


/**************** grid_2dto1d ****************/
/* This function converts a two dimensional coordinate to a one dimensional coordinate.
 *
 * We return:
 *   the one dimensional coordinate.
 * 
 * Caller is responsible for:
 *   providing a two dimensional location within the range of the grid.
 */
int grid_2dto1d(int x, int y, int NR, int NC);


/**************** grid_isVisible ****************/
/* This function checks if a point in the grid located at `end_loc` is visible from a player located at the `start_loc`.
 *
 * Caller is responsible for:
 *   providing two one dimensional locations within the range of the grid.
 */
bool grid_isVisible(char* grid, int start_loc, int end_loc);


/**************** grid_2dto1d ****************/
/* This function updates the visibility of a player according to the mainGrid, 
 * the previous localMap and the new location and updates the localMap.
 *
 * Caller is responsible for:
 *   providing a one dimensional location within the range of the grid.
 */
void grid_update_vis(char* mainGrid, char* localMap, int loc);