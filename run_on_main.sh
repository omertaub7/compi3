#!/bin/bash
make
./hw5 < main.txt > machine_code.llvm
lli machine_code.llvm