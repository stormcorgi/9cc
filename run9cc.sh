#!/bin/bash

make 9cc >/dev/null
./9cc "$(cat "$1")" >tmp9cc.s
cc -o tmp9cc tmp9cc.s
./tmp9cc
echo $?

make clean >/dev/null
