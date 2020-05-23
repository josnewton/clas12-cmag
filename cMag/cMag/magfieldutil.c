//
//  magfieldutil.c
//  cMag
//
//  Created by David Heddle on 5/22/20.
//

#include "magfield.h"
#include "magfieldutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//some strings for prints
const char *csLabels[] = { "cylindrical", "Cartesian" };
const char *lengthUnitLabels[] = { "cylindrical", "Cartesian" };
const char *angleUnitLabels[] = { "degrees", "radians" };
const char *fieldUnitLabels[] = { "kG", "G", "T" };

/**
 * Get the magnitude of a field value
 * fvPtr: a pointer to a field value
 * return: the magnitude of a field value
 */
float fieldMagnitude(FieldValue *fvPtr) {
    double b1 = fvPtr->b1;
    double b2 = fvPtr->b2;
    double b3 = fvPtr->b3;
    return sqrt(b1 * b1 + b2 * b2 + b3 * b3);
}

/**
 * Print a diagnostic summary of a field
 * magPtr: a pointer to a magnetics field
 * stream: a file stream, e.g. stdout
 */
void printFieldSummary(MagneticFieldPtr magPtr, FILE *stream) {
    FieldMapHeaderPtr headerPtr = magPtr->headerPtr;

    fprintf(stream, "\n%s: [%s]\n", magPtr->name, magPtr->path);
    fprintf(stream, "Created: %s\n", magPtr->creationDate);
    fprintf(stream, "Symmetric: %s\n", magPtr->symmetric ? "true" : "false");

    //print the grid info
    for (int i = 0; i < 3; i++) {
        fprintf(stream, "%s\n", gridStr(magPtr->gridPtr[i]));
    }

    fprintf(stream, "num field values: %d\n", magPtr->numValues);
    fprintf(stream, "grid cs: %s\n", csLabels[headerPtr->gridCS]);
    fprintf(stream, "field cs: %s\n", csLabels[headerPtr->fieldCS]);
    fprintf(stream, "length unit: %s\n",
            lengthUnitLabels[headerPtr->lengthUnits]);
    fprintf(stream, "angular unit: %s\n",
            angleUnitLabels[headerPtr->angleUnits]);
    fprintf(stream, "field unit: %s\n", fieldUnitLabels[headerPtr->fieldUnits]);

    //now the metrics
    fprintf(stream, "max field at index: %d\n",
            magPtr->metricsPtr->maxFieldIndex);
    fprintf(stream, "max field magnitude: %10.6f %s\n",
            magPtr->metricsPtr->maxFieldMagnitude, fieldUnits(magPtr));
}

/**
 * Get the field units of the magnetic field
 * magPtr: a pointer to the field
 * return: a string representing the field units, e.g. "kG"
 */
const char *fieldUnits(MagneticFieldPtr magPtr) {
    int funits = magPtr->headerPtr->fieldUnits;
    return fieldUnitLabels[funits];
}

/**
 * Get the length units of the magnetic field
 * magPtr: a pointer to the field
 * return: a string representing the length units, e.g. "cm"
 */
const char *lengthUnits(MagneticFieldPtr magPtr) {
    int lunits = magPtr->headerPtr->lengthUnits;
    return lengthUnitLabels[lunits];
}

/**
 * Print the components and magnitude of field value
 * fvPtr: a pointer to a FieldValue structure
 * stream: a file stream, e.g. stdout
 */
void printFieldValue(FieldValue *fvPtr, FILE *stream) {
    fprintf(stream, "(%12.5f, %12.5f, %12.5f), [%12.5f]\n", fvPtr->b1,
            fvPtr->b2, fvPtr->b3, fieldMagnitude(fvPtr));
}

/**
 *  Allocate a field map with no content
 *  return: a pointer to an empty field map structure
 */
MagneticFieldPtr createFieldMap() {
    MagneticFieldPtr magPtr = (MagneticFieldPtr) malloc(sizeof(MagneticField));
    magPtr->metricsPtr = (FieldMetricsPtr) malloc(sizeof(FieldMetrics));
    return magPtr;
}

/**
 * Free the memory associated with a field map
 * mapPtr: a pointer to the field
 */
void freeFieldMap(MagneticFieldPtr magPtr) {
    free(magPtr->metricsPtr);
    free(magPtr);

    for (int i = 0; i < 3; i++) {
        free(magPtr->gridPtr[i]->name);
        free(magPtr->gridPtr[i]->values);
        free(magPtr->gridPtr[i]);
    }
}

/**
 * Copy a string and create the pointer
 * dest: will become the pointer to the new string copy
 * src: the string being copied
 */
void stringCopy(char **dest, const char *src) {
    unsigned long len = strlen(src);
    *dest = (char*) malloc(len + 1);
    strcpy(*dest, src);
}


