//
// Created by David Heddle on 6/1/20.
//

#ifndef CMAG_MAGFIELDDRAW_H
#define CMAG_MAGFIELDDRAW_H

#include "magfield.h"

//external function prototypes
extern void createSVGImageFixedPhi(char *path, double, MagneticFieldPtr, MagneticFieldPtr);
extern void createSVGImageFixedZ(char *path, double, MagneticFieldPtr, MagneticFieldPtr);

#endif //CMAG_MAGFIELDDRAW_H
