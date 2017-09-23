snowball: snowball.c loan.c loanlist.c
	gcc -o snowball loan.c loanlist.c snowball.c

.PHONY: test demo

demo: snowball
	./snowball simple 4650 < loans.dat
	./snowball report 3000 < loans.dat
	./snowball report 2000 < loans.dat

# calling pattern: snowball {report|simple} {extra-payment}
test: snowball
	./snowball simple 10 < test-loans.dat
	./snowball report 1000 < loans.dat
