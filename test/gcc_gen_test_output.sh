#!/bin/bash

# Creates 'output' files for all of the ./output_test files

for f in ./test/output_test/*.bcc; do
    gcc -xc $f -o ./test_exe
    ./test_exe > $f.output
done

