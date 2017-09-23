#!/usr/bin/env bash
echo "Demoing with random data."
test_loans=$(bash ./testgen.sh 25)
months=$(echo "$test_loans" | ./snowball simple)
echo "'simple' mode: $months\n"
echo "'report' mode executions follow.\n\n"
echo "$test_loans" | ./snowball report 3000
echo "$test_loans" | ./snowball report 2000 | grep -v "^system:"
