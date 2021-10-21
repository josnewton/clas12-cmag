
//  cMag
//
//  Created by David Heddle on 5/22/20.
//  The main program runs the unit tests.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "magfield.h"
#include "magfieldio.h"
#include "munittest.h"
#include "magfieldutil.h"
#include "magfielddraw.h"
#include "svg.h"
#include "mapcolor.h"

//the three fields we'll try to initialize
static MagneticFieldPtr symmetricTorus;
static MagneticFieldPtr fullTorus;
static MagneticFieldPtr solenoid;

/**
 * Run the unit tests
 * return: NULL or the first failed test message
 */
static char *allTests() {
    fprintf(stdout, "\n\n***** Unit tests ****** \n");
    mu_run_test(gridUnitTest);
    mu_run_test(randomUnitTest);
    mu_run_test(conversionUnitTest);
    mu_run_test(binarySearchUnitTest);

    fprintf(stdout, "\n  [SYMMETRIC TORUS]");
    testFieldPtr = symmetricTorus;
    mu_run_test(compositeIndexUnitTest);
    mu_run_test(containsUnitTest);
    mu_run_test(nearestNeighborUnitTest);

    fprintf(stdout, "\n  [FULL  TORUS]");
    testFieldPtr = fullTorus;
    mu_run_test(compositeIndexUnitTest);
    mu_run_test(containsUnitTest);
    mu_run_test(nearestNeighborUnitTest);

    testFieldPtr = solenoid;
    fprintf(stdout, "\n  [SOLENOID]");
    mu_run_test(compositeIndexUnitTest);
    mu_run_test(containsUnitTest);
    mu_run_test(nearestNeighborUnitTest);

    fprintf(stdout, "\n ***** End of unit tests ******\n");
    return NULL;
}

/**
 * The main method of the test application.
 * @param argc the number of arguments
 * @param argv the command line argument. Only one is processed, the path
 * to the directory containing the magnetic fields. If that argument is missing,
 * it will look in $(HOME)/magfield.
 * @return 0 on successful completion, 1 if any error occurred.
 */
int main(int argc, const char * argv[]) {
    
    //paths to field maps
    char *solenoidPath = (char*) malloc(255);
    char *torusSymmetricPath = (char*) malloc(255);
    char *torusFullPath = (char*) malloc(255);


    //for testing will look for mag fields in either the first
    // command line argument, if provided, or in $HOME/magfield.
    //Here we build the paths
    const char *dataDir;

    if (argc > 1) {
        dataDir = argv[1];
        fprintf(stdout, "Using command line data directory: [%s]", dataDir);
    }
    else {
        dataDir = strcat(getenv("HOME"), "/magfield");
    }

    //build the full paths
    sprintf(solenoidPath, "%s/Symm_solenoid_r601_phi1_z1201_13June2018.dat", dataDir);
    sprintf(torusSymmetricPath, "%s/Symm_torus_r2501_phi16_z251_24Apr2018.dat", dataDir);
    sprintf(torusFullPath, "%s/Full_torus_r251_phi181_z251_03March2020.dat", dataDir);

    fprintf(stdout, "\nTesting the cMag library\n");

    //try to read the symmetric torus
    symmetricTorus = initializeTorus(torusSymmetricPath);
    if (symmetricTorus == NULL) {
        fprintf(stderr, "\ncMag ERROR failed to read symmetric torus map from [%s]\n", torusSymmetricPath);
        return 1;
    }

    //try to read the full torus
    fullTorus = initializeTorus(torusFullPath);
    if (fullTorus == NULL) {
        fprintf(stderr, "\ncMag ERROR failed to read full torus map from [%s]\n", torusFullPath);
        return 1;
    }

    //try to read the solenoid
    solenoid = initializeSolenoid(solenoidPath);
    if (solenoid == NULL) {
        fprintf(stderr, "\ncMag ERROR failed to read solenoid map from [%s]\n",
                solenoidPath);
        return 1;
    }




    FieldValuePtr torusValuePtr = (FieldValuePtr) malloc(sizeof (FieldValue));
    FieldValuePtr solenoidValuePtr = (FieldValuePtr) malloc(sizeof (FieldValue));
    FieldValuePtr combinedValuePtr = (FieldValuePtr) malloc(sizeof (FieldValue));    

    double rho = 60.0;
    double phi = 0.0;
    double z = 400.0;

    double phiRad = toRadians(phi);
    double cosPhi = cos(phiRad);
    double sinPhi = sin(phiRad);

    double rhoRad = toRadians(rho);
    double cosRho = cos(phiRad);
    double sinRho = sin(phiRad);

    //getFieldValueTorus(torusValuePtr, phiRad, rhoRad, z, fullTorus);
    //getFieldValueSolenoid(solenoidValuePtr, phiRad, rhoRad, z, solenoid);

    getCompositeFieldValue(combinedValuePtr, rho*cosPhi, rho*sinPhi, z, fullTorus, solenoid);
    double magnitude = fieldMagnitude(combinedValuePtr);
    
    //double tbx = torusValuePtr->b1;
    //double tby = torusValuePtr->b2;
    //double tbz = torusValuePtr->b3;

    //double sbx = solenoidValuePtr->b1;
    //double sby = solenoidValuePtr->b2;
    //double sbz = solenoidValuePtr->b3;


    printf("\n", 0.0);
    printf("\n", 0.0);
    printf("Total Field Magnitude is %lf \n", magnitude);

    return 0;
    //return (testResult == NULL) ? 0 : 1;
}








