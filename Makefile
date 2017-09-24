FILES:=$(wildcard src/*.c)
FILES_BARE:=$(notdir $(FILES))

snowball: $(FILES)
	gcc -Wall -o snowball $(FILES)

.PHONY: test demo coverage clean

demo: snowball
	bash test/demo.sh

# calling pattern: snowball {report|simple|shuffle} [extra-payment]
test: snowball
	./snowball -h >/dev/null
	bash test/testgen.sh 25 | ./snowball simple >/dev/null
	bash test/testgen.sh 50 | ./snowball simple 10 >/dev/null
	bash test/testgen.sh 100 > test/loans.dat
	./snowball report 1000 < test/loans.dat >/dev/null
	./snowball shuffle 1000 < test/loans.dat >/dev/null
	./snowball >/dev/null || true #should fail
	./snowball 1 2 3 >/dev/null || true #should fail
	./snowball fake-mode >/dev/null || true #should fail

coverage: clean
	cd src && gcc -Wall -fprofile-arcs -ftest-coverage -o ../snowball $(FILES_BARE)
	$(MAKE) test -W snowball #don't run the `snowball' target
	cd src && gcov $(FILES_BARE)
	mkdir -p coverage-data && mv src/*.g* coverage-data

clean:
	rm -rf coverage-data
