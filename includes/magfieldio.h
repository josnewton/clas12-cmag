//
// Created by David Heddle on 5/31/20.
//


#ifndef CMAG_MAGFIELDIO_H
#define CMAG_MAGFIELDIO_H

#include "magfield.h"


// external function prototypes
extern MagneticFieldPtr initializeTorus(const char *);
extern MagneticFieldPtr initializeSolenoid(const char *);
extern void createCell3D(MagneticFieldPtr);
extern void createCell2D(MagneticFieldPtr);
extern void freeCell3D(Cell3DPtr);
extern void freeCell2D(Cell2DPtr);

#endif //CMAG_MAGFIELDIO_H
