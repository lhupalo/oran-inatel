#!/bin/bash

mimo=$1
radio=$2
secto=$3
trials=1000

echo "Simulating mimo=$mimo"x"$mimo, r=$radio and sector=$secto"

for counter in $(seq 1 $trials); do
  seedv=$(( $RANDOM % 1000 + 1 ))
  runseedv=$(( $RANDOM % 1000 + 1 ))
  ./waf --run-no-build "beampairdata 
  --path=$PWD 
  --mimo=$mimo 
  --radius=$radio 
  --trial=$counter 
  --sector=$secto 
  --seed=$seedv 
  --runseed=$runseedv" --cwd=./results/ >> ./results/beam_pair_data_mimo=$mimo"x"$mimo"_"r=$radio"_"sector=$secto.txt
done
echo "Finished mimo=$mimo"x"$mimo, r=$radio and sector=$secto"

