# This script find the optimal sort, starting from avalanche and
# working towards snowball until there is a substantial time cost.

LOAN_DATA_FILE = "test.dat"
TEMP_FILE = "ordered-loans.tmp"

from subprocess import call, check_output
from random import shuffle
import os
import time
import shutil
import re

def readlines(f):
    with open(f, "r") as f:
        content = f.readlines()
    return content

def writelines(f, content):
    with open(f, "w") as f:
        f.writelines(content)
    return content

def newfile(extrapmt, months):
    idx = 1
    fmt = "{}.out/{}/{}/".format(LOAN_DATA_FILE, extrapmt, months) + "{:04d}.out"
    while os.path.isfile(fmt.format(idx)):
        idx += 1
    return fmt.format(idx)

def getmin(extrapmt):
    try:
        return int(sorted(os.listdir("{}.out/{}".format(LOAN_DATA_FILE, extrapmt)))[0])
    except:
        return 1000

def record_output(data_file, outfile, extrapmt):
    d = os.path.dirname(outfile)
    if not os.path.exists(d):
        os.makedirs(d)
    with open(data_file, "r") as infile:
        with open(outfile, "w") as outfile:
            call(["./snowball", "report", str(extrapmt)],
                 stdin=infile, stdout=outfile)

def getmonths(data_file, extrapmt):
    args = ["./snowball", "simple", str(extrapmt)]
    code = None
    with open(data_file, "r") as f:
        code = int(check_output(args, stdin=f).strip())
    return code

print getmonths(LOAN_DATA_FILE, 10000)
record_output(LOAN_DATA_FILE, newfile(10000, 16), 10000)
