#! /usr/bin/env bash
if [[ $1 =~ $2 ]]; then
    echo "pass"
else
    echo "fail"
fi
