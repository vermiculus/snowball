snowball: snowball.c util.c loan.c loanlist.c
	gcc -o snowball util.c loan.c loanlist.c snowball.c

.PHONY: test demo

demo: snowball
	bash demo.sh

# calling pattern: snowball {report|simple|shuffle} [extra-payment]
test: snowball
	bash testgen.sh 25 | ./snowball simple 10
	./snowball report 1000 < loans-reversed.dat
	./snowball shuffle 1000 < loans-reversed.dat
