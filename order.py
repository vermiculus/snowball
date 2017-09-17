# This script shuffles your data file many times to see if it can find
# the optimal sort.

LOAN_DATA_FILE = "test-loans.dat"

from subprocess import check_output
from random import shuffle
import os
import time
import shutil
import re

def readlines(f):
    with open(f, "r") as f:
        content = f.readlines()
    return content

def writelines(f, lines):
    with open(f, "w") as f:
        f.writelines(lines)

def newfile(extra, months):
    idx = 1
    fmt = "out/{}/{}/".format(extra, months) + "{:04d}.out"
    while os.path.isfile(fmt.format(idx)):
        idx += 1
    return fmt.format(idx)

def getmin(extra):
    try:
        return int(sorted(os.listdir("out/{}".format(extra)))[0])
    except:
        return 1000

def record_output(data_file, outfile, extra):
    d = os.path.dirname(outfile)
    if not os.path.exists(d):
        os.makedirs(d)
    check_output(["./snowball", "singlemode", data_file, outfile, str(extra)])

def main(maxshuf = 2500, extra_payments = None):
    if extra_payments is None: extra_payments = list()
    minimums = dict()
    content = readlines(LOAN_DATA_FILE)
    lastsec = time.time()
    tmpfile = "tmp-loan-data"

    for extra in extra_payments:
        print "Trying with a total payment of ${}/month.".format(extra)
        minimum_months = getmin(extra)
        for i in xrange(maxshuf):
            if i % 100 == 0 and time.time() - lastsec >= 1:
                lastsec = time.time()
                print "{}/{}".format(i,maxshuf)
            writelines(tmpfile, content)
            months = int(check_output(["./snowball", "batchmode",
                                       tmpfile, "@stdout",
                                       str(extra)])
                         .strip())
            if months <= minimum_months:
                if months < minimum_months:
                    minimum_months = months
                    print "new minimum: {} months".format(months)
                record_output(tmpfile, newfile(extra, months), extra)
            shuffle(content)
        print "Done trials.  Minimum was {} months.\n\n".format(minimum_months)
        minimums[extra] = minimum_months
    os.remove(tmpfile)
    return minimums

mins = main(2000, [4000,5000,6000])
for pmt in sorted(mins.keys()):
    print " ${:8.2f} => {:3} months".format(pmt,mins[pmt])
