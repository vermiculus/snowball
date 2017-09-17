#include "stdio.h"
#include "math.h"
#include "time.h"
#include "string.h"

#define ERR_NO_FILE -1
#define PERIOD_SUBDIVISION 12
#define NUMBER_OF_LOANS 23

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

/* Try to open FILEPATH in MODE.  Handle errors. */
/* If FILEPATH is opened successfully, pass to HANDLER. */
/* After HANDLER is done, close the file again. */
int withopenfile(const char *filepath, const char *mode, int (*handler)(FILE*));

/* The main logic of the program. */
int snowball(FILE *data_in, FILE *data_out);

/* Calculates the minimum payment of LOAN */
money_t calc_minimum_payment(struct Loan *loan);

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

/* Entry point. */
int main (int argc, char **argv) {
  clock_t start, stop;
  int handler_return_value;
  FILE *data_in, *outfile;

  if (argc == 2 && !strcmp(argv[1],"-h")) {
    help(argv[0]);
    return 0;
  }

  if (argc < 2) {
    help(argv[0]);
    return error("need loan data file (or use \"--\" for stdin)");
  }

  if (!strcmp(argv[1], "--")) {
    data_in = stdin;
  } else {
    data_in = fopen(argv[1], "r");
    if (data_in == NULL) {
      return error("unreadable file for argv[1]");
    }
  }
  if (argc >= 3) {
    if (!strcmp(argv[2], "--")) {
      outfile = stdout;
    } else {
      outfile = fopen(argv[2], "w");
      if (outfile == NULL) {
        return error("unwritable file for argv[2]");
      }
    }
  }

  printf("system: using loan data file: %s\n", (data_in == stdin) ? "(standard input)" : argv[1]);
  printf("system: using output file: %s\n",    (outfile == NULL)  ? "(no output)" : (outfile == stdout) ? "(standard output)" : argv[2]);

  start = clock();
  snowball(data_in, outfile);
  stop  = clock();

  if (data_in != stdin)
    fclose(data_in);
  if (outfile != NULL && outfile != stdout)
    fclose(outfile);

  printf("system: execution time: %lfms\n", (double)(stop - start) / CLOCKS_PER_SEC * 1000);
  printf("system: bye!\n");
  return 0;
}

int error (const char *message) {
  printf("error: %s\n", message);
  return 1;
}

int withopenfile(const char *filepath, const char *mode, int (*handler)(FILE*)) {
  FILE *file;
  int handler_return_value;

  file = fopen(filepath, mode);

  if (file == NULL) {
    return ERR_NO_FILE;
  }

  handler_return_value = (*handler)(file);

  fclose(file);

  return handler_return_value;
}

money_t calc_minimum_payment(struct Loan *l) {
  double pvif;
  if (l->rate == 0)
    return l->balance / l->term;
  pvif = pow(l->rate + 1, l->term);
  return l->rate * l->balance * pvif / (pvif - 1);
}

struct Loan make_loan(int id, double balance, double rate, double term) {
  struct Loan l;
  l.id      = id;
  l.balance = balance;
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
  for (int line = 0; line < NUMBER_OF_LOANS; line++) {
    l = loans[line];
    fprintf(to, "(id:%d) %.2lf %.4lf %.0lf %.2lf\n",
           l.id,
           (double)l.balance,
           l.rate * PERIOD_SUBDIVISION,
           l.term / PERIOD_SUBDIVISION,
           l.minimum_payment);
  }
}

int snowball(FILE *file, FILE *out) {
  struct Loan loans[NUMBER_OF_LOANS];
  struct Loan l;
  int month = 0;
  money_t extra_payment;

  read_loans(file, loans);
  if (out != NULL) {
    fprintf(out, "Summary of Loans\n");
    print_loan_summary(out, loans);
  }

  while (balance(loans) < 0) {
    month += 1;
    extra_payment = -50;
    extra_payment += freed_payments(loans);
    if (out != NULL)
      fprintf(out, "(extra:%.2lf) ", extra_payment);
    for (int i = 0; i < NUMBER_OF_LOANS; i++) {
      pay(&loans[i], &extra_payment);
      if (out != NULL)
        fprintf(out, "%.2lf ", loans[i].balance);
    }
    if (out != NULL)
      fprintf(out, "\n");
  }

  printf("snowball: %d months\n", month);

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
