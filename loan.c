#include "math.h"
#include "string.h"
#include "stdio.h"

#include "util.h"
#include "loan.h"

money_t calc_minimum_payment(const Loan *l) {
  double pvif;
  if (l->rate == 0)
    return l->balance / l->term;
  pvif = pow(l->rate + 1, l->term);
  return l->rate * l->balance * pvif / (pvif - 1);
}

Loan *loan_make(Loan *dest, char *name, double balance, double rate, double term) {
  if (dest == NULL)
    return NULL;
  rate /= 100;
  strcpy(dest->name, name);
  dest->balance = -balance;
  dest->rate    = rate / PERIOD_SUBDIVISION;
  dest->term    = term * PERIOD_SUBDIVISION;
  dest->minimum_payment = calc_minimum_payment(dest);

  dest->__original_balance = dest->balance;
  dest->__colwidth = util_digits(-dest->balance) + 4;

  return dest;
}

void loan_reset(Loan *loan) {
  loan->balance = loan->__original_balance;
}

money_t loan_pay(Loan *loan, money_t *extra) {
  int total_pmt = loan->minimum_payment;

  /* If `extra' is null, we want to give it a default value. */
  money_t ignore = 0;
  if (extra == NULL)
    extra = &ignore;

  total_pmt += *extra;

  if (loan->balance >= total_pmt) {
    *extra = total_pmt - loan->balance;
    total_pmt -= loan->balance;
    loan->balance = 0;
  } else {
    loan->balance -= total_pmt;
    loan->balance += loan->balance * loan->rate;
    *extra = 0;
  }
  return total_pmt;
}

money_t balance(const Loan *loans) {
  money_t bal = 0;
  for (int i = 0; i < NUMBER_OF_LOANS; i++) {
    bal += loans[i].balance;
  }
  return bal;
}

money_t freed_payments(const Loan *loans) {
  money_t bal = 0;
  for (int i = 0; i < NUMBER_OF_LOANS; i++) {
    if (loans[i].balance == 0) {
      bal += loans[i].minimum_payment;
    }
  }
  return bal;
}

money_t total_minimum_payments(const Loan *loans) {
  money_t bal = 0;
  for (int i = 0; i < NUMBER_OF_LOANS; i++) {
    bal += loans[i].minimum_payment;
  }
  return bal;
}
