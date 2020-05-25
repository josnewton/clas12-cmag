//
//  maggrid.h
//  cMag
//
//  Created by David Heddle on 5/22/20.
//  Copyright © 2020 David Heddle. All rights reserved.
//

#ifndef maggrid_h
#define maggrid_h

#include <stdio.h>

typedef struct grid *GridPtr;

/**
 * Holds the uniformly spaced grid values for a coordinate.
 */
typedef struct grid {
        char *name;    //the name of the coordinate, e.g., "phi".
        float minVal;  //the min value of the coordinate
        float maxVal;  //the max value of the coordinate
        unsigned int num; //the number of vals including the ends
        float delta; // (max-min)/(n-1)
        float *values; //values of the coordinates
} Grid;

//external prototypes
extern GridPtr createGrid(const char*, float, float, unsigned int);
extern char *gridStr(GridPtr);
float valueAtIndex(GridPtr, int);
char *gridUnitTest(void);

#endif /* maggrid_h */
