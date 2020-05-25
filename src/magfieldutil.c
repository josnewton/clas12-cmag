//
//  magfieldutil.c
//  cMag
//
//  Created by David Heddle on 5/22/20.
//

#include "magfield.h"
#include "magfieldutil.h"
#include "munittest.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

//some strings for prints
const char *csLabels[] = { "cylindrical", "Cartesian" };
const char *lengthUnitLabels[] = { "cylindrical", "Cartesian" };
const char *angleUnitLabels[] = { "degrees", "radians" };
const char *fieldUnitLabels[] = { "kG", "G", "T" };

//local prototypes
static void freeGrid(GridPtr gridPtr);

/**
 * Get the magnitude of a field value.
 * @param fvPtr a pointer to a field value
 * @return return: the magnitude of a field value.
 */
double fieldMagnitude(FieldValue *fvPtr) {
    double b1 = fvPtr->b1;
    double b2 = fvPtr->b2;
    double b3 = fvPtr->b3;
    return sqrt(b1 * b1 + b2 * b2 + b3 * b3);
}
/**
 * Print a summary of the map for diagnostics and debugging.
 * @param fieldPtr the pointer to the map.
 * @param stream a file stream, e.g. stdout.
 */
void printFieldSummary(MagneticFieldPtr fieldPtr, FILE *stream) {
    FieldMapHeaderPtr headerPtr = fieldPtr->headerPtr;

    fprintf(stream, "\n========================================\n");
    fprintf(stream, "%s: [%s]\n", (fieldPtr->type == TORUS) ? "TORUS" : "SOLENOID", fieldPtr->path);
    fprintf(stream, "Created: %s\n", fieldPtr->creationDate);
    fprintf(stream, "Symmetric: %s\n", fieldPtr->symmetric ? "true" : "false");
    fprintf(stream, "scale factor: %-6.2f\n", fieldPtr->scale);

    //print the grid info for the three coordinate grids
    fprintf(stream, "%s\n", gridStr(fieldPtr->q1GridPtr));
    fprintf(stream, "%s\n", gridStr(fieldPtr->q2GridPtr));
    fprintf(stream, "%s\n", gridStr(fieldPtr->q3GridPtr));

    fprintf(stream, "num field values: %d\n", fieldPtr->numValues);
    fprintf(stream, "grid cs: %s\n", csLabels[headerPtr->gridCS]);
    fprintf(stream, "field cs: %s\n", csLabels[headerPtr->fieldCS]);
    fprintf(stream, "length unit: %s\n",
            lengthUnitLabels[headerPtr->lengthUnits]);
    fprintf(stream, "angular unit: %s\n",
            angleUnitLabels[headerPtr->angleUnits]);
    fprintf(stream, "field unit: %s\n", fieldUnitLabels[headerPtr->fieldUnits]);

    //now the metrics
    fprintf(stream, "max field at index: %d\n",
            fieldPtr->metricsPtr->maxFieldIndex);

    //get the location of the max field
    int q1index, q2index, q3index;
    getCoordinateIndices(fieldPtr, fieldPtr->metricsPtr->maxFieldIndex, &q1index, &q2index, &q3index);

    fprintf(stream, "max field magnitude: %-10.6f %s\n",
            fieldPtr->metricsPtr->maxFieldMagnitude, fieldUnits(fieldPtr));

    FieldValuePtr fieldValPtr= getFieldAtIndex(fieldPtr, fieldPtr->metricsPtr->maxFieldIndex);
    fprintf(stdout, "max field vector");
    printFieldValue(fieldValPtr, stdout);

    //get the location of the max field
    int q1Index, q2Index, q3Index;
    getCoordinateIndices(fieldPtr, fieldPtr->metricsPtr->maxFieldIndex, &q1Index, &q2Index, &q3Index);
    float phi = fieldPtr->q1GridPtr->values[q1Index];
    float rho = fieldPtr->q2GridPtr->values[q2Index];
    float z = fieldPtr->q3GridPtr->values[q3Index];
    fprintf(stdout, "max field location (phi, rho, z) = (%-6.2f, %-6.2f, %-6.2f)\n", phi, rho, z);
    fprintf(stdout, "avg field magnitude: %-10.6f %s", fieldPtr->metricsPtr->avgFieldMagnitude, fieldUnits(fieldPtr));
}

 /**
  * Get the field units of the magnetic field
  * @param fieldPtr a pointer to the field.
  * @return a string representing the field units, e.g. "kG".
  */
const char *fieldUnits(MagneticFieldPtr fieldPtr) {
    unsigned int funits = fieldPtr->headerPtr->fieldUnits;
    return fieldUnitLabels[funits];
}

 /**
  * Get the length units of the magnetic field
  * @param fieldPtr a pointer to the field.
  * @return a string representing the length units, e.g. "cm".
  */
const char *lengthUnits(MagneticFieldPtr fieldPtr) {
    int lunits = fieldPtr->headerPtr->lengthUnits;
    return lengthUnitLabels[lunits];
}

 /**
  * Print the components and magnitude of field value.
  * @param fvPtr a pointer to the field value
  * @param stream a file stream, e.g. stdout.
  */
void printFieldValue(FieldValuePtr fvPtr, FILE *stream) {
    fprintf(stream, "(%-9.5f, %-9.5f, %-9.5f), magnitude: %12.5f\n", fvPtr->b1,
            fvPtr->b2, fvPtr->b3, fieldMagnitude(fvPtr));
}

 /**
  * Allocate a field map with no content.
  * @return a pointer to an empty field map structure.
  */
MagneticFieldPtr createFieldMap() {
    MagneticFieldPtr magPtr = (MagneticFieldPtr) malloc(sizeof(MagneticField));
    magPtr->metricsPtr = (FieldMetricsPtr) malloc(sizeof(FieldMetrics));
    magPtr->scale = 1;
    magPtr->shiftX = 0;
    magPtr->shiftY = 0;
    magPtr->shiftZ = 0;
    return magPtr;
}


/**
 * Free the memory associated with a field map.
 * @param fieldPtr a pointer to the field
 */
void freeFieldMap(MagneticFieldPtr fieldPtr) {
    free(fieldPtr->metricsPtr);
    freeGrid(fieldPtr->q1GridPtr);
    freeGrid(fieldPtr->q2GridPtr);
    freeGrid(fieldPtr->q3GridPtr);
    free(fieldPtr);
}

/**
 * Free the memory associated with a coordinate grid.
 * @param gridPtr the poiner to deallocate.
 */
static void freeGrid(GridPtr gridPtr) {
    free(gridPtr->name);
    free(gridPtr->values);
    free(gridPtr);
}

/**
 * Copy a string and create the pointer
 * @param dest on input a pointer to an unallocated string.
 * On output the string will be allocated and contain a copy of src.
 * @param src the string to be copied.
 */
void stringCopy(char **dest, const char *src) {
    unsigned long len = strlen(src);
    *dest = (char*) malloc(len + 1);
    strncpy(*dest, src, len);
}

/**
 * Obtain a random int in an inclusive range[minVal, maxVal]. Used for testing.
 * @param seed a seed for the generator, or 0 for no seed.
 * @param minVal the minimum value
 * @param maxVal the maximum Value;
 * @return
 */
int randomInt(unsigned int seed, int minVal, int maxVal) {

    //seed?
    if (seed > 0) {
        srand(seed);
    }

    int del = maxVal - minVal;
    int randVal = rand();

    return minVal + (randVal % (del + 1) );
}

/**
 * A unit test for the random number generator
 * @return an error message if the test fails, or NULL if it passes.
 */
char *randomUnitTest() {

    int minVal = 0;
    int maxVal = 301;

    int count = 100000;
    bool result;

    for (int i = 0; i < count; i++) {
        int val = randomInt(0, minVal, maxVal);

        //result should be true if we pass
        bool result = (val >= minVal) && (val <= maxVal);

        if (!result) {
            fprintf(stdout, "OUT OF RANGE: val = %d range: [%d, %d] \n", val, minVal, maxVal);
        }

        mu_assert("Random number generated out of range.", result);
    }

    fprintf(stdout, "\nPASSED randomUnitTest\n");

    return NULL;
}
