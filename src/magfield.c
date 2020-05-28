//
//  magfield.c
//  cMag
//
//  Created by David Heddle on 5/22/20.
//

#include "magfield.h"
#include "magfieldutil.h"
#include "munittest.h"
#include "svg.h"

#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

//used for unit testing only
MagneticFieldPtr testFieldPtr;

//the field algorithm (global; applies to all fields)
enum Algorithm _algorithm = INTERPOLATION;

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
 * Set the global option for the algorithm used to extract field values.
 * @param algorithm it can either be
 */
void setAlgorithm(enum Algorithm algorithm) {
    if (algorithm != _algorithm) {
        _algorithm = algorithm;
        fprintf(stdout, "The algorithm for finding field values has been changed to: %s",
                (_algorithm == INTERPOLATION) ? "INTERPOLATION" : "NEAREST_NEIGHBOR");
    }
}
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
 * This checks whether the given point is within the boundary of the field. This is so the methods
 * that retrieve a field value can short-circuit to zero. Note ther is no phi parameter, because
 * all values of phi are "contained."
 * NOTE: this assumes, as is the case at the time of writing, that the CLAS12 fields have grids
 * in cylindrical coordinates and length units of cm.
 * @param fieldPtr
 * @param rho the rho (q2) coordinate in cm.
 * @param z the z (q3) coordinate in cm.
 * @return true if the point is within the boundary of the field.
 */
bool containsCylindrical(MagneticFieldPtr fieldPtr, double rho, double z) {
    if ((z < fieldPtr->zGridPtr->minVal) || (z > fieldPtr->zGridPtr->maxVal)) {
        return false;
    }

    if ((rho < fieldPtr->rhoGridPtr->minVal) || (rho > fieldPtr->rhoGridPtr->maxVal)) {
        return false;
    }

    return true;
}

/**
 * This checks whether the given point is within the boundary of the field. This is so the methods
 * that retrieve a field value can short-circuit to zero.
 * NOTE: this assumes, as is the case at the time of writing, that the CLAS12 fields have grids
 * in cylindrical coordinates and length units of cm.
 * @param fieldPtr
 * @param x the x coordinate in cm.
 * @param y the y coordinate in cm.
 * @param z the z coordinate in cm.
 * @return true if the point is within the boundary of the field.
 */
bool containsCartesian(MagneticFieldPtr fieldPtr, double x, double y, double z) {

    if ((z < fieldPtr->zGridPtr->minVal) || (z > fieldPtr->zGridPtr->maxVal)) {
        return false;
    }

    double rho = hypot(x, y);
    if ((rho < fieldPtr->rhoGridPtr->minVal) || (rho > fieldPtr->rhoGridPtr->maxVal)) {
        return false;
    }

    return true;
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
    //CLAS fields always have cylindrical grids
    //with q1 = phi, q2 = rho and q3 = z
    fieldPtr->phiGridPtr = createGrid("phi", headerPtr->q1min,
                                      headerPtr->q1max, headerPtr->nq1);
    fieldPtr->rhoGridPtr = createGrid("rho", headerPtr->q2min,
                                      headerPtr->q2max, headerPtr->nq2);
    fieldPtr->zGridPtr = createGrid("z", headerPtr->q3min,
                                    headerPtr->q3max, headerPtr->nq3);

    //this is useful to cache for indexing purposes
    fieldPtr->N23 = headerPtr->nq2 * headerPtr->nq3;

    //solenoid files have nq1 (nPhi) = 1 and are symmetric (no phi dependence apart from rotation)
    //torus symmetric if phi max = 30.
    fieldPtr->symmetric = false;

    if (headerPtr->nq1 < 2) {
        fieldPtr->type = SOLENOID;
        fieldPtr->symmetric = true;
        fieldPtr->cell3DPtr = NULL;
        createCell2D(fieldPtr);
    }
    else {
        fieldPtr->type = TORUS;
        if ((headerPtr->q1max - headerPtr->q1min) < 31) {
            fieldPtr->symmetric = true;
        }
        createCell3D(fieldPtr);
        fieldPtr->cell2DPtr = NULL;
    }


    //compute some metrics
    computeFieldMetrics(fieldPtr);

    printFieldSummary(fieldPtr, stdout);
    return fieldPtr;
}

/**
 * Create a 3D cell, which is used by the torus.
 * Note that nothing is
 * returned, the field's 2D cell pointer is made to point at the cell,
 * and the cell is given a reference to the field.
 * @param fieldPtr a pointer to the solenoid field.
 */
void createCell3D(MagneticFieldPtr fieldPtr) {
    Cell3DPtr cell3DPtr = (Cell3DPtr) malloc(sizeof(Cell3D));
    cell3DPtr->phiMin = INFINITY;
    cell3DPtr->phiMax = -INFINITY;
    cell3DPtr->rhoMin = INFINITY;
    cell3DPtr->rhoMax = -INFINITY;
    cell3DPtr->zMin = INFINITY;
    cell3DPtr->zMax = -INFINITY;
    cell3DPtr->fieldPtr = fieldPtr;
    fieldPtr->cell3DPtr = cell3DPtr;
}

/**
 * Create a 2D cell, which is used by the solenoid, since the lack
 * of phi dependence renders the solenoidal field effectively 2D.
 * Note that nothing is
 * returned, the field's 2D cell pointer is made to point at the cell,
 * and the cell is given a reference to the field.
 * @param fieldPtr a pointer to the solenoid field.
 */
void createCell2D(MagneticFieldPtr fieldPtr) {
    Cell2DPtr cell2DPtr = (Cell2DPtr) malloc(sizeof(Cell2D));
    cell2DPtr->rhoMin = INFINITY;
    cell2DPtr->rhoMax = -INFINITY;
    cell2DPtr->zMin = INFINITY;
    cell2DPtr->zMax = -INFINITY;
    cell2DPtr->fieldPtr = fieldPtr;
    fieldPtr->cell2DPtr = cell2DPtr;
}

/**
 * Free the memory associated with a 3D cell.
 * @param cell3DPtr a pointer to the cell.
 */
void freeCell3D(Cell3DPtr cell3DPtr) {
    free(cell3DPtr);
}

/**
 * Free the memory associated with a 2D cell.
 * @param cell2DPtr a pointer to the cell.
 */
void freeCell2D(Cell2DPtr cell2DPtr) {
    free(cell2DPtr);
}

/**
 * Reset the cell based on a new location. If the location is
 * contained by the cell, then we can use some cached values,
 * such as the neighbors. If it isn't, we have to recalculate all.
 * @param cell3DPtr a pointer to the 3D cell.
 * @param phi the azimuthal angle, in degrees
 * @param rho the transverse coordinate, in cm.
 * @param z the z coordinate, in cm.
 */
void resetCell3D(Cell3DPtr cell3DPtr, double phi, double rho, double z) {
    MagneticFieldPtr fieldPtr = cell3DPtr->fieldPtr;
    GridPtr phiGrid = fieldPtr->phiGridPtr;
    GridPtr rhoGrid = fieldPtr->rhoGridPtr;
    GridPtr zGrid = fieldPtr->zGridPtr;

    // get the field indices for the coordinates
    int nPhi, nRho, nZ;
    getCoordinateIndices(fieldPtr, phi, rho, z, &nPhi, &nRho, &nZ);

    cell3DPtr->phiIndex = nPhi;
    cell3DPtr->rhoIndex = nRho;
    cell3DPtr->zIndex = nZ;

    if (nPhi < 0) {
        fprintf(stdout, "WARNING cell3D bad index for phi = %-12.5f\n", phi);
        return;
    }

    if (nRho < 0) {
        fprintf(stdout, "WARNING cell3D bad index for rho = %-12.5f\n", rho);
        return;
    }

    if (nZ < 0) {
        fprintf(stdout, "WARNING cell3D bad index for z = %-12.5f\n", z);
        return;
    }

    // precompute the boundaries and some factors
    cell3DPtr->phiMin = phiGrid->values[nPhi];
    cell3DPtr->phiMax = phiGrid->values[nPhi + 1];
    cell3DPtr->phiNorm = 1. / phiGrid->delta;

    cell3DPtr->rhoMin = rhoGrid->values[nRho];
    cell3DPtr->rhoMax = rhoGrid->values[nRho + 1];
    cell3DPtr->rhoNorm = 1. / rhoGrid->delta;

    cell3DPtr->zMin = zGrid->values[nZ];
    cell3DPtr->zMax = zGrid->values[nZ + 1];
    cell3DPtr->zNorm = 1. / zGrid->delta;

    int i000 = getCompositeIndex(fieldPtr, nPhi, nRho, nZ);
    int i001 = i000 + 1; // nPhi nRho nZ+1

    int i010 = getCompositeIndex(fieldPtr, nPhi, nRho + 1, nZ); // nPhi nRho+1 nZ
    int i011 = i010 + 1; // nPhi nRho+1 nZ+1

    int i100 = getCompositeIndex(fieldPtr, nPhi + 1, nRho, nZ); // nPhi+1 nRho nZ
    int i101 = i100 + 1; // nPhi+1 nRho nZ+1

    int i110 = getCompositeIndex(fieldPtr, nPhi + 1, nRho + 1, nZ); // nPhi+1 nRho+1 nZ
    int i111 = i110 + 1; // nPhi+1 nRho+1 nZ+1

    // field at 8 corners

    FieldValuePtr fp_000 = getFieldAtIndex(fieldPtr, i000);
    FieldValuePtr fp_001 = getFieldAtIndex(fieldPtr, i001);
    FieldValuePtr fp_010 = getFieldAtIndex(fieldPtr, i010);
    FieldValuePtr fp_011 = getFieldAtIndex(fieldPtr, i011);
    FieldValuePtr fp_100 = getFieldAtIndex(fieldPtr, i100);
    FieldValuePtr fp_101 = getFieldAtIndex(fieldPtr, i101);
    FieldValuePtr fp_110 = getFieldAtIndex(fieldPtr, i110);
    FieldValuePtr fp_111 = getFieldAtIndex(fieldPtr, i111);

    cell3DPtr->b[0][0][0].b1 = fp_000->b1;
    cell3DPtr->b[0][0][1].b1 = fp_001->b1;
    cell3DPtr->b[0][1][0].b1 = fp_010->b1;
    cell3DPtr->b[0][1][1].b1 = fp_011->b1;
    cell3DPtr->b[1][0][0].b1 = fp_100->b1;
    cell3DPtr->b[1][0][1].b1 = fp_101->b1;
    cell3DPtr->b[1][1][0].b1 = fp_110->b1;
    cell3DPtr->b[1][1][1].b1 = fp_111->b1;

    cell3DPtr->b[0][0][0].b2 = fp_000->b2;
    cell3DPtr->b[0][0][1].b2 = fp_001->b2;
    cell3DPtr->b[0][1][0].b2 = fp_010->b2;
    cell3DPtr->b[0][1][1].b2 = fp_011->b2;
    cell3DPtr->b[1][0][0].b2 = fp_100->b2;
    cell3DPtr->b[1][0][1].b2 = fp_101->b2;
    cell3DPtr->b[1][1][0].b2 = fp_110->b2;
    cell3DPtr->b[1][1][1].b2 = fp_111->b2;

    cell3DPtr->b[0][0][0].b3 = fp_000->b3;
    cell3DPtr->b[0][0][1].b3 = fp_001->b3;
    cell3DPtr->b[0][1][0].b3 = fp_010->b3;
    cell3DPtr->b[0][1][1].b3 = fp_011->b3;
    cell3DPtr->b[1][0][0].b3 = fp_100->b3;
    cell3DPtr->b[1][0][1].b3 = fp_101->b3;
    cell3DPtr->b[1][1][0].b3 = fp_110->b3;
    cell3DPtr->b[1][1][1].b3 = fp_111->b3;

}

/**
 * Reset the cell based on a new location. If the location is
 * contained by the cell, then we can use some cached values,
 * such as the neighbors. If it isn't, we have to recalculate all.
 * @param cell2DPtr a pointer to the 2D cell.
 * @param rho the transverse coordinate, in cm.
 * @param z the z coordinate, in cm.
 */
void resetCell2D(Cell2DPtr cell2DPtr, double rho, double z) {
    MagneticFieldPtr fieldPtr = cell2DPtr->fieldPtr;

    GridPtr rhoGrid = fieldPtr->rhoGridPtr;
    GridPtr zGrid = fieldPtr->zGridPtr;

    // get the field indices for the coordinates
    int dummy, nRho, nZ;
    getCoordinateIndices(fieldPtr, 0.0, rho, z, &dummy, &nRho, &nZ);

    cell2DPtr->rhoIndex = nRho;
    cell2DPtr->zIndex = nZ;

    if (nRho < 0) {
        fprintf(stdout, "WARNING cell2D bad index for rho = %-12.5f\n", rho);
        return;
    }

    if (nZ < 0) {
        fprintf(stdout, "WARNING cell2D bad index for z = %-12.5f\n", z);
        return;
    }

    // precompute the boundaries and some factors

    cell2DPtr->rhoMin = rhoGrid->values[nRho];
    cell2DPtr->rhoMax = rhoGrid->values[nRho + 1];
    cell2DPtr->rhoNorm = 1. / rhoGrid->delta;

    cell2DPtr->zMin = zGrid->values[nZ];
    cell2DPtr->zMax = zGrid->values[nZ + 1];
    cell2DPtr->zNorm = 1. / zGrid->delta;

    int i00 = getCompositeIndex(fieldPtr, 0, nRho, nZ);
    int i01 = i00 + 1;

    int i10 = getCompositeIndex(fieldPtr, 0, nRho + 1, nZ);
    int i11 = i10 + 1;

    // field at 4 corners
    FieldValuePtr fp_00 = getFieldAtIndex(fieldPtr, i00);
    FieldValuePtr fp_01 = getFieldAtIndex(fieldPtr, i01);
    FieldValuePtr fp_10 = getFieldAtIndex(fieldPtr, i10);
    FieldValuePtr fp_11 = getFieldAtIndex(fieldPtr, i11);

    cell2DPtr->b[0][0].b2 = fp_00->b2;
    cell2DPtr->b[0][1].b2 = fp_01->b2;
    cell2DPtr->b[1][0].b3 = fp_10->b3;
    cell2DPtr->b[1][1].b3 = fp_11->b3;

}

/**
 * Obtain the value of the field by tri-linear interpolation or nearest neighbor,
 * depending on settings.
 * @param fieldValuePtr should be a valid pointer to a FieldValue. Upon
 * return it will hold the value of the field in kG, in Cartesian components
 * Bx, By, BZ, regardless of the field coordinate system of the map.
 * @param x the x coordinate in cm.
 * @param y the y coordinate in cm.
 * @param z the z coordinate in cm.
 * @param fieldPtr a pointer to the field map.
 */
void getFieldValue(FieldValuePtr fieldValuePtr,
                   double x,
                   double y,
                   double z,
                   MagneticFieldPtr fieldPtr) {

    //see if we are contained
    double rho = hypot(x, y);

    if (!containsCylindrical(fieldPtr, rho, z)) {
        fieldValuePtr->b1 = 0;
        fieldValuePtr->b2 = 0;
        fieldValuePtr->b3 = 0;
    } else {

        //scale the field
        fieldValuePtr->b1 *= fieldPtr->scale;
        fieldValuePtr->b2 *= fieldPtr->scale;
        fieldValuePtr->b3 *= fieldPtr->scale;
    }
}

/**
 * Obtain the value of the field, using one or more field maps. The field
 * is obtained by tri-linear interpolation or nearest neighbor, depending on settings.
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
                            double x,
                            double y,
                            double z,
        MagneticFieldPtr fieldPtr, ...) {
    va_list valist;

    fieldValuePtr->b1 = 0;
    fieldValuePtr->b2 = 0;
    fieldValuePtr->b3 = 0;

    FieldValue temp;

    va_start(valist, fieldPtr); //initialize valist for num number of arguments
    while (fieldPtr != NULL) {
        fieldPtr = va_arg(valist, MagneticFieldPtr);
        getFieldValue(&temp, x, y, z, fieldPtr);
        fieldValuePtr->b1 += temp.b1;
        fieldValuePtr->b2 += temp.b2;
        fieldValuePtr->b3 += temp.b3;
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
    return n1 * fieldPtr->N23 + n2 * fieldPtr->zGridPtr->num + n3;
}

/**
 * This inverts the "composite" index of the 1D data array holding
 * the field data into an index for each coordinate. This can
 * be used, for example, to find the grid coordinate values and field components.
 * @param fieldPtr the pointer to the field map
 * @param index the composite index into the 1D data array. Upon return,
 * coordinate indices of -1 indicate error.
 * @param phiIndex will hold the index for the first coordinate.
 * @param rhoIndex will hold the index for the second coordinate.
 * @param zIndex will hold the index for the third coordinate.
 */
void invertCompositeIndex(MagneticFieldPtr fieldPtr, int index,
                          int *phiIndex, int *rhoIndex, int *zIndex) {
    if ((index < 0) || (index >= fieldPtr->numValues)) {
        *phiIndex = -1;
        *rhoIndex = -1;
        *zIndex = -1;
    }
    else {
        int NZ = fieldPtr->zGridPtr->num;
        int n3 = index % NZ;
        index = (index - n3) / NZ;

        int NRho = fieldPtr->rhoGridPtr->num;
        int n2 = index % NRho;
        int n1 = (index - n2) / NRho;
        *phiIndex  = n1;
        *rhoIndex  = n2;
        *zIndex  = n3;
    }
}

/**
 * Get the coordinate indices from coordinates.
 * @param fieldPtr the field ptr.
 * @param phi the value of the phi coordinate.
 * @param rho the value of the rho coordinate.
 * @param z the value of the z coordinate.
 * @param nPhi upon return, the phi index.
 * @param nRho upon return, the rho index.
 * @param nZ upon return, the z index.
 */
void getCoordinateIndices(MagneticFieldPtr fieldPtr, double phi, double rho, double z,
                          int *nPhi, int *nRho, int *nZ) {
    *nPhi = getIndex(fieldPtr->phiGridPtr, phi);
    *nRho = getIndex(fieldPtr->zGridPtr, rho);
    *nZ = getIndex(fieldPtr->zGridPtr, z);
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
 * A unit test for checking the boundary contains check.
 * @return an error message if the test fails, or NULL if it passes.
 */
char *containsUnitTest() {

    int count = 1000000;
    int phiIndex, rhoIndex, zIndex;
    bool result;

    double x, y, z;

    //first inside
    for (int i = 0; i < count; i++) {

        double phi = randomDouble(0, 360);
        double rho = randomDouble(testFieldPtr->rhoGridPtr->minVal, testFieldPtr->rhoGridPtr->maxVal);
        double z = randomDouble(testFieldPtr->zGridPtr->minVal, testFieldPtr->zGridPtr->maxVal);

        cylindricalToCartesian(&x, &y, phi, rho);
        result = containsCartesian(testFieldPtr, x, y, z);

        mu_assert("The (inside) boundary contains test failed.", result);
    }

    //now outside

    //first inside
    for (int i = 0; i < count; i++) {

        double phi = randomDouble(0, 360);
        double rho = randomDouble(testFieldPtr->rhoGridPtr->maxVal, 2 * testFieldPtr->rhoGridPtr->maxVal);
        double z = randomDouble(testFieldPtr->zGridPtr->minVal, testFieldPtr->zGridPtr->maxVal);

        cylindricalToCartesian(&x, &y, phi, rho);
        result = !containsCartesian(testFieldPtr, x, y, z);
        mu_assert("The (outside) boundary contains test failed (A).", result);

        rho = randomDouble(testFieldPtr->rhoGridPtr->minVal, testFieldPtr->rhoGridPtr->maxVal);
        z = randomDouble(-1000, testFieldPtr->zGridPtr->minVal - 0.01);

        result = !containsCartesian(testFieldPtr, x, y, z);
        mu_assert("The (outside) boundary contains test failed (B).", result);

        rho = randomDouble(testFieldPtr->rhoGridPtr->minVal, testFieldPtr->rhoGridPtr->maxVal);
        z = randomDouble(testFieldPtr->zGridPtr->maxVal + 0.01, 2000);

        result = !containsCartesian(testFieldPtr, x, y, z);
        mu_assert("The (outside) boundary contains test failed (B).", result);
    }


    fprintf(stdout, "\nPASSED containsUnitTest\n");
    return NULL;
}

/**
 * A unit test for the composite indexing
 * @return an error message if the test fails, or NULL if it passes.
 */
char *compositeIndexUnitTest() {

    int count = 1000000;
    int phiIndex, rhoIndex, zIndex;
    bool result;

    for (int i = 0; i < count; i++) {
        int compositeIndex = randomInt(0, testFieldPtr->numValues-1);

        //break it apart an put it back together.
        invertCompositeIndex(testFieldPtr, compositeIndex, &phiIndex, &rhoIndex, &zIndex);

        int testIndex = getCompositeIndex(testFieldPtr, phiIndex, rhoIndex, zIndex);
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

/**
 * Create an SVG image of the fields. Not much flexibility here. Will
 * make the canonical Bmag plot sector 1 midplane, so
 * x (vertical): [0, 360], y = 0, z (horizontal) [-100, 500]. So the
 * svg image is 600 x 360. The pixel size is 2x2.
 * @param path the path to the svg file.
 * @param fieldPtr the first (and perhaps the only) of
 * @param ... the continuation of the the list of field pointers.
 */
void createSVGImage(char *path, MagneticFieldPtr torus, MagneticFieldPtr solenoid) {

    int zmin = -100;
    int zmax = 500;
    int xmin = 0;
    int xmax = 360;

    int del = 20;

    int marginLeft =  50;
    int marginRight =  50;
    int marginTop =  50;
    int marginBottom =  50;

    int imageWidth = zmax - zmin;
    int imageHeight = xmax - xmin;
    int width = imageWidth + marginLeft + marginRight;
    int height = imageHeight + marginTop + marginBottom;

    svg* psvg;
    psvg = svgStart(path, width, height);

    svgFill(psvg, "white");

    char *cstr = (char *) malloc(10);

    fprintf(stdout, "\nStarting svg image creation for: [%s]", path);

    int x = xmin;
    while (x < xmax) {
        if ((x % 50) == 0) {
            fprintf(stdout, ".");
        }
        int xPic = x - xmin + marginTop; //x is vertical
        int z = zmin;
        while (z < zmax) {

            int zPic = z - zmin + marginLeft;

            //random color
            int r = randomInt(0, 255);
            int g = randomInt(0, 255);
            int b = randomInt(0, 255);

            colorToHex(cstr, r, 125, 255);
            svgRectangle(psvg, del, del, zPic, xPic, cstr, "none", 0, 0, 0);

            z += del;
        }
        x += del;
    }
    free(cstr);

    svgRectangle(psvg, imageWidth, imageHeight, marginLeft, marginTop, "none", "black", 1, 0, 0);

    char *label = (char *) malloc(40);


    int z = marginLeft - 14;
    x = xmin;
    while (x <= xmax) {
        int xPic = x - xmin + marginTop;

        sprintf(label, "%d", xmax - x);
        svgRotatedText(psvg, z, xPic+8, "times", 12, "black", "none", -90, label);
        svgLine(psvg, "#cccccc", 1, marginLeft, xPic, marginLeft+imageWidth, xPic);
        x += 60;
    }


    z = zmin;
    x = marginTop + imageHeight + 20;
    while (z <= zmax) {

        int zPic = z - zmin + marginLeft;
        sprintf(label, "%d", z);
        svgText(psvg, zPic-12, x, "times", 12, "black", "none", label);
        svgLine(psvg, "#cccccc", 1, zPic, marginTop, zPic, marginTop+imageHeight);

        z += 100;
    }
    free(label);

    svgEnd(psvg);
    fprintf(stdout, "done.\n");
}




