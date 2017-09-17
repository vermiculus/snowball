#include "stdio.h"
#include "math.h"
#include "time.h"
#include "string.h"
#include "stdlib.h"

#define ERR_NO_FILE -1
#define PERIOD_SUBDIVISION 12
#define NUMBER_OF_LOANS 23

#define BATCH_MODE 1
#define SINGLE_MODE 2

typedef double money_t;

struct Loan {
  int     id;
  money_t balance;
  double  rate;
  double  term;
  money_t minimum_payment;
};

/* Print out MESSAGE.  Returns `1' */
int error (const char *message);

/* The main logic of the program. */
int snowball(FILE *data_in, FILE *data_out, int mode, money_t extra_payment);

/* Calculates the minimum payment of LOAN */
money_t calc_minimum_payment(const struct Loan *loan);

/* Pay LOAN with an optional EXTRA_PAYMENT */
void pay(struct Loan *loan, money_t *extra_payment);

/* Determine the remaining balance of an array of LOANS */
money_t balance(const struct Loan *loans);

/* Sum the minimum payments of all loans in LOANS whose balance is 0 */
money_t freed_payments(const struct Loan *loans);

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

int parseargs(int argc, char **argv, int *mode, FILE **data_in, FILE **data_out, double *extra_payment);

/* Entry point. */
int main (int argc, char **argv) {
  clock_t start, stop;
  FILE *data_in, *outfile;
  int mode;
  double extra_payment;

  if (parseargs(argc, argv, &mode, &data_in, &outfile, &extra_payment))
    return 1;

  if (mode == SINGLE_MODE) {
    printf("system: using loan data file: %s\n", (data_in == stdin) ? "(standard input)" : argv[2]);
    printf("system: using output file: %s\n",    (outfile == NULL)  ? "(no output)" : (outfile == stdout) ? "(standard output)" : argv[3]);
    start = clock();
  }
  snowball(data_in, outfile, mode, extra_payment);
  if (mode == SINGLE_MODE)
    stop  = clock();

  if (data_in != stdin)
    fclose(data_in);
  if (outfile != NULL && outfile != stdout)
    fclose(outfile);

  if (mode == SINGLE_MODE) {
    printf("system: execution time: %lfms\n", (double)(stop - start) / CLOCKS_PER_SEC * 1000);
    printf("system: bye!\n");
  }
  return 0;
}

int parseargs(int argc, char **argv, int *mode, FILE **data_in, FILE **data_out, double *extra_payment) {
  if (argc == 2 && !strcmp(argv[1],"-h")) {
    help(argv[0]);
    return 0;
  }
  if (argc != 5) {
    help(argv[0]);
    return 1;
  }
  if (!strcmp(argv[2], argv[3])) {
    return error("refusing to clobber in-file with out-file");
  }

  /* 0          1          2       3        4             */
  /* ./snowball -h                                        */
  /* ./snowball batchmode  @stdin  @stdout  extra_payment */
  /* ./snowball batchmode  @stdin  @none    extra_payment */
  /* ./snowball singlemode @stdin  file.out extra_payment */
  /* ./snowball singlemode file.in file.out extra_payment */

  if (!strcmp(argv[1], "batchmode")) {
    *mode = BATCH_MODE;
  } else if (!strcmp(argv[1], "singlemode")) {
    *mode = SINGLE_MODE;
  } else {
    help(argv[0]);
    return 1;
  }

  *extra_payment = -strtod(argv[4], NULL);

  if (!strcmp(argv[2], "@stdin")) {
    *data_in = stdin;
  } else {
    *data_in = fopen(argv[2], "r");
    if (*data_in == NULL) {
      return error("unreadable in-file");
    }
  }

  if (!strcmp(argv[3], "@stdout")) {
    *data_out = stdout;
  } else if (strcmp(argv[3], "@none")) {
    *data_out = fopen(argv[3], "w");
    if (*data_out == NULL) {
      if (*data_in != stdin) {
        fclose(*data_in);
      }
      return error("unwritable out-file");
    }
  }

  return 0;
}

int error (const char *message) {
  printf("error: %s\n", message);
  return 1;
}

money_t calc_minimum_payment(const struct Loan *l) {
  double pvif;
  if (l->rate == 0)
    return l->balance / l->term;
  pvif = pow(l->rate + 1, l->term);
  return l->rate * l->balance * pvif / (pvif - 1);
}

struct Loan make_loan(int id, double balance, double rate, double term) {
  struct Loan l;
  l.id      = id;
  l.balance = -balance;
  l.rate    = rate / PERIOD_SUBDIVISION;
  l.term    = term * PERIOD_SUBDIVISION;
  l.minimum_payment = calc_minimum_payment(&l);
  return l;
}

void read_loans(FILE *file, struct Loan *loans) {
  int line, loan_id;
  double balance, rate, term;
  for (line = 0; line < NUMBER_OF_LOANS; line++) {
    fscanf(file, "%d %lf %lf %lf", &loan_id, &balance, &rate, &term);
    loans[line] = make_loan(loan_id, balance, rate, term);
  }
}

void print_loan_summary(FILE *to, const struct Loan *loans) {
  struct Loan l;
  fprintf(to, "| Loan ID | Balance | Rate | Term | Minimum Payment |\n|--|\n");
  for (int line = 0; line < NUMBER_OF_LOANS; line++) {
    l = loans[line];
    fprintf(to, "| (id:%d) | %.2lf | %.4lf | %.0lf | %.2lf |\n",
           l.id,
           (double)l.balance,
           l.rate * PERIOD_SUBDIVISION,
           l.term / PERIOD_SUBDIVISION,
           l.minimum_payment);
  }
}

int snowball(FILE *file, FILE *out, int mode, money_t extra_payment) {
  struct Loan loans[NUMBER_OF_LOANS];
  struct Loan l;
  int month = 0;
  money_t extra_payment_remaining;
  int verbose = mode == SINGLE_MODE && out != NULL;

  read_loans(file, loans);
  if (verbose) {
    fprintf(out, "# -*- mode: org -*-\n");
    fprintf(out, "* Summary of Loans\n");
    print_loan_summary(out, loans);
    fprintf(out, "\n* Amortization\n");
    fprintf(out, "| month | snowball");
    for (int i = 0; i < NUMBER_OF_LOANS; i++) {
      fprintf(out, " | (id:%d)", loans[i].id);
    }
    fprintf(out, " |\n|       | minimum ");
    for (int i = 0; i < NUMBER_OF_LOANS; i++) {
      fprintf(out, " | %.2lf", loans[i].minimum_payment);
    }
    fprintf(out, " |\n|--|\n");
  }

  while (balance(loans) < 0) {
    month += 1;
    extra_payment_remaining = extra_payment + freed_payments(loans);
    if (verbose)
      fprintf(out, "| %d | %.2lf", month, extra_payment_remaining);
    for (int i = 0; i < NUMBER_OF_LOANS; i++) {
      if (loans[i].balance < 0) {
        pay(&loans[i], &extra_payment_remaining);
      }
      if (verbose)
        fprintf(out, " | %.2lf", loans[i].balance);
    }
    if (verbose)
      fprintf(out, " |\n");
  }

  if (mode == BATCH_MODE) {
    fprintf(out, "%d\n", month);
  } else if (verbose) {
    fprintf(out, "Loans will be paid in %d months.\n", month);
  }

  return 0;
}

void pay(struct Loan *loan, money_t *extra) {
  int total_pmt = loan->minimum_payment;

  if (extra != NULL)
    total_pmt += *extra;

  if (loan->balance >= total_pmt) {
    *extra = total_pmt - loan->balance;
    loan->balance = 0;
  } else {
    loan->balance -= total_pmt;
    loan->balance += loan->balance * loan->rate;
    *extra = 0;
  }
}

money_t balance(const struct Loan *loans) {
  money_t bal = 0;
  for (int i = 0; i < NUMBER_OF_LOANS; i++) {
    bal += loans[i].balance;
  }
  return bal;
}

money_t freed_payments(const struct Loan *loans) {
  money_t bal = 0;
  for (int i = 0; i < NUMBER_OF_LOANS; i++) {
    if (loans[i].balance == 0) {
      bal += loans[i].minimum_payment;
    }
  }
  return bal;
}
