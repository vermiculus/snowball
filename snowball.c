#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "string.h"

#include "snowball.h"

#define BATCH_MODE 1
#define SINGLE_MODE 2

void help(const char *me) {
  printf(
         "%s v1: Snowball some loans\n"
         "First argument should be a path pointing to a file of the following format:\n\n"
         "  ID:integer BALANCE:double INTEREST_RATE:double TERM:double\n"
         "  ...\n\n"
         "Interest rate should be given as a number between 0 and 1.\n\n"
         "Optional second argument can be an output file.  By default, no output is done.\n\n"
         "Either argument can be \"--\" to use stdin or stdout, respectively.\n\n"
         "Have fun!\n"
         , me);
}

/* Entry point. */
int main (int argc, char **argv) {
  clock_t start, stop;
  int mode, code, do_output;
  double monthly_payment;
  struct Loan loans[NUMBER_OF_LOANS];

  if (parseargs(argc, argv, &mode, &monthly_payment))
    return 1;

  do_output = mode == SINGLE_MODE;

  if (do_output) {
    printf("system: monthly payoff rate: %.2lf\n", -monthly_payment);
    start = clock();
  }

  code = snowball_internal(mode, monthly_payment);

  if (do_output)
    stop  = clock();

  if (do_output) {
    printf("system: execution time: %lfms\n", (double)(stop - start) / CLOCKS_PER_SEC * 1000);
    printf("system: bye!\n");
  }
  return code;
}

int snowball_internal(int mode, double monthly_payment) {
  struct Loan loans[NUMBER_OF_LOANS];
  read_loans(loans);
  money_t tot_min = total_minimum_payments(loans);
  if (monthly_payment > tot_min) {
    printf("%.2lf doesn't cover the minimum payments totaling %.2lf\n",
           -monthly_payment, -tot_min);
    return 1;
  }
  snowball(loans, mode, monthly_payment - tot_min);
  return 0;
}

int parseargs(int argc, char **argv, int *mode, double *monthly_payment) {
  if (argc == 2 && !strcmp(argv[1],"-h")) {
    help(argv[0]);
    return 0;
  }
  if (argc != 3) {
    help(argv[0]);
    return 1;
  }

  /* 0          1          2               */
  /* ./snowball -h                         */
  /* ./snowball batchmode  monthly_payment */
  /* ./snowball batchmode  monthly_payment */
  /* ./snowball singlemode monthly_payment */
  /* ./snowball singlemode monthly_payment */

  if (!strcmp(argv[1], "batchmode")) {
    *mode = BATCH_MODE;
  } else if (!strcmp(argv[1], "singlemode")) {
    *mode = SINGLE_MODE;
  } else {
    help(argv[0]);
    return 1;
  }

  *monthly_payment = -strtod(argv[2], NULL);

  return 0;
}

int error (const char *message) {
  printf("error: %s\n", message);
  return 1;
}

int snowball(struct Loan *loans, int mode, money_t extra_payment) {
  struct Loan l;
  int month = 0;
  money_t extra_payment_remaining;
  int verbose = mode == SINGLE_MODE;

  if (verbose) {
    printf("# -*- mode: org -*-\n");
    printf("* Summary of Loans\n");
    print_loan_summary(loans);
    printf("\n* Amortization\n");
    printf("| month | snowball");
    for (int i = 0; i < NUMBER_OF_LOANS; i++) {
      printf(" | %s", loans[i].id);
    }
    printf(" |\n|       | minimum ");
    for (int i = 0; i < NUMBER_OF_LOANS; i++) {
      printf(" | %.2lf", loans[i].minimum_payment);
    }
    printf(" |\n|--|\n");
  }

  while (balance(loans) < 0) {
    month += 1;
    extra_payment_remaining = extra_payment + freed_payments(loans);
    if (verbose) {
      if (extra_payment_remaining == 0) {
        printf("| %d | --", month);
      } else {
        printf("| %d | %.2lf", month, -extra_payment_remaining);
      }
    }
    for (int i = 0; i < NUMBER_OF_LOANS; i++) {
      if (loans[i].balance < 0) {
        pay(&loans[i], &extra_payment_remaining);
      }
      if (verbose) {
        if (loans[i].balance == 0) {
          printf(" | --");
        } else {
          printf(" | %.2lf", -loans[i].balance);
        }
      }
    }
    if (verbose)
      printf(" |\n");
  }

  if (mode == BATCH_MODE) {
    printf("%d\n", month);
  } else if (verbose) {
    printf("Loans will be paid in %d months.\n", month);
  }

  return 0;
}
