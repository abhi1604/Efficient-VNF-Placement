#!/bin/bash

for i in 10 50 100 200 300 400 500 1000
do
	for j in {1..5}
	do
		./a.out $i
	done
done
