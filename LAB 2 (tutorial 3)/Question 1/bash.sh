#!/bin/bash

echo "points,threads,pi,error,time" > serial_results.csv
echo "points,threads,pi,error,time" > parallel_results.csv

POINTS=(10 100 10000 1000000 10000000 100000000 1000000000)
THREADS=(1 2 4 8 16)

echo "=========================="
echo " SERIAL"
echo "=========================="

for n in "${POINTS[@]}"
do
    echo "Serial: $n"

    ./question1 "$n" >> serial_results.csv
done


echo ""
echo "=========================="
echo " PTHREADS"
echo "=========================="

for n in "${POINTS[@]}"
do
    for t in "${THREADS[@]}"
    do
        echo "Pthreads: $n points, $t threads"

        ./parallel_question1 "$n" "$t" >> parallel_results.csv
    done
done

echo ""
echo "=========================="
echo " DONE"
echo "=========================="