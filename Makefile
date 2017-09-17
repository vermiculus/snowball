.PHONY: test

test: snowball
	python order.py

snowball:
	gcc -o snowball snowball.c
