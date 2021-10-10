#!/bin/bash
assert() 
{
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 21 '5-4+20'
assert 14 '3   + 23  - 12'
assert 54 '18 * 3'
assert 5 '20 / 4'
assert 21 '3 + 7 * (3-1) + 4'
assert 15 '3 *+ 5'
assert 3 '-3 + 6'

echo OK