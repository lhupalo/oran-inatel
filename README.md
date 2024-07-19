# Repository for Machine-Learning-Aided Method for Optimizing Beam Selection and Update Period in 5G Networks and Beyond

## Overview

This repository contains simulation scripts for generating a beam pair database based on the method proposed in "Machine-Learning-Aided Method for Optimizing Beam Selection and Update Period in 5G Networks and Beyond". The simulation scripts utilize the NS3 simulator to emulate a mmWave scenario ([NS3 mmWave](https://github.com/nyuwireless-unipd/ns3-mmwave)) consisting of a single user device randomly located relative to a gNB placed at the origin. Parameters for transmission and details of the data collection methodology are explained thoroughly in the proposed method section. Please refer to the Instructions section for guidance on dataset generation.

## Instructions

1. **Clone repository**: Clone the repository by running the following command:

    ```bash
    git clone https://github.com/lhupalo/oran-inatel.git
    ```

2. **Configure NS3 simulator**: Set up NS3 simulator with the following command:

    ```bash
    ./waf configure --enable-examples
    ```

3. **Build NS3 Simulator along mmWave module**: Build NS3 simulator including the mmWave module:

    ```bash
    ./waf build
    ```

4. **Generate beam pair around gNB**: Use the following command to generate beam pair data:

    ```bash
    ./beam_pair_data_all_sectors.sh <mimo_config> <cpu_number>
    ```

    Replace `<mimo_config>` with the MIMO configuration (options: 2, 4, 8) and `<cpu_number>` with the number of CPUs for parallel distributed simulations.

    >**OPTIONAL**: To generate beam pair data for a specific sector, use:
    > ```bash
    > ./beam_pair_data_by_sector.sh <mimo_config> <radius_value> <sector_number>
    > ```
    > Replace `<radius_value>` with the external radius of the ring for beam pair sampling and `<sector_number>` with the sector number (1 to 12).

    Simulation results are saved as TXT files in the `results` folder with names like `beam_pair_data_mimo=<mimo_config>x<mimo_config>_r=<radius_value>_sector=<sector_number>.txt`.

5. **Processing simulation files to create beam pair database**: After simulations, process beam pair data with:

    ```bash
    ./beam_pair_database.sh
    ```

    This script generates Parquet files organized by sector and rings for further analysis. The main file required for training ML models is `beam_pair_data_mimo=<mimo_config>x<mimo_config>.pqt`.

## Reproducing Results

To reproduce the results and figures presented in the manuscript, please follow these steps:

1. **Download artifacts**: Visit [Artifacts](https://drive.google.com/file/d/1tiQb5DxJHROuh8QkSgJyTMgb80n5qaqz/view?usp=sharing) and download the zip file containing all artifacts used in this study.
2. **Extract files**: Place the downloaded "reported_results.zip" file in this directory and extract its contents.
3. **Runnig Jupyter Notebook**: Refer to the "reproduce_results.ipynb" file for instructions on reproducing the figures and results.

## Cite us

If you find the paper and this repository useful in your research, please use the following BibTeX entry for citation.

```BibTeX
@article{marenco2024beampairml,
  author =       {Ludwing Marenco, Luiz E. Hupalo, Naylson F. Andrade and Felipe A. P. Figueiredo},
  title =        {Machine-Learning-Aided Method for Optimizing Beam Selection and Update Period in 5G Networks and Beyond},
  doi = {https://doi.org/10.21203/rs.3.rs-4546043/v1},
  year =         {2024}
}
