#!/bin/bash
python3 genRandomcircuit.py
rm -rf "./P3" 
mkdir -p "./P3"
for (( i=0; i<=15; i=i+1 ))
do
> "./P3/t$i.log"
for (( j=1; j<=50; j=j+1 ))
do
CIRCUIT_PATH="../benchmark/t$i/case$j.qasm"
echo ---$CIRCUIT_PATH--- >> "./P3/t$i.log"
../qsyn -f P3.dof $CIRCUIT_PATH	>> "./P3/t$i.log"
done
done
python3 calculateMean.py