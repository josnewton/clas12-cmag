//
//  magfieldutil.h
//  cMag
//
//  Created by David Heddle on 5/22/20.
//  Copyright © 2020 David Heddle. All rights reserved.
//

#ifndef magfieldutil_h
#define magfieldutil_h

#include "magfield.h"

//external prototypes
extern void stringCopy(char **, const char *);
extern const char *fieldUnits(MagneticFieldPtr);
extern const char *lengthUnits(MagneticFieldPtr);
extern double fieldMagnitude(FieldValue *);
extern void printFieldSummary(MagneticFieldPtr, FILE *);
extern void printFieldValue(FieldValue *, FILE *);
extern MagneticFieldPtr createFieldMap(void);
extern void freeFieldMap(MagneticFieldPtr);
extern int randomInt(unsigned int, int, int);
extern char *randomUnitTest();

#endif /* magfieldutil_h */
