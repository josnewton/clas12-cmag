//
//  magfield.c
//  cMag
//
//  Created by David Heddle on 5/22/20.
//

#include "magfield.h"
#include "magfieldutil.h"
#include "munittest.h"
#include "math.h"

#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

//coordinate names
static const char *q1Names[] = { "phi", "x" };
static const char *q2Names[] = { "rho", "y" };
static const char *q3Names[] = { "z", "z" };

//used for unit testing only
MagneticFieldPtr testFieldPtr;

//local prototypes
static FieldMapHeaderPtr readMapHeader(FILE *);
static MagneticFieldPtr readField(const char *);
static long getFileSize(FILE*);
static void swap32(char*, int);
static char* getCreationDate(MagneticFieldPtr);
static void computeFieldMetrics(MagneticFieldPtr);

//do we have to swap bytes?
//since the fields were produced by Java which uses
//new format (BigEndian) we probably will have to swap.
static bool swapBytes = false;

//this is used by the minimal unit testing
int mtests_run = 0;

/**
 * Initialize the torus field.
 * @param torusPath a path to a torus field map. If you want to use environment variables, pass NULL
 * in this parameter, in which case the code make two attempts two attempts at finding the field. The first
 * will be to try the a path specified by the COAT_MAGFIELD_TORUSMAP environment variable. If that
 * fails, it will then check TORUSMAP.
 * @return a valid field pointer on success, NULL on failure.
 */
MagneticFieldPtr initializeTorus(const char *torusPath) {
    if (torusPath == NULL) {
        torusPath = getenv("COAT_MAGFIELD_TORUSMAP");
        if (torusPath == NULL) {
            torusPath = getenv("TORUSMAP");
        }
    }

    if (torusPath == NULL) {
        fprintf(stderr, "\ncMag ERROR null torus path even after trying environment variables.\n");
        return NULL;
    }
    return readField(torusPath);
}

/**
 * Initialize the solenoid field.
 * @param solenoidPath a path to a solenoid field map. If you want to use environment variables, pass NULL
 * in this parameter, in which case the code make two attempts two attempts at finding the field. The first
 * will be to try the a path specified by the COAT_MAGFIELD_SOLENOIDMAP environment variable. If that
 * fails, it will then check SOLENOIDMAP.
 * @return a valid field pointer on success, NULL on failure.
 */
MagneticFieldPtr initializeSolenoid(const char *solenoidPath) {
    if (solenoidPath == NULL) {
        solenoidPath = getenv("COAT_MAGFIELD_SOLENOIDMAP");
        if (solenoidPath == NULL) {
            solenoidPath = getenv("SOLENOIDMAP");
        }
    }

    if (solenoidPath == NULL) {
        fprintf(stderr, "\ncMag ERROR null solenoid path even after trying environment variables.\n");
        return NULL;
    }
    return readField(solenoidPath);
}

/**
 * Read a binary field map at the given location.
 * @param path the full path to a field map file.
 * @return a valid field pointer on success, NULL on failure.
 */
static MagneticFieldPtr readField(const char *path) {

    debugPrint("\nAttempting to read field map from [%s]\n", path);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "\ncMag ERROR could not read field map file: [%s]\n", path);
        return NULL;
    }

    //get the header
    FieldMapHeaderPtr headerPtr = readMapHeader(file);
    if (headerPtr == NULL) {
        fclose(file);
        fprintf(stderr, "\ncMag ERROR could not read field map header from: [%s]\n", path);
        return NULL;
    }

    MagneticFieldPtr fieldPtr = createFieldMap();

    //copy the path and name
    stringCopy(&(fieldPtr->path), path);

    fieldPtr->headerPtr = headerPtr;
    fieldPtr->numValues = headerPtr->nq1 * headerPtr->nq2 * headerPtr->nq3;
    fieldPtr->creationDate = getCreationDate(fieldPtr);

    //malloc the data array
    fieldPtr->fieldValues = malloc(fieldPtr->numValues * sizeof(FieldValue));

    //did we have enough memory?
    if (fieldPtr->fieldValues == NULL) {
        fprintf(stderr, "\ncMag ERROR out of memory when allocating space for field map.\n");
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
    fieldPtr->q1GridPtr = createGrid(q1Names[cs], headerPtr->q1min,
            headerPtr->q1max, headerPtr->nq1);
    fieldPtr->q2GridPtr = createGrid(q2Names[cs], headerPtr->q2min,
            headerPtr->q2max, headerPtr->nq2);
    fieldPtr->q3GridPtr = createGrid(q3Names[cs], headerPtr->q3min,
            headerPtr->q3max, headerPtr->nq3);

    //this is useful to cache for indexing purposes
    fieldPtr->N23 = headerPtr->nq2 * headerPtr->nq3;

    //solenoid files have nq1 = 1 and are symmetric (no phi dependence apart from rotation)
    //torus symmetric if phi max = 30.
    fieldPtr->symmetric = false;

    if (headerPtr->nq1 < 2) {
        fieldPtr->type = SOLENOID;
        fieldPtr->symmetric = true;
    }
    else {
        fieldPtr->type = TORUS;
        if ((headerPtr->q1max - headerPtr->q1min) < 31) {
            fieldPtr->symmetric = true;
        }
    }


    //compute some metrics
    computeFieldMetrics(fieldPtr);

    printFieldSummary(fieldPtr, stdout);
    return fieldPtr;
}

/**
 *
 * @param fieldValuePtr should be a valid pointer to a FieldValue. Upon
 * return it will hold the value of the field in kG, in Cartesian components
 * Bx, By, BZ, regardless of the field coordinate system of the map.
 * @param x the x coordinate in cm.
 * @param y the y coordinate in cm.
 * @param z the z coordinate in cm.
 * @param fieldPtr a pointer to the field map.
 */
void getFieldValue(FieldValuePtr fieldValuePtr,
                   float x,
                   float y,
                   float z,
                   MagneticFieldPtr fieldPtr) {

}

/**
 * Obtain the value of the field, using one or more field maps.
 * @param fieldValuePtr should be a valid pointer to a FieldValue. Upon
 * return it will hold the value of the field, in kG, in Cartesian components
 * Bx, By, BZ, regardless of the field coordinate system of the maps,
 * obtained from all the field maps that it is given in the variable length
 * argument list. For example, if torus and solenoid point to fields,
 * then one can obtain the combined field at (x, y, z) by calling
 * getCompositeFieldValue(fieldVal, x, y, x, torus, solenoid).
 * @param x the x coordinate in cm.
 * @param y the y coordinate in cm.
 * @param z the z coordinate in cm.
 * @param fieldPtr the first (and perhaps the only) of
 * @param ... the continuation of the the list of field pointers.
 */
void getCompositeFieldValue(FieldValuePtr fieldValuePtr,
        float x,
        float y,
        float z,
        MagneticFieldPtr fieldPtr, ...) {
    va_list valist;

    va_start(valist, fieldPtr); //initialize valist for num number of arguments
    while (fieldPtr != NULL) {
        fprintf(stdout, "USING %s\n", (fieldPtr->type == TORUS) ? "TORUS" : "SOLENOID");
        fieldPtr = va_arg(valist, MagneticFieldPtr);
    }
}

/**
 * Read the 80 byte field map header.
 * @param fd the file descriptor.
 * @return a valid pointer to a field map header, or NULL upon failure.
 */
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
                "\ncMag ERROR Magic word doesn't match, even after a byte swap.\n");
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
                "\ncMag ERROR computed file size and actual file size do not match.\n");
        free(headerPtr);
        return NULL;
    }

    return headerPtr;
}

/**
 * Compute some diagnostic metrics for this field.
 * @param fieldPtr the field map pointer.
 */
static void computeFieldMetrics(MagneticFieldPtr fieldPtr) {

    FieldMetricsPtr metrics = fieldPtr->metricsPtr;
    metrics->maxFieldIndex = 0;
    metrics->maxFieldMagnitude = 0;
    metrics->avgFieldMagnitude = 0;

    for (unsigned int i = 0; i < fieldPtr->numValues; i++) {
        FieldValuePtr fieldValuePtr = fieldPtr->fieldValues+i;

        double magnitude = fieldMagnitude(fieldValuePtr);
        if (magnitude > metrics->maxFieldMagnitude) {
            metrics->maxFieldMagnitude = magnitude;
            metrics->maxFieldIndex = i;
        }

        metrics->avgFieldMagnitude += magnitude;
    }

    metrics->avgFieldMagnitude /= fieldPtr->numValues;
}

/**
 * Get the composite index into the 1D data array holding
 * the field data from the coordinate indices.
 * @param fieldPtr the pointer to the field map
 * @param n1 the index for the first coordinate.
 * @param n2 the index for the second coordinate.
 * @param n3 the index for the third coordinate.
 * @return the composite index into the 1D data array.
 */
int getCompositeIndex(MagneticFieldPtr fieldPtr, int n1, int n2, int n3) {
    return n1 * fieldPtr->N23 + n2 * fieldPtr->q3GridPtr->num + n3;
}

/**
 * This converts the "composite" index of the 1D data array holding
 * the field data into an index for each coordinate. This can
 * be used, for example, to find the grid coordinate values and field components.
 * @param fieldPtr the pointer to the field map
 * @param index the composite index into the 1D data array. Upon return,
 * coordinate indices of -1 indicate error.
 * @param q1Index will hold the index for the first coordinate.
 * @param q2Index will hold the index for the second coordinate.
 * @param q3Index will hold the index for the third coordinate.
 */
void getCoordinateIndices(MagneticFieldPtr fieldPtr, int index, int *q1Index, int *q2Index, int *q3Index) {
    if ((index < 0) || (index >= fieldPtr->numValues)) {
        *q1Index = -1;
        *q2Index = -1;
        *q3Index = -1;
    }
    else {
        int N3 = fieldPtr->q3GridPtr->num;
        int n3 = index % N3;
        index = (index - n3) / N3;

        int N2 = fieldPtr->q2GridPtr->num;
        int n2 = index % N2;
        int n1 = (index - n2) / N2;
        *q1Index  = n1;
        *q2Index  = n2;
        *q3Index  = n3;
    }
}

 /**
  * Get the creation date of a field map.
  * @param fieldPtr a pointer to the field map.
  * @return A string representation of the date and the the field map
  * was created from the engineering data.
  */
static char *getCreationDate(MagneticFieldPtr fieldPtr) {

    int high = fieldPtr->headerPtr->cdHigh;
    int low = fieldPtr->headerPtr->cdLow;

    //the divide by 1000 is because Java creation time (which was used) is in nS
    long dlow = low & 0x00000000ffffffffL;
    time_t utime = (((long) high << 32) | (dlow & 0xffffffffL)) / 1000;

    return ctime(&utime);

}

/**
 * Get the file size in bytes.
 * @param fd the file descriptor.
 * @return the file size in bytes.
 */
static long getFileSize(FILE *fd) {
    long size;
    fseek(fd, 0L, SEEK_END);
    size = ftell(fd);
    rewind(fd);
    return size;
}

/**
 * Byte swap a block of 32-bit entities
 * @param ptr a pointer to the block.
 * @param num32 the number of entities.
 */
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

/**
 * A unit test for the composite indexing
 * @return an error message if the test fails, or NULL if it passes.
 */
char *compositeIndexUnitTest() {

    int count = 10000;
    int q1Index, q2Index, q3Index;
    bool result;

    for (int i = 0; i < count; i++) {
        int compositeIndex = randomInt(0, 0, testFieldPtr->numValues-1);

        //break it apart an put it back together.
        getCoordinateIndices(testFieldPtr, compositeIndex, &q1Index, &q2Index, &q3Index);

        int testIndex = getCompositeIndex(testFieldPtr, q1Index, q2Index, q3Index);
        result = (testIndex == compositeIndex);

        mu_assert("Reconstructed index did not match composite index.", result);
    }

    fprintf(stdout, "\nPASSED compositeIndexUnitTest\n");
    return NULL;
}

/**
 * Get the field at a given composite index.
 * @param fieldPtr a pointer to the field.
 * @param compositeIndex the composite index.
 * @return a pointer to the field value, or NULL if out of range.
 */
FieldValuePtr getFieldAtIndex(MagneticFieldPtr fieldPtr, int compositeIndex) {
    if ((compositeIndex < 0) || (compositeIndex > fieldPtr->numValues)) {
        return NULL;
    }
    return fieldPtr->fieldValues + compositeIndex;
}




