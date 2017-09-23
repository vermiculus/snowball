#include "assert.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "time.h"
#include "stdlib.h"

#include "util.h"
#include "loan.h"
#include "loanlist.h"

LoanList loanlist_new() {
  LoanList r;
  r.count = 0;
  r.__allocated_space = 32;
  r.values = malloc(sizeof(Loan) * r.__allocated_space);
  return r;
}

void loanlist_add(LoanList *loans, char *name, money_t balance, double rate, double term) {
  /* realloc will give us a new address if and only if the memory was
     reallocated and freed.  If we get a new address, then we need to
     keep track of that new address. */
  if (loans->count == loans->__allocated_space) {
    loans->__allocated_space *= 2;
    loans->values = realloc(loans->values, sizeof(Loan) * loans->__allocated_space);
  }

  loans->count += 1;

  loan_make(&(loans->values[loans->count - 1]),
            name, balance, rate, term);
}

void loanlist_print_one(Loan *l, void *header_lengths) {
  int *lens = (int*)header_lengths;
  printf("   %*s   %*.2lf   %*.2lf   %*.1lf   %*.2lf\n",
         lens[0], l->name,
         lens[1], -(double)l->balance,
         lens[2], l->rate * PERIOD_SUBDIVISION * 100,
         lens[3], l->term / PERIOD_SUBDIVISION,
         lens[4], -l->minimum_payment);
}

int max(int a, int b) { return a > b ? a : b; }
void loanlist_print__find_col_widths(Loan *l, void *io) {
  int *lens = (int*)io;
  lens[0] = max(lens[0], strlen(l->name));
  lens[1] = max(lens[1], util_digits(-l->balance));
  lens[2] = max(lens[2], util_digits(l->rate * PERIOD_SUBDIVISION * 100));
  lens[3] = max(lens[3], util_digits(l->term / PERIOD_SUBDIVISION));
  lens[4] = max(lens[4], util_digits(-l->minimum_payment));
}
void loanlist_print(LoanList *loans) {
  const char * const headers[5] = {
    "Loan ID",
    "Balance ($)",
    "Rate (%)",
    "Term",
    "Min Pmt ($)"
  };
  int lens[5];
  for (int i = 0; i < 5; i++)
    lens[i] = strlen(headers[i]);
  loanlist_loop(loans, lens, loanlist_print__find_col_widths, NULL);
  for (int i = 0; i < 5; i++)
    printf("   %*s", lens[i], headers[i]);
  printf("\n");
  loanlist_loop(loans, lens, loanlist_print_one, NULL);
}

void loanlist_amort__pay_loan(Loan *l, void *extra_payment_remaining) {
  loan_pay(l, (money_t*)extra_payment_remaining);
}
int  loanlist_amort__loan_has_outstanding_balance(Loan *l) { return l->balance < 0; }
void loanlist_amort__print_one(Loan *l, void *ignored) {
  printf("  ");
  if (l->balance < 0) {
    printf("%*.2lf", l->__colwidth, -l->balance);
  } else {
    printf("%*s--", l->__colwidth - 2, "");
  }
}
void loanlist_amort__print_one_header(Loan *l, void *ignored) {
  printf("  %*s", l->__colwidth, l->name);
}
unsigned int loanlist_amort(LoanList *loans, money_t extra_payment, int print) {
  unsigned int num_months = 0;
  money_t extra_payment_remaining;
  if (print) {
    loanlist_loop(loans, NULL, loanlist_amort__print_one_header, NULL);
    printf("\n");
    loanlist_loop(loans, NULL, loanlist_amort__print_one, NULL);
    printf("\n");
  }
  while (loanlist_balance(loans) < 0) { /* might be able to replace this with a cheaper check to _remaining */
    num_months += 1;
    extra_payment_remaining = extra_payment + loanlist_freed_payments(loans);
    loanlist_loop(loans,
                  &extra_payment_remaining,
                  loanlist_amort__pay_loan,
                  loanlist_amort__loan_has_outstanding_balance);
    if (print) {
      loanlist_loop(loans, NULL, loanlist_amort__print_one, NULL);
      printf("\n");
    }
  }
  return num_months;
}

void loanlist_reset__one(Loan *l, void *ignored) { loan_reset(l); }
void loanlist_reset(LoanList *loans) {
  loanlist_loop(loans, NULL, loanlist_reset__one, NULL);
}

unsigned int loanlist_read (LoanList *dest) {
  unsigned int line = 0;

  char *loan_id_buf = malloc(sizeof(dest->values->name));
  double balance, rate, term;

  while (scanf("%s %lf %lf %lf", loan_id_buf, &balance, &rate, &term) == 4) {
    loanlist_add(dest, loan_id_buf, balance, rate, term);
  }

  free(loan_id_buf);
  return line;
}

void loanlist_loop(LoanList *loans, void *io, fnLoanOperation oper, fnLoanFilter filt) {
  for (int i = 0; i < loans->count; i++)
    if (filt == NULL || (*filt)(&(loans->values[i])))
      (*oper)(&(loans->values[i]), io);
}

void    loanlist_balance__accum(Loan *l, void *sum) { *(money_t*)sum += l->balance; }
money_t loanlist_balance(LoanList *loans) {
  money_t bal = 0;
  loanlist_loop(loans, &bal, loanlist_balance__accum, NULL);
  return bal;
}

int     loanlist_freed_payments__filter(Loan *l) { return l->balance == 0; }
void    loanlist_freed_payments__accum (Loan *l, void *sum) { *(money_t*)sum += l->minimum_payment; }
money_t loanlist_freed_payments(LoanList *loans) {
  money_t bal = 0;
  loanlist_loop(loans, &bal, loanlist_freed_payments__accum, loanlist_freed_payments__filter);
  return bal;
}

int     loanlist_minimum_payments__filter(Loan *l) { return l->balance < 0; }
money_t loanlist_minimum_payments(LoanList *loans) {
  money_t bal = 0;
  loanlist_loop(loans, &bal, loanlist_freed_payments__accum, loanlist_minimum_payments__filter);
  return bal;
}



void loanlist_order__swap(Loan *a, Loan *b) {
  Loan t = *a;
  *a = *b;
  *b = t;
}
void loanlist_order__shuffle(LoanList *loans) {
  int i, r;
  srand(time(NULL));
  for (i = loans->count - 1; i > 0; i--) {
    r = rand() % (i+1);
    loanlist_order__swap(&loans->values[i], &loans->values[r]);
  }
}

void loanlist_order(LoanList *loans, unsigned int trials, money_t extra_payment) {
  int minimum_months = loanlist_amort(loans, extra_payment, 0);
  printf("Starting minumum: %d\n", minimum_months);
  int months;
  for (int i = 0; i < trials; i++) {
    loanlist_reset(loans);
    loanlist_order__shuffle(loans);
    months = loanlist_amort(loans, extra_payment, 0);
    if (months < minimum_months) {
      minimum_months = months;
      printf("New minimum: %d\n", minimum_months);
      loanlist_reset(loans);
      loanlist_amort(loans, extra_payment, 1);
    }
  }
}



/* Have `pay' keep track of how much money was paid.  At the end, this
   can be compared with total principle to determine how much interest
   was paid. */
