#!/bin/bash

#strace -c ./ltrace_test
#ltrace -S -T ./ltrace_test
#objdump -d ./ltrace_test

ltrace -o ./ltrace/ltrace_output.txt ./build/ltrace/ltrace_test