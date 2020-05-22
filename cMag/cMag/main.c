//
//  main.c
//  cMag
//
//  Created by David Heddle on 5/22/20.
//

#include <stdio.h>
#include <stdlib.h>
#include "magfield.h"

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
    MagneticFieldPtr torus = readTorus(torusPath);
    if (torus == NULL) {
        fprintf(stderr, "Failed to read torus map from [%s]\n", torusPath);
        return -1;
    }

    //try to read the solenoid
    MagneticFieldPtr solenoid = readSolenoid(solenoidPath);
    if (solenoid == NULL) {
        fprintf(stderr, "Failed to read solenoid map from [%s]\n",
                solenoidPath);
        return -1;
    }

    freeFieldMap(torus);
    freeFieldMap(solenoid);
    fprintf(stdout, "Program ran successfully.");
    return 0;
}

