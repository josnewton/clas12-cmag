//
//  munittest.h
//  cMag
//  minimal unit testing macro
//
// A MinUnit test case is just a function that returns NULL if the tests pass.
// If the test fails, the function should return a string describing the failing test.
// mu_assert is simply a macro that returns
// string if the expression passed to it is false.
// The mu_runtest macro calls another test case and returns if that test case fails.

#ifndef munittest_h
#define munittest_h

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); mtests_run++; \
                               if (message) return message; } while (0)
extern int mtests_run;

#endif /* munittest_h */
