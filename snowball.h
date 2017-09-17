#ifndef SNOWBALL_H
#define SNOWBALL_H

#include "loan.h"

/* Print out MESSAGE.  Returns `1' */
int error (const char *message);

/* The main logic of the program. */
int snowball(Loan *loans, int mode, money_t monthly_payment);

int snowball_internal(int mode, double monthly_payment);

int parseargs(int argc, char **argv, int *mode, double *monthly_payment);

#endif
