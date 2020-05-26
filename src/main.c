//
//  main.c
//  cMag
//
//  Created by David Heddle on 5/22/20.
//  The main program runs the unit tests.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "magfield.h"
#include "munittest.h"
#include "magfieldutil.h"

//the two fields we'll try to initialize
static MagneticFieldPtr torus;
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

    fprintf(stdout, "\n  [TORUS]");
    testFieldPtr = torus;
    mu_run_test(compositeIndexUnitTest);
    mu_run_test(containsUnitTest);

    testFieldPtr = solenoid;
    fprintf(stdout, "\n  [SOLENOID]");
    mu_run_test(compositeIndexUnitTest);
    mu_run_test(containsUnitTest);

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
    char *torusPath = (char*) malloc(255);


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

    sprintf(solenoidPath, "%s/Symm_solenoid_r601_phi1_z1201_13June2018.dat", dataDir);
    sprintf(torusPath, "%s/Symm_torus_r2501_phi16_z251_24Apr2018.dat", dataDir);

    fprintf(stdout, "\nTesting the cMag library\n");

    //try to read the torus
    torus = initializeTorus(torusPath);
    if (torus == NULL) {
        fprintf(stderr, "\ncMag ERROR failed to read torus map from [%s]\n", torusPath);
        return 1;
    }

    //try to read the solenoid
    solenoid = initializeSolenoid(solenoidPath);
    if (solenoid == NULL) {
        fprintf(stderr, "\ncMag ERROR failed to read solenoid map from [%s]\n",
                solenoidPath);
        return 1;
    }
    
    //run the unit tests
    testFieldPtr = torus; //used for unit test
    char *testResult = allTests();

    freeFieldMap(torus);
    freeFieldMap(solenoid);

  //  setAlgorithm(NEAREST_NEIGHBOR);

  //  FieldValuePtr fieldValuePtr = (FieldValuePtr) malloc(sizeof (FieldValue));
  //  getCompositeFieldValue(fieldValuePtr, 200, 200, 300, torus, solenoid);
  //  free (fieldValuePtr);

    if (testResult != NULL) {
        fprintf(stdout, "Unit test failed: [%s]\n", testResult);
    }
    else {
        fprintf(stdout, "\nProgram ran successfully.\n");
    }
    return (testResult == NULL) ? 0 : 1;
}




