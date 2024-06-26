#!/bin/bash

mimo=$1
cpu_cores=$2

rm nohup.out
rm ./results/*mimo=$mimo"x"$mimo*

nohup parallel -j $cpu_cores  ./beam_pair_data_by_sector.sh {} ::: $mimo ::: {10..100..10} ::: {1..12} &
