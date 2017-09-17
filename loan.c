#include "math.h"
#include "string.h"
#include "stdio.h"

#include "loan.h"

money_t calc_minimum_payment(const struct Loan *l) {
  double pvif;
  if (l->rate == 0)
    return l->balance / l->term;
  pvif = pow(l->rate + 1, l->term);
  return l->rate * l->balance * pvif / (pvif - 1);
}

struct Loan make_loan(char *id, double balance, double rate, double term) {
  rate /= 100;
  struct Loan l;
  strcpy(l.id, id);
  l.balance = -balance;
  l.rate    = rate / PERIOD_SUBDIVISION;
  l.term    = term * PERIOD_SUBDIVISION;
  l.minimum_payment = calc_minimum_payment(&l);
  return l;
}

void read_loans(struct Loan *loans) {
  int line;
  char loan_id[MAX_ID_LEN];
  double balance, rate, term;
  for (line = 0; line < NUMBER_OF_LOANS; line++) {
    scanf("%s %lf %lf %lf", loan_id, &balance, &rate, &term);
    loan_id[MAX_ID_LEN-1] = '\0';
    loans[line] = make_loan(loan_id, balance, rate, term);
  }
}

void print_loan_summary(const struct Loan *loans) {
  struct Loan l;
  printf("| Loan ID | Balance ($) | Rate (%%) | Term | Minimum Payment ($) |\n|--|\n");
  for (int line = 0; line < NUMBER_OF_LOANS; line++) {
    l = loans[line];
    printf("| %s | %0.2lf | %02.2lf | %0.1lf | %.2lf |\n",
            l.id,
            -(double)l.balance,
            l.rate * PERIOD_SUBDIVISION * 100,
            l.term / PERIOD_SUBDIVISION,
            -l.minimum_payment);
  }
}

void pay(struct Loan *loan, money_t *extra) {
  int total_pmt = loan->minimum_payment;
  money_t ignore = 0;
  if (extra == NULL)
    extra = &ignore;

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

money_t total_minimum_payments(const struct Loan *loans) {
  money_t bal = 0;
  for (int i = 0; i < NUMBER_OF_LOANS; i++) {
    bal += loans[i].minimum_payment;
  }
  return bal;
}
