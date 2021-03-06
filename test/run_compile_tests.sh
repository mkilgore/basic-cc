#!/bin/bash

echo " STARTING TESTS"
echo "--------------------"
echo

mkdir -p /tmp/bcc
TMP=/tmp/bcc

rm -fr $TMP/*

i=0
FAILURE=0

USE_VALGRIND=0

for f in ./test/compile_test/*.bcc; do
    i=$((i + 1))
    ./bin/basic-cc --gen assembler $f -o $TMP/basic-cc_output$i > $TMP/basic-cc_result$i 2> $TMP/basic-cc_result_err$i
    if [ -s $TMP/basic-cc_result$i ] || [ -s $TMP/basic-cc_result_err$i ]; then
        FAILURE=$((FAILURE + 1))
        echo -e "[$i] $f: \e[31mFAIL\e[0m"
        cat $TMP/basic-cc_result_err$i
    else
        echo -e "[$i] $f: \e[32mPASS\e[0m"
    fi
done

for f in ./test/buggy_test/*.bcc; do
    i=$((i + 1))
    ./bin/basic-cc --gen assembler $f -o $TMP/basic-cc_bug_output$i > $TMP/basic-cc_bug_result$i 2> $TMP/basic-cc_bug_result_err$i
    if [ -s $TMP/basic-cc_bug_result$i ] || [ -s $TMP/basic-cc_bug_result_err$i ]; then
        echo -e "[$i] $f: \e[32mPASS\e[0m"
    else
        FAILURE=$((FAILURE + 1))
        echo -e "[$i] $f: \e[31mFAIL\e[0m"
    fi
done

for f in ./test/output_test/*.bcc; do
    i=$((i + 1))
    ./bin/basic-cc --gen assembler $f -o $TMP/basic-cc_cmp_output$i > $TMP/basic-cc_cmp_result$i 2> $TMP/basic-cc_cmp_result_err$i
    if [ -s $TMP/basic-cc_result$i ] || [ -s $TMP/basic-cc_cmp_result_err$i ]; then
        FAILURE=$((FAILURE + 1))
        echo -e "[$i] $f: \e[31mFAIL\e[0m"
        echo "  - Did not compile:"
        cat $TMP/basic-cc_cmp_result_err$i
    else
        rm -fr ./test/bin/test$i
        gcc -x assembler -g -m32 -o ./test/bin/test$i $TMP/basic-cc_cmp_output$i
        if [ ! -s ./test/bin/test$i ]; then
            FAILURE=$((FAILURE + 1))
            echo -e "[$i] $f: \e[31mFAIL\e[0m"
            echo "  - Did not assemble:"
        else
            if [[ $USE_VALGRIND != 0 ]]; then
                valgrind --quiet --error-exitcode=1 \
                    --leak-check=full --show-possibly-lost=yes --show-reachable=yes \
                    ./test/bin/test$i > $TMP/basic-cc_result$i
            else
                ./test/bin/test$i > $TMP/basic-cc_result$i
            fi
            let exit_code=$?

            if [[ $exit_code == 0 ]] && cmp --silent $TMP/basic-cc_result$i $f.output; then
                echo -e "[$i] $f: \e[32mPASS\e[0m"
            else
                FAILURE=$((FAILURE + 1))
                echo -e "[$i] $f: \e[31mFAIL\e[0m"
                echo "  - Return code: $error_code"
                echo "  - Expected: `cat $f.output`"
                echo "  - Got: `cat $TMP/basic-cc_result$i`"
            fi
        fi
    fi
done

echo
echo "--------------------"
echo " FINISHED TESTS"

if [ "$FAILURE" != "0" ]; then
    echo -e "Result:\e[31m FAILED: $FAILURE\e[0m"
else
    echo -e "Result:\e[32m PASSED\e[0m"
fi

exit $FAILURE
