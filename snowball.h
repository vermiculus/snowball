#ifndef SNOWBALL_H
#define SNOWBALL_H

#define MAX_ID_LEN 8

typedef double money_t;

struct Loan {
  char    id[MAX_ID_LEN];
  money_t balance;
  double  rate;
  double  term;
  money_t minimum_payment;
};

void help(const char *me);

struct Loan make_loan(char *id, double balance, double rate, double term);

void print_loan_summary(const struct Loan *loans);

/* Print out MESSAGE.  Returns `1' */
int error (const char *message);

/* The main logic of the program. */
int snowball(struct Loan *loans, int mode, money_t monthly_payment);

int snowball_internal(int mode, double monthly_payment);

/* Calculates the minimum payment of LOAN */
money_t calc_minimum_payment(const struct Loan *loan);

/* Pay LOAN with an optional EXTRA_PAYMENT */
void pay(struct Loan *loan, money_t *extra_payment);

/* Determine the remaining balance of an array of LOANS */
money_t balance(const struct Loan *loans);

/* Sum the minimum payments of all loans in LOANS whose balance is 0 */
money_t freed_payments(const struct Loan *loans);

void read_loans(struct Loan *loans);

money_t total_minimum_payments(const struct Loan *loans);

int parseargs(int argc, char **argv, int *mode, double *monthly_payment);



#endif
