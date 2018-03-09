#!/bin/bash

echo " STARTING TESTS"
echo "--------------------"
echo

i=0
FAILURE=0

for f in ./test/compile_test/*.bcc; do
    i=$((i + 1))
    ./bin/basic-cc $f /tmp/basic-cc_output$i > /tmp/basic-cc_result$i 2> /tmp/basic-cc_result_err$i
    if [ -s /tmp/basic-cc_result$i ] || [ -s /tmp/basic-cc_result_err$i ]; then
        FAILURE=$((FAILURE + 1))
        echo -e "[$i] $f: \e[31mFAIL\e[0m"
        cat /tmp/basic-cc_result_err$i
    else
        echo -e "[$i] $f: \e[32mPASS\e[0m"
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
