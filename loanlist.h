#ifndef LOANLIST_H
#define LOANLIST_H

#include "stdlib.h"

#include "loan.h"

typedef struct LoanList LoanList;

typedef void (*fnLoanOperation)(Loan *loan, void *return_value);
typedef int  (*fnLoanFilter)   (Loan *loan);

struct LoanList {
  Loan *values;
  size_t count;
  size_t __allocated_space;
};

LoanList loanlist_new();
void loanlist_add  (LoanList *loans, char *name, money_t balance, double rate, double term);
void loanlist_print(LoanList *loans);
unsigned int loanlist_amort(LoanList *loans, money_t extra_payment, int print);
unsigned int loanlist_read (LoanList *dest);
void loanlist_reset(LoanList *loans);

void loanlist_loop (LoanList *loans, void *io, fnLoanOperation, fnLoanFilter);
money_t loanlist_freed_payments(LoanList *loans);
money_t loanlist_minimum_payments(LoanList *loans);
money_t loanlist_balance(LoanList *loans);

#endif
