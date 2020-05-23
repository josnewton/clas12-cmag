//
//  maggrid.c
//  cMag
//
//  Created by David Heddle on 5/22/20.
//  Copyright © 2020 David Heddle. All rights reserved.
//

#include "maggrid.h"
#include "magfieldutil.h"
#include <stdlib.h>
#include <math.h>


/**
 * Create a uniform coordinate grid
 * name: the name of the coordinate, e.g. "phi"
 * minVal the min value of the grid
 * maxVal the max valueof the grid
 * num: the number of points on the grid, including the ends
 * return: a pointer to the coordinate grid
 */
GridPtr createGrid(const char *name, float minVal, float maxVal,
        unsigned int num) {
    GridPtr gridPtr;

    gridPtr = (GridPtr) malloc(sizeof(Grid));

    stringCopy(&(gridPtr->name), name);
    gridPtr->minVal = minVal;
    gridPtr->maxVal = maxVal;
    gridPtr->num = num;

    if (num < 2) {
        gridPtr->delta = INFINITY;
        gridPtr->values = NULL;
    } else {
        gridPtr->delta = (maxVal - minVal) / (num - 1);
        gridPtr->values = (float*) malloc(num * sizeof(float));

        gridPtr->values[0] = minVal;
        gridPtr->values[num - 1] = maxVal;

        for (int i = 1; i < (num - 1); i++) {
            gridPtr->values[i] = i * gridPtr->delta;
        }
    }

    return gridPtr;
}

/**
 * Get a summary string for a coordinate grid
 * grid: a pointer to a coordinate grid
 * return: a summary string
 */
char* gridStr(GridPtr grid) {
    char *str = (char*) malloc(128);

    sprintf(str, "%3s min: %6.1f  max: %6.1f  Np: %4d  delta: %6.1f",
            grid->name, grid->minVal, grid->maxVal, grid->num, grid->delta);
    return str;
}
