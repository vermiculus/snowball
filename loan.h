#ifndef LOAN_H
#define LOAN_H

#define MAX_ID_LEN 8
#define PERIOD_SUBDIVISION 12
#define NUMBER_OF_LOANS 23

typedef double money_t;

typedef struct Loan Loan;

struct Loan {
  char    id[MAX_ID_LEN];
  money_t balance;
  double  rate;
  double  term;
  money_t minimum_payment;
};

Loan make_loan(char *id, double balance, double rate, double term);

void print_loan_summary(const Loan *loans);

/* Calculates the minimum payment of LOAN */
money_t calc_minimum_payment(const Loan *loan);

/* Pay LOAN with an optional EXTRA_PAYMENT */
void pay(Loan *loan, money_t *extra_payment);

/* Determine the remaining balance of an array of LOANS */
money_t balance(const Loan *loans);

/* Sum the minimum payments of all loans in LOANS whose balance is 0 */
money_t freed_payments(const Loan *loans);

void read_loans(Loan *loans);

money_t total_minimum_payments(const Loan *loans);

#endif
