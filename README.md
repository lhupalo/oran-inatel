# Repository for Machine-Learning-Aided Method for Optimizing Beam Selection and Update Period in 5G Networks and Beyond

## Overview

<!-- Copyright 2024 Inatel<br>
Luiz Eduardo Hupalo -->


<!-- This work is based on the mmwave repo: [GitHub NYU Wireless mmwave](https://github.com/nyuwireless-unipd/ns3-mmwave)

# Instructions

First of all, take a look on the mmwave repo mentioned previously. There, you'll find instructions of the required libs and packages to run ns3.

After that, clone this repo and run the following commands:

1. `./waf configure --enable-examples` (ok)
2. `./waf build` (ok)

<!-- Maybe it can occur errors related to the folder `src/opengym`. If that happens, run the following commands:

3. `cd src/opengym`
4. `pip3 install --user ./model/ns3gym/`

Then, re-run the commands 1 and 2. -->

<!-- ## Building the Scenario

You can build the scenario you want. This is done by modifying the `.cc` scripts inside each subfolder on `scratch/`.

IMPORTANT: AFTER FINISHING THE SETUP OF YOUR SCENARIO, MAKE SURE TO RUN THE BUILD COMMAND `./waf build`!!!

## Generating a Dataset

With the proper scenario built, let's generate a dataset.

There is three options here to generate a codebook beamforming dataset: MIMO 2x2, 4x4 and 8x8. 

Choose one and proceed as following (example here 4x4):

1. On script `randomdatasector4x4.sh`, optionally choose the number of positions collected per sector and radius. Default = 1000.
2. On script `runalldata4x4.sh`, adjust the parallelization arguments and define the range of generation: in which radius do you want to start, where to stop, the lenght of the steps of the process and in how sectors the circle will be divided. More details on the comments inside script.
3. Run `./runalldata4x4.sh`. Typically, it takes a lot of time. You can watch the process by the logs on `nohup.out`, or looking at the output files on `results4x4/`.
4. After finished, run the script `./datasector.sh results4x4`. Here, the argument is the output folder of step 3.
5. Finnaly, open the jupyter notebook available on `notebooks/handledata.ipynb` and follow the internal instructions. You'll have a parquet dataset containing (x, y, z) positions and speeds, the instantaneous received power and the beams and its pairs IDs used on that transmission. --> -->