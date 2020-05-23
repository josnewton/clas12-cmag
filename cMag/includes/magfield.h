//
//  magfield.h
//  cMag
//
//  Created by David Heddle on 5/22/20.
//

#ifndef magfield_h
#define magfield_h


#include "maggrid.h"
#include <stdio.h>
#include <stdbool.h>

#define FMDEBUG 1

// debug print
#define debugPrint(fmt, ...) \
  do { if (FMDEBUG) fprintf(stdout, fmt, __VA_ARGS__); } while (0)

//magic word used to test if byte swapping is required
#define MAGICWORD 0xced

//pointers to structures defined below
typedef struct fieldmapheader *FieldMapHeaderPtr;
typedef struct magneticfield *MagneticFieldPtr;
typedef struct fieldmetrics *FieldMetricsPtr;

//some strings for prints
extern const char *csLabels[];
extern const char *lengthUnitLabels[];
extern const char *angleUnitLabels[];
extern const char *fieldUnitLabels[];

//the header of the binary files
typedef struct fieldmapheader {
    unsigned int magicWord;   // if not 0xced = 3309 then byteswap
    unsigned int gridCS;      // 0 for cylindrical, 1 for Cartesian
    unsigned int fieldCS;     // 0 for cylindrical, 1 for Cartesian
    unsigned int lengthUnits; // 0 for cm, 1 for meters
    unsigned int angleUnits;  // 0 for degrees, 1 for radians
    unsigned int fieldUnits;  // 0 for kG, 1 for G, 2 for T
    float q1min;  // min value of slowest varying coordinate
    float q1max;  // max value of slowest varying coordinate
    unsigned int nq1; // num equally spaced in q1 direction including ends
    float q2min;  // min value of medium varying coordinate
    float q2max;  // max value of medium varying coordinate
    unsigned int nq2; // num equally spaced in q2 direction including ends
    float q3min;  // min value of fastest varying coordinate
    float q3max;  // max value of fastest varying coordinate
    unsigned int nq3; // num equally spaced in q3 direction including ends
    int cdHigh; //high word of unix creation date of map
    int cdLow; //low word of unix creation date of map
    unsigned int reserved3; //reserved
    unsigned int reserved4; //reserved
    unsigned int reserved5; //reserved
} FieldMapHeader;

//holds a single field value
typedef struct fieldvalue {
    float b1; //1st component (Bphi for solenoid, Bx for torus)
    float b2; //2nd component (Brho for solenoid, By for torus)
    float b3; //3rd component (Bz for solenoid and also for torus)
} FieldValue;

//holds some metrics of the field
typedef struct fieldmetrics {
  unsigned int maxFieldIndex; //the index where we find the max field value
  float maxFieldMagnitude; //the value of the max field index
  float maxFieldLocation[3]; //the location of the max field vector
  float avgFieldMagnitude; //the average field magnitude
} FieldMetrics;

//holds the entire field map
typedef struct magneticfield {
    FieldMapHeaderPtr headerPtr; //pointer to the header data

    char *path; //the path to the file
    char *name; //a descriptive name
    bool symmetric; //is this a symmetric grid (solenoid always is)
    char *creationDate;  //date the map was created
    int numValues;  //total number of field values
    GridPtr gridPtr[3];  //the three coordinate grids
    FieldMetricsPtr metricsPtr; //some field metrics
    
    float scale; //scale factor of the field

    //use 1D array which will require manual indexing
    FieldValue *fieldValues;
} MagneticField;

// external function prototypes
extern MagneticFieldPtr readField(char*, char*);


#endif /* magfield_h */
