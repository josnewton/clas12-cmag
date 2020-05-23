//
//  main.c
//  cMag
//
//  Created by David Heddle on 5/22/20.
//  The main program runs the unit tests.
//

#include <stdio.h>
#include <stdlib.h>
#include "magfield.h"
#include "munittest.h"
#include "magfieldutil.h"
#include "munittest.h"

/**
 * Run the unit tests
 * return: NULL or the first failed test message
 */
static char *allTests() {
    mu_run_test(gridUnitTest);
    
    return NULL;
}

/**
 * The main program. The arguments are ignored.
 * argc: number of arguments
 * argv: command line arguments
 * return: 0 on successful completion, 1 on error.
 */
int main(int argc, const char * argv[]) {
    
    //paths to field maps
    char *solenoidPath = (char*) malloc(255);
    char *torusPath = (char*) malloc(255);

    //for testing will look for mag fields in $HOME/magfield
    //here we build the paths
    const char *homedir = getenv("HOME");
    sprintf(solenoidPath, "%s/magfield/Symm_solenoid_r601_phi1_z1201_13June2018.dat", homedir);
    sprintf(torusPath, "%s/magfield/Symm_torus_r2501_phi16_z251_24Apr2018.dat",
            homedir);

    fprintf(stdout, "Testing the magfield reader\n");

    //try to read the torus
    MagneticFieldPtr torus = readField(torusPath, "TORUS");
    if (torus == NULL) {
        fprintf(stderr, "Failed to read torus map from [%s]\n", torusPath);
        return 1;
    }

    //try to read the solenoid
    MagneticFieldPtr solenoid = readField(solenoidPath, "SOLENOID");
    if (solenoid == NULL) {
        fprintf(stderr, "Failed to read solenoid map from [%s]\n",
                solenoidPath);
        return 1;
    }
    
    //run the unit tests
    char *testResult = allTests();

    freeFieldMap(torus);
    freeFieldMap(solenoid);
    
    if (testResult != NULL) {
        fprintf(stdout, "Unit test failed: [%s]\n", testResult);
    }
    else {
        fprintf(stdout, "Program ran successfully.\n");
    }
    return (testResult == NULL) ? 0 : 1;
}




