//
//  magfield.c
//  cMag
//
//  Created by David Heddle on 5/22/20.
//

#include "magfield.h"
#include "magfieldutil.h"
#include "munittest.h"
#include "maggrid.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

//coordinate names
static const char *q1Names[] = { "phi", "x" };
static const char *q2Names[] = { "rho", "y" };
static const char *q3Names[] = { "z", "z" };

//local prototypes
static FieldMapHeaderPtr readMapHeader(FILE*);
static long getFileSize(FILE*);
static void swap32(char*, int);
static char* getCreationDate(FieldMapHeaderPtr);

static void computeFieldMetrics(MagneticFieldPtr);

//do we have to swap bytes?
//since the fields were produced by Java which uses
//new format (BigEndian) we probably will have to swap.
static bool swapBytes = false;

//this is used by the minimal unit testing
int mtests_run = 0;

/**
 * Read a binary field map at the given location
 * path: the full path to a field map file
 * name: a descriptive name of the map, e.g., "TORUS"
 * return: NULL on failure, the field pointer on success.
 */
MagneticFieldPtr readField(char *path, char *name) {

    debugPrint("\nAttempting to read field map from [%s]\n", path);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return NULL;
    }

    //get the header
    FieldMapHeaderPtr headerPtr = readMapHeader(file);
    if (headerPtr == NULL) {
        fclose(file);
        return NULL;
    }

    MagneticFieldPtr fieldPtr = createFieldMap();

    //copy the path and name
    stringCopy(&(fieldPtr->path), path);
    stringCopy(&(fieldPtr->name), name);

    fieldPtr->headerPtr = headerPtr;
    fieldPtr->numValues = headerPtr->nq1 * headerPtr->nq2 * headerPtr->nq3;
    fieldPtr->creationDate = getCreationDate(headerPtr);

    debugPrint("map creation date: %s\n", fieldPtr->creationDate);

    //malloc the data array
    fieldPtr->fieldValues = malloc(fieldPtr->numValues * sizeof(FieldValue));

    //did we have enough memory?
    if (fieldPtr->fieldValues == NULL) {
        fprintf(stderr, "\nOut of memory when allocating space for field map.");
        fclose(file);
        return NULL;
    }

    //now we can read the field. Reading the header should have left
    //the file pointer positioned at the right spot.
    fread(fieldPtr->fieldValues, sizeof(FieldValue), fieldPtr->numValues, file);
    fclose(file);

    //swap?
    if (swapBytes) {
        for (int i = 0; i < fieldPtr->numValues; i++) {
            swap32((char*) (fieldPtr->fieldValues + i), 3);
        }
    }

    //create the coordinate grids
    int cs = headerPtr->gridCS;
    fieldPtr->gridPtr[0] = createGrid(q1Names[cs], headerPtr->q1min,
            headerPtr->q1max, headerPtr->nq1);
    fieldPtr->gridPtr[1] = createGrid(q2Names[cs], headerPtr->q2min,
            headerPtr->q2max, headerPtr->nq2);
    fieldPtr->gridPtr[2] = createGrid(q3Names[cs], headerPtr->q3min,
            headerPtr->q3max, headerPtr->nq3);

    //solenoid files have nq1 = 1 and are symmetric (no phi dependence apart from rotation)
    //torus symmetric if phi max = 30
    fieldPtr->symmetric = false;
    if (headerPtr->nq1 < 2) {
        fieldPtr->symmetric = true;
    } else if ((headerPtr->q1max - headerPtr->q1min) < 31) {
        fieldPtr->symmetric = true;
    }

    //compute some metrics
    computeFieldMetrics(fieldPtr);

    printFieldSummary(fieldPtr, stdout);
    return fieldPtr;
}

//Read the header part of the map file
//on failure return NULL
static FieldMapHeaderPtr readMapHeader(FILE *fd) {

    //create space for the header
    FieldMapHeaderPtr headerPtr = (FieldMapHeaderPtr) malloc(
            sizeof(FieldMapHeader));

    //get the actual file size in bytes
    long actualFileSize = getFileSize(fd);
    debugPrint("Actual file size: %ld bytes\n", actualFileSize);

    //get the magic word and see if byteswap required
    fread(&(headerPtr->magicWord), sizeof(unsigned int), 1, fd);
    swapBytes = (headerPtr->magicWord != MAGICWORD);

    debugPrint("byteswap required: %s\n", swapBytes ? "yes" : "no");

    if (swapBytes) {
        headerPtr->magicWord = htonl(headerPtr->magicWord);
    }

    //if still not a match, it is fatal
    if (headerPtr->magicWord != 0xced) {
        fprintf(stderr,
                "[MAGREADER ERROR] Magic word doesn't match, even after byteswap.\n");

        free(headerPtr);
        return NULL;
    }

    //now that we know if we have to swap, let's read the header all at once
    rewind(fd);
    fread(headerPtr, sizeof(FieldMapHeader), 1, fd);

    if (swapBytes) {
        swap32((char*) headerPtr, sizeof(FieldMapHeader) / 4);
    }

    //if debugging, print header
    debugPrint("\nHEADER:\nmagic word: \"%03x\"\n", headerPtr->magicWord);
    debugPrint("grid CS: %s\n", csLabels[headerPtr->gridCS]);
    debugPrint("field CS: %s\n", csLabels[headerPtr->fieldCS]);

    debugPrint("length units: %s\n", lengthUnitLabels[headerPtr->lengthUnits]);
    debugPrint("angular units: %s\n", angleUnitLabels[headerPtr->angleUnits]);
    debugPrint("field units: %s\n", fieldUnitLabels[headerPtr->fieldUnits]);
    debugPrint("q1Min: %-5.2f\n", headerPtr->q1min);
    debugPrint("q1Max: %-5.2f\n", headerPtr->q1max);
    debugPrint("NumQ2: %d\n", headerPtr->nq1);
    debugPrint("q2Min: %-5.2f\n", headerPtr->q2min);
    debugPrint("q2Max: %-5.2f\n", headerPtr->q2max);
    debugPrint("NumQ2: %d\n", headerPtr->nq2);
    debugPrint("q3Min: %-5.2f\n", headerPtr->q3min);
    debugPrint("q3Max: %-5.2f\n", headerPtr->q3max);
    debugPrint("NumQ3: %d\n", headerPtr->nq3);

    //get the number of field values and the computed file size
    int numFieldValues = headerPtr->nq1 * headerPtr->nq2 * headerPtr->nq3;
    long computedFileSize = sizeof(FieldMapHeader) + 4 * 3 * numFieldValues;

    debugPrint("Computed file size: %ld bytes\n", computedFileSize);
    if (actualFileSize != computedFileSize) {
        fprintf(stderr,
                "[MAGREADER ERROR] Computed file size and actual file size do not match.\n");
        free(headerPtr);
        return NULL;
    }

    return headerPtr;
}

//compute the metrics for this field
static void computeFieldMetrics(MagneticFieldPtr fieldPtr) {

    FieldMetricsPtr metrics = fieldPtr->metricsPtr;
    metrics->maxFieldIndex = -1;
    metrics->maxFieldMagnitude = 0;
    metrics->avgFieldMagnitude = 0;

    for (int i = 0; i < fieldPtr->numValues; i++) {
        FieldValue fieldValue = fieldPtr->fieldValues[i];

        float magnitude = fieldMagnitude(&fieldValue);
        if (magnitude > metrics->maxFieldMagnitude) {
            metrics->maxFieldMagnitude = magnitude;
            metrics->maxFieldIndex = i;
        }

        metrics->avgFieldMagnitude += magnitude;
    }

    metrics->avgFieldMagnitude /= fieldPtr->numValues;
}

//get the time the map was created
static char* getCreationDate(FieldMapHeaderPtr headerPtr) {

    int high = headerPtr->cdHigh;
    int low = headerPtr->cdLow;

    //the divide by 1000 is because Java creation time (which was used) is in nS
    long dlow = low & 0x00000000ffffffffL;
    time_t utime = (((long) high << 32) | (dlow & 0xffffffffL)) / 1000;

    return ctime(&utime);

}

//obtain the size of the file in bytes
// not the file is rewound at exit
static long getFileSize(FILE *fd) {
    long size;
    fseek(fd, 0L, SEEK_END);
    size = ftell(fd);
    rewind(fd);
    return size;
}

//swap a block of 32 bit entities
static void swap32(char *ptr, int num32) {
    int i, j, k;
    char temp[4];

    for (i = 0, j = 0; i < num32; i++, j += 4) {
        for (k = 0; k < 4; k++) {
            temp[k] = ptr[j + k];
        }
        for (k = 0; k < 4; k++) {
            ptr[j + k] = temp[3 - k];
        }
    }
}




