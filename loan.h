#ifndef LOAN_H
#define LOAN_H

#define MAX_LOAN_NAME_LEN 8
#define PERIOD_SUBDIVISION 12
#define NUMBER_OF_LOANS 23

typedef double money_t;

typedef struct Loan Loan;

struct Loan {
  char    name[MAX_LOAN_NAME_LEN];
  money_t balance;
  double  rate;
  double  term;
  money_t minimum_payment;
  money_t __original_balance;
  unsigned int __colwidth;
};

Loan *loan_make(Loan *dest, char *id, double balance, double rate, double term);

void loan_reset(Loan *loan);

/* Calculates the minimum payment of LOAN */
money_t loan_calc_minimum_payment(const Loan *loan);

/* Pay LOAN with an optional EXTRA_PAYMENT */
money_t loan_pay(Loan *loan, money_t *extra_payment);

#endif
