#!/usr/bin/env python

import sys
import re
import os


def make_tests(rv_tests_dir):
    # print(rv_tests_dir)
    # print(dest_dir)
    files = [f for f in os.listdir(rv_tests_dir) 
            if (os.path.isfile(os.path.join(rv_tests_dir, f)) 
                & (".dump" not in f)
                & ("Make" not in f)
                & ("git" not in f)
                )]
    # print(files)

    for f in files:
        f = os.path.join(rv_tests_dir, f)
        filename = f.split("/")[-1]
        os.system("riscv64-unknown-elf-objcopy -O binary " +
                f + " " + filename + ".bin")
                



if __name__ == "__main__":
    if len(sys.argv) != 2:
        print ("Usage: ./test.py <riscv-tests-dir> ")
        exit(1)
    rv_tests_dir = sys.argv[1] + "/isa"
    make_tests(rv_tests_dir)