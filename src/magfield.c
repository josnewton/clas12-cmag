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
#include "mapcolor.h"
#include "testdata.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>

//used for unit testing only
MagneticFieldPtr testFieldPtr;

//the field algorithm (global; applies to all fields)
enum Algorithm _algorithm = INTERPOLATION;

//for sector rotations
static double cosSect[] = { NAN, 1, 0.5, -0.5, -1, -0.5, 0.5 };
static double sinSect[] = { NAN, 0, ROOT3OVER2, ROOT3OVER2, 0, -ROOT3OVER2, -ROOT3OVER2 };

//local prototypes
static bool containedInCell3D(Cell3DPtr, double, double, double);
static bool containedInCell2D(Cell2DPtr, double, double);

static void getFieldValueTorus(FieldValuePtr, double, double, double, MagneticFieldPtr);
static void getFieldValueSolenoid(FieldValuePtr, double, double, double, MagneticFieldPtr);

static void rotatePhi(double phi, FieldValuePtr fieldValuePtr);

static void torusCalculate(FieldValuePtr,
                           double,
                           double,
                           double,
                           MagneticFieldPtr);

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
 * Check whether the cell contains the given point. If not, it will
 * have to be reset.
 * @param cell3DPtr the pointer to the 3D cell.
 * @param phi the phi coordinate in degrees.
 * @param rho the rho coordinate in cm.
 * @param z  the z coordinate in cm.
 * @return true if the cell contains the given point.
 */
bool containedInCell3D(Cell3DPtr cell3DPtr, double phi, double rho, double z) {
    return ((phi >= cell3DPtr->phiMin) && (phi < cell3DPtr->phiMax))
    && ((rho >= cell3DPtr->rhoMin) && (rho < cell3DPtr->rhoMax)) &&
    ((z >= cell3DPtr->zMin) && (z < cell3DPtr->zMax));
}

/**
 * Check whether the cell contains the given point. If not, it will
 * have to be reset.
 * @param cell2DPtr the pointer to the 2D cell.
 * @param rho the rho coordinate in cm.
 * @param z  the z coordinate in cm.
 * @return true if the cell contains the given point.
 */
bool containedInCell2D(Cell2DPtr cell2DPtr, double rho, double z) {
    return ((rho >= cell2DPtr->rhoMin) && (rho < cell2DPtr->rhoMax)) &&
           ((z >= cell2DPtr->zMin) && (z < cell2DPtr->zMax));
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

    cell3DPtr->b[0][0][0] = getFieldAtIndex(fieldPtr, i000);
    cell3DPtr->b[0][0][1] = getFieldAtIndex(fieldPtr, i001);
    cell3DPtr->b[0][1][0] = getFieldAtIndex(fieldPtr, i010);
    cell3DPtr->b[0][1][1] = getFieldAtIndex(fieldPtr, i011);
    cell3DPtr->b[1][0][0] = getFieldAtIndex(fieldPtr, i100);
    cell3DPtr->b[1][0][1] = getFieldAtIndex(fieldPtr, i101);
    cell3DPtr->b[1][1][0] = getFieldAtIndex(fieldPtr, i110);
    cell3DPtr->b[1][1][1] = getFieldAtIndex(fieldPtr, i111);

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
    cell2DPtr->b[0][0] = getFieldAtIndex(fieldPtr, i00);
    cell2DPtr->b[0][1] = getFieldAtIndex(fieldPtr, i01);
    cell2DPtr->b[1][0] = getFieldAtIndex(fieldPtr, i10);
    cell2DPtr->b[1][1] = getFieldAtIndex(fieldPtr, i11);

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

    //here is where we apply any shifts
    x -= fieldPtr->shiftX;
    y -= fieldPtr->shiftY;
    z -= fieldPtr->shiftX;

    //see if we are contained
    double rho = hypot(x, y);

    if (!containsCylindrical(fieldPtr, rho, z)) {
        fieldValuePtr->b1 = 0;
        fieldValuePtr->b2 = 0;
        fieldValuePtr->b3 = 0;
    } else {

        //will even need phi for solenoid to rotate
        double phi = toDegrees(atan2(y, x));
        double rho = hypot(x, y);

        if (fieldPtr->type == TORUS) {
            getFieldValueTorus(fieldValuePtr, phi, rho, z, fieldPtr);
        }
        else  { //solenoid
            getFieldValueSolenoid(fieldValuePtr, phi, rho, z, fieldPtr);
        }

        //scale the field
        fieldValuePtr->b1 *= fieldPtr->scale;
        fieldValuePtr->b2 *= fieldPtr->scale;
        fieldValuePtr->b3 *= fieldPtr->scale;
    }
}

/**
 * Convenience function to avoid duplicating code for symmetric and full torus maps.
 * @param fieldValuePtr should be a valid pointer to a Torus field. Upon
 * return it will hold the value of the field in kG, in Cartesian components
 * Bx, By, BZ.
 * @param phi the phi coordinate in degrees.
 * @param rho the rho coordinate in cm.
 * @param z the z coordinate in cm.
 * @param fieldPtr a pointer to a torus field map.
 */
static void torusCalculate(FieldValuePtr fieldValuePtr,
                           double phi,
                           double rho,
                           double z,
                           MagneticFieldPtr fieldPtr) {

    Cell3DPtr cell = fieldPtr->cell3DPtr;

    if (!containedInCell3D(cell, phi, rho, z)) {
        resetCell3D(cell, phi, rho, z);
    }

    //nearest neighbor
    double fractPhi = (phi - cell->phiMin) * cell->phiNorm;
    double fractRho = (rho - cell->rhoMin) * cell->rhoNorm;
    double fractZ = (z - cell->zMin) * cell->zNorm;

    int N1 = (fractPhi < 0.5) ? 0 : 1;
    int N2 = (fractRho < 0.5) ? 0 : 1;
    int N3 = (fractZ < 0.5) ? 0 : 1;

    fieldValuePtr->b1 = cell->b[N1][N2][N3]->b1; // Bx
    fieldValuePtr->b2 = cell->b[N1][N2][N3]->b2; // By
    fieldValuePtr->b3 = cell->b[N1][N2][N3]->b3; // Bz

}

/**
 * Get the TORUS field value by tri-linear interpolation or nearest neighbor,
 * depending on settings.
 * @param fieldValuePtr should be a valid pointer to a Torus field. Upon
 * return it will hold the value of the field in kG, in Cartesian components
 * Bx, By, BZ.
 * @param phi the phi coordinate in degrees.
 * @param rho the rho coordinate in cm.
 * @param z the z coordinate in cm.
 * @param fieldPtr a pointer to a torus field map.
 */
void getFieldValueTorus(FieldValuePtr fieldValuePtr,
                   double phi,
                   double rho,
                   double z,
                   MagneticFieldPtr fieldPtr) {

    if (fieldPtr->symmetric) { //torus with 12-fold symmetry
        // relativePhi (-30, 30) phi relative to middle of sector
        double relPhi = relativePhi(phi);
        bool flip = (relPhi < 0.0);
        torusCalculate(fieldValuePtr, fabs(relPhi), rho, z, fieldPtr);

        //do we need to flip?
        if (flip) {
            //flip x and z components
            fieldValuePtr->b1 = -fieldValuePtr->b1;
            fieldValuePtr->b3 = -fieldValuePtr->b3;
        }

        //do we need to rotate?
        int sector = getSector(phi);

        if (sector > 1) {
            double cos = cosSect[sector];
            double sin = sinSect[sector];
            double bx = fieldValuePtr->b1;
            double by = fieldValuePtr->b2;
            fieldValuePtr->b1 = (float) (bx * cos - by * sin);
            fieldValuePtr->b2 = (float) (bx * sin + by * cos);
        }

    }
    else { // full map
        if (phi < 0) {
            phi += 360;
        }
        torusCalculate(fieldValuePtr, phi, rho, z, fieldPtr);
    }


}

/**
 * Get the SOLENOID field value by tri-linear interpolation or nearest neighbor,
 * depending on settings.
 * @param fieldValuePtr should be a valid pointer to a Solenoid field. Upon
 * return it will hold the value of the field in kG, in Cartesian components
 * Bx, By, BZ.
 * @param phi the phi coordinate in degrees. This is needed for a final rotation
 * of the field after it was calculated in the phi = 0 plane.
 * @param rho the rho coordinate in cm.
 * @param z the z coordinate in cm.
 * @param fieldPtr a pointer to a torus field map.
 */
void getFieldValueSolenoid(FieldValuePtr fieldValuePtr,
                           double phi,
                           double rho,
                           double z,
                           MagneticFieldPtr fieldPtr) {

    Cell2DPtr cell = fieldPtr->cell2DPtr;

    if (!containedInCell2D(cell, rho, z)) {
        resetCell2D(cell, rho, z);
    }

    //nearest neighbor
    double fractRho = (rho - cell->rhoMin) * cell->rhoNorm;
    double fractZ = (z - cell->zMin) * cell->zNorm;

    int N2 = (fractRho < 0.5) ? 0 : 1;
    int N3 = (fractZ < 0.5) ? 0 : 1;

    fieldValuePtr->b1 = 0; // Bphi is 0
    fieldValuePtr->b2 = cell->b[N2][N3]->b2; // Brho
    fieldValuePtr->b3 = cell->b[N2][N3]->b3; // Bz

    //rotate with knowledge that for solenoid Bphi = 0 in map
    double phiRad = toRadians(phi);
    double bRho = fieldValuePtr->b2;

    fieldValuePtr->b1 = bRho*cos(phiRad);
    fieldValuePtr->b2 = bRho*sin(phiRad);
}

/**
 * Rotate the field about z
 * @param phi the azimuthal angle in degrees
 * @param fieldValuePtr holds the field value that will be rotated
 */
static void rotatePhi(double phi, FieldValuePtr fieldValuePtr) {
    double phiRad = toRadians(phi);
    double cp = cos(phiRad);
    double sp = sin(phiRad);
    double bPhi = fieldValuePtr->b1;
    double bRho = fieldValuePtr->b2;

    fieldValuePtr->b1 = bRho*cp - bPhi*sp;
    fieldValuePtr->b2 = bRho*sp + bPhi*cp;
}

/**
 * Obtain the combined value of two fields. The field
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
 * @param field1 the first field.
 * @param field2 the second field.
 */
void getCompositeFieldValue(FieldValuePtr fieldValuePtr,
                            double x,
                            double y,
                            double z,
                            MagneticFieldPtr field1,
                            MagneticFieldPtr field2) {

    fieldValuePtr->b1 = 0;
    fieldValuePtr->b2 = 0;
    fieldValuePtr->b3 = 0;

    FieldValue temp;

    getFieldValue(fieldValuePtr, x, y, z, field1);
    getFieldValue(&temp, x, y, z, field2);
    fieldValuePtr->b1 += temp.b1;
    fieldValuePtr->b2 += temp.b2;
    fieldValuePtr->b3 += temp.b3;
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
    *nRho = getIndex(fieldPtr->rhoGridPtr, rho);
    *nZ = getIndex(fieldPtr->zGridPtr, z);
}


/**
 * A unit test for the test field.
 * @return an error message if the test fails, or NULL if it passes.
 * @return
 */
char *nearestNeighborUnitTest() {

    double resolution = 0.1;   //gauss

    setAlgorithm(NEAREST_NEIGHBOR);
    double *data = (double *)malloc(6 * sizeof(double));
    FieldValuePtr fieldValuePtr = (FieldValuePtr) malloc (sizeof(FieldValue));

    if (testFieldPtr->type == TORUS) {

    }
    else { //solenoid
        for (int i = 0; i < ARRAYSIZE(solenoidNN); i++) {
            data = solenoidNN[i];
            getFieldValue(fieldValuePtr, data[0], data[1], data[2], testFieldPtr);

            //test data in Gauss
            double bx = 1000*fieldValuePtr->b1;
            double by = 1000*fieldValuePtr->b2;
            double bz = 1000*fieldValuePtr->b3;

            if (sign(bx) != sign(data[3])) {
                mu_assert("The X components had different signs", false);
            }
            if (sign(by) != sign(data[4])) {
                mu_assert("The Y components had different signs", false);
            }
            if (sign(bz) != sign(data[5])) {
                mu_assert("The Z components had different signs", false);
            }

            if (fabs(bx - data[3]) > resolution) {
                mu_assert("The X components had different values", false);
            }
            if (fabs(by - data[4]) > resolution) {
                mu_assert("The Y components had different values", false);
            }
            if (fabs(bz - data[5]) > resolution) {
                mu_assert("The Z components had different values", false);
            }

        }
    }

    fprintf(stdout, "\nPASSED nearest neighbor UnitTest\n");
    return NULL;
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

    ColorMapPtr colorMap = defaultColorMap();

    int zmin = -100;
    int zmax = 500;
    int xmin = 0;
    int xmax = 360;

    int del = 2;

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


    fprintf(stdout, "\nStarting svg image creation for: [%s]", path);

    FieldValuePtr fieldValuePtr = (FieldValuePtr) malloc(sizeof (FieldValue));

    int x = xmin+del;
    while (x < xmax+del) {
        if ((x % 50) == 0) {
            fprintf(stdout, ".");
        }
        int xPic = marginTop + imageHeight - x; //x is vertical
        int z = zmin;
        while (z < zmax) {

            int zPic = z - zmin + marginLeft;

            getCompositeFieldValue(fieldValuePtr, x, 0, z, torus, solenoid);
            double magnitude = fieldMagnitude(fieldValuePtr);

            char *color = getColor(colorMap, magnitude);
            svgRectangle(psvg, del, del, zPic, xPic, color, "none", 0, 0, 0);

            z += del;
        }
        x += del;
    }

    free(fieldValuePtr);

    //border
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




