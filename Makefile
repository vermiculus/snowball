FILES:=snowball.c util.c loan.c loanlist.c
snowball: $(FILES)
	gcc -Wall -o snowball $(FILES)

.PHONY: test demo coverage clean

demo: snowball
	bash demo.sh

# calling pattern: snowball {report|simple|shuffle} [extra-payment]
test: snowball
	./snowball -h >/dev/null
	bash testgen.sh 25 | ./snowball simple >/dev/null
	bash testgen.sh 50 | ./snowball simple 10 >/dev/null
	./snowball report 1000 < loans-reversed.dat >/dev/null
	./snowball shuffle 1000 < loans-reversed.dat >/dev/null
	./snowball >/dev/null || true #should fail
	./snowball 1 2 3 >/dev/null || true #should fail
	./snowball fake-mode >/dev/null || true #should fail

coverage: $(FILES) clean
	gcc -Wall -fprofile-arcs -ftest-coverage -o snowball $(FILES)
	$(MAKE) test
	echo "Done test"
	gcov $(FILES)
	rm -f *.gcda *.gcno

clean:
	rm -f *.gcov
