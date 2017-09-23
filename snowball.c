#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "string.h"

#include "loan.h"
#include "loanlist.h"
#include "snowball.h"

#define MODE_SIMPLE 1
#define MODE_REPORT 2
#define MODE_SHUFFLE 3

void help(const char *me) {
  printf(
         "%s v1: Snowball some loans\n\n"
         "Operates on STDIN and writes results to STDOUT.\n"
         "Use shell redirection to your advantage.\n\n"
         "  %s {simple|report|shuffle} [extra_payment]\n\n"
         "  `simple'\n"
         "     Just output the minimum number of months it will take to pay off loans.\n"
         "  `report'\n"
         "     Output the full amortization of the optimal payment plan.\n"
         "  `shuffle'\n"
         "     Randomly try to find a good ordering.\n"
         "  `extra_payment'\n"
         "     An extra payment to apply to principle."
         "\n\n\n"
         "Input is a stream of the following format:\n\n"
         "  ID:string BALANCE:double INTEREST_RATE:double TERM:double\n"
         "  ...\n\n"
         "ID should be fewer than eight characters."
         "INTEREST_RATE should be given as a percentage (e.g., \"4.5\").\n\n"
         "Have fun!\n"
         , me, me);
}

#include "stdarg.h"

int writelog(int mode, const char *format, ...) {
  va_list args;
  int ret = 0;
  va_start(args, format);

  if(mode == MODE_REPORT || mode == MODE_SHUFFLE)
    ret
      =  printf("system: ")
      + vprintf(format, args)
      +  printf("\n");

  va_end(args);
  return ret;
}

/* Entry point. */
int main (int argc, char **argv) {
  clock_t start, stop;
  int mode, code;
  double monthly_payment;
  struct Loan loans[NUMBER_OF_LOANS];

  if (parseargs(argc, argv, &mode, &monthly_payment))
    return 1;

  writelog(mode, "report mode");
  if (monthly_payment < 0) {
    writelog(mode, "extra payment: %.2lf", -monthly_payment);
  } else {
    writelog(mode, "extra payment: none");
  }

  start = clock();
  code = snowball_internal(mode, monthly_payment);
  stop  = clock();

  writelog(mode, "execution time: %lfms", (double)(stop - start) / CLOCKS_PER_SEC * 1000);
  writelog(mode, "bye!");

  return code;
}

int snowball_internal(int mode, double extra_monthly_payment) {
  LoanList loans = loanlist_new();
  money_t tot_min;
  writelog(mode, "reading loans from stdin...");
  loanlist_read(&loans);
  writelog(mode, "reading loans from stdin...done");

  tot_min = loanlist_minimum_payments(&loans);

  writelog(mode, "minimum payment: %.2lf", -tot_min);
  writelog(mode, "total monthly payment: %.2lf", -(tot_min + extra_monthly_payment));

  switch(mode) {
  case MODE_SIMPLE:
    printf("%d\n", loanlist_amort(&loans, extra_monthly_payment, 0));
    break;
  case MODE_REPORT:
    loanlist_print(&loans);
    printf("\n");
    printf("These loans will be paid in %d months.\n",
           loanlist_amort(&loans, extra_monthly_payment, 1));
  case MODE_SHUFFLE:
    loanlist_print(&loans);
    loanlist_order(&loans, 10000, extra_monthly_payment);
  }

  return 0;
}

int parseargs(int argc, char **argv, int *mode, double *monthly_payment) {
  if (argc == 2 && !strcmp(argv[1],"-h")) {
    help(argv[0]);
    return 0;
  }
  if (argc > 3 || argc == 1) {
    help(argv[0]);
    return 1;
  }

  if (!strcmp(argv[1], "simple")) {
    *mode = MODE_SIMPLE;
  } else if (!strcmp(argv[1], "report")) {
    *mode = MODE_REPORT;
  } else if (!strcmp(argv[1], "shuffle")) {
    *mode = MODE_SHUFFLE;
  } else {
    help(argv[0]);
    return 1;
  }

  if (argc == 3) {
    *monthly_payment = -strtod(argv[2], NULL);
  } else {
    *monthly_payment = 0;
  }

  return 0;
}

int error (const char *message) {
  printf("error: %s\n", message);
  return 1;
}

int snowball(LoanList *loans, int mode, money_t extra_payment) {
  struct Loan l;
  int month = 0;
  money_t extra_payment_remaining;
  int verbose = mode == MODE_REPORT;

  /* if (verbose) { */
  /*   printf("# -*- mode: org -*-\n"); */
  /*   printf("* Summary of Loans\n"); */
  /*   loanlist_print(loans); */
  /*   printf("\n* Amortization\n"); */
  /*   printf("| month | snowball"); */
  /*   for (int i = 0; i < NUMBER_OF_LOANS; i++) { */
  /*     printf(" | %s", loans[i].name); */
  /*   } */
  /*   printf(" |\n|       | minimum "); */
  /*   for (int i = 0; i < NUMBER_OF_LOANS; i++) { */
  /*     printf(" | %.2lf", loans[i].minimum_payment); */
  /*   } */
  /*   printf(" |\n|--|\n"); */
  /* } */

  /* while (balance(loans) < 0) { */
  /*   month += 1; */
  /*   extra_payment_remaining = extra_payment + freed_payments(loans); */
  /*   if (verbose) { */
  /*     if (extra_payment_remaining == 0) { */
  /*       printf("| %d | --", month); */
  /*     } else { */
  /*       printf("| %d | %.2lf", month, -extra_payment_remaining); */
  /*     } */
  /*   } */
  /*   for (int i = 0; i < NUMBER_OF_LOANS; i++) { */
  /*     if (loans[i].balance < 0) { */
  /*       pay(&loans[i], &extra_payment_remaining); */
  /*     } */
  /*     if (verbose) { */
  /*       if (loans[i].balance == 0) { */
  /*         printf(" | --"); */
  /*       } else { */
  /*         printf(" | %.2lf", -loans[i].balance); */
  /*       } */
  /*     } */
  /*   } */
  /*   if (verbose) */
  /*     printf(" |\n"); */
  /* } */

  /* if (mode == MODE_SIMPLE) { */
  /*   printf("%d\n", month); */
  /* } else if (verbose) { */
  /*   printf("Loans will be paid in %d months.\n", month); */
  /* } */

  //loanlist_print(loans);

  unsigned int months = loanlist_amort(loans, extra_payment, 1);

  printf("called snowball(): %d\n", months);

  return 0;
}
