#!/bin/bash

cpu_cores=2
rm nohup.out

rm ./results2x2/*.txt

# below arguments of parallel: -j 2 = here, the 2 is the number of CPU cores. This is a parallelization solution. Use according to your hardware
#                              {starting radius..max radius..step radius} (start at r=20, and generate until r=100 in batches of r=10)
#                              {1..12} = this is the number of sectors which the 360 degrees circle is divided (here, 12 sectors of 30 degrees)
nohup parallel -j $cpu_cores  ./beam_pair_data_by_sector.sh {} ::: {20..100..10} ::: {1..12} &
