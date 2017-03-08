#!/bin/bash -e

for line in `sed '/^[$#]/d' $1`; do
  cmd="wget -nc $line"
  eval $cmd
done
