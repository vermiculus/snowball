.PHONY: test

test: snowball
	python order.py

snowball: snowball.c loan.c
	gcc -o snowball loan.c snowball.c
