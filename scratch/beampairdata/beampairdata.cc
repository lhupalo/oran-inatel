/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 University of Padova, Dep. of Information Engineering, SIGNET lab.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdexcept>
#include "random_cinematics.h"
#include "mmwave_scenario.h"

using namespace std;
using namespace ns3;
using namespace mmwave;

int main(int argc, char *argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Usage("...");
    std::string path;
    int radius = 1;
    int trial = 0;
    int sector = 0;
    int mimo;
    int rseed;
    int simSeed;
    cmd.AddValue("path", "Path for setting up codebooks", path);
    cmd.AddValue("mimo", "MIMO Configuration", mimo);
    cmd.AddValue("radius", "Maximum radius for generating random numbers", radius);
    cmd.AddValue("trial", "Trial number", trial);
    cmd.AddValue("sector", "Angle sector", sector);
    cmd.AddValue("seed", "Seed for simulation", rseed);
    cmd.AddValue("runseed", "Seed for simulation", simSeed);
    cmd.Parse(argc, argv);

    // gNB is located at (0,0,10)
    double x0 = 0;
    double y0 = 0;
    double z0 = 10;
    Vector gnbpos(x0, y0, z0);

    // random position for UE device
    double inner_radius;
    double ue_height = 1.5;
    Vector uepos;
    Vector uevel;
    if (radius < 10)
    {
        NS_FATAL_ERROR( "Radius must be greater or equal than 10m" );
    }
    else if (radius == 10)
    {
        inner_radius = radius - 5;
    }
    else
    {
        inner_radius = radius - 10;
    }

    uepos = random_position(x0, y0, ue_height, radius, inner_radius, sector);
    uevel = random_velocity();

    // random seeds for statistically independent simulations
    std::vector<int> seeds;
    seeds.push_back(rseed);
    seeds.push_back(simSeed);

    // sampling time by trial
    double time = 0.2;

    // running simulation in a mmwave scenario
    mmwave_simulation(uepos, gnbpos, uevel, seeds, time, trial, path, mimo);

    return 0;
}
