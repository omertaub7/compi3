#!/bin/bash
make
./hw5 < test.in > machine_code.llvm
lli machine_code.llvm > test.res
diff test.res test.out
