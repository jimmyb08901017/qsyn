#!/bin/bash

for (( i=0; i<=15; i=i+1 ))
do
for (( j=1; j<=50; j=j+1 ))
do
CIRCUIT_PATH="benchmark/t$i/case$j.qasm"
echo ---$CIRCUIT_PATH---
../qsyn -f P3.dof $CIRCUIT_PATH	
done
done