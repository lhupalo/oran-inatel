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

#include "ns3/trace-helper.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include <ns3/buildings-helper.h>
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include <ns3/buildings-module.h>
#include "ns3/isotropic-antenna-model.h"
#include <random>

using namespace std;
using namespace ns3;
using namespace mmwave;

std::random_device rd{};
std::mt19937 gen1{rd()};

void SetVelocitySim (Ptr<Node> node, Vector vel)
{
    Ptr<ConstantVelocityMobilityModel> mobility = node->GetObject<ConstantVelocityMobilityModel> ();
    mobility->SetVelocity (vel);
}


void SetPosition(Ptr<Node> node)
{
    Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
    x->SetAttribute ("Min", DoubleValue (0));
    x->SetAttribute ("Max", DoubleValue (10));
    double rn = x->GetValue ();
    std::cout << "random number   " << rn << std::endl;
    Ptr<ConstantPositionMobilityModel> mobility = node->GetObject<ConstantPositionMobilityModel> ();
    mobility->SetPosition (Vector (rn, rn, 1.5));
}

Vector randomposition(double x0, double y0, double zue, double r, double ri)
{
  std::uniform_real_distribution<double> dist1(0.0, 1.0);
  double random_r; double random_theta;
  bool stop=true;
  while(stop)
  {
    random_r=r*sqrt(dist1(gen1));
    random_theta=2*M_PI*dist1(gen1);
    if(random_r>=ri)
    {
      stop=false;
    }
    else
    {
      stop=true;
    }
  }
  // std:: cout << random_theta << std::endl;
  return Vector(x0 + random_r*cos(random_theta), y0 + random_r*sin(random_theta), zue);
}

Vector randomvelocity()
{
  
  const std::vector<double> velocities{sqrt(2)/2, 9*sqrt(2)/2, 22*sqrt(2)/2, 33*sqrt(2)/2};
  std::uniform_int_distribution<> distr(0, velocities.size() - 1);
  int index=distr(gen1);
  double vx=velocities[index];
  double vy=velocities[index];
  double vz=0;

  std::uniform_real_distribution<double> dist1(0.0, 1.0);

  if(dist1(gen1)<0.5)
  {
    vx=-1*vx;
  }
  if(dist1(gen1)>0.5)
  {
    vy=-1*vy;
  }

  // std:: cout << vx << "  " << vy << std:: endl;

  return Vector(vx, vy, vz);
}

void simulation(Vector uepos, Vector gnbpos, Vector uevel, double time)
{

  //LogComponentEnable ("ThreeGppChannelModel", LOG_LEVEL_INFO);
  //LogComponentEnable ("MmWaveBeamformingModel", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpClient", LOG_LEVEL_ALL);
  

  Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
  ptr_mmWave->SetChannelConditionModelType ("ns3::BuildingsChannelConditionModel");
  //ptr_mmWave->SetChannelConditionModelType ("ns3::MmWavePropagationLossModel");
  ptr_mmWave->SetChannelConditionModelType ("ns3::AlwaysLosChannelConditionModel");

  Config::SetDefault ("ns3::MmWaveHelper::PathlossModel", StringValue ("ns3::ThreeGppUmaPropagationLossModel"));
  Config::SetDefault ("ns3::ThreeGppChannelModel::Scenario", StringValue ("UMa"));
  Config::SetDefault ("ns3::ThreeGppChannelModel::Blockage", BooleanValue (false)); // Enable/disable the blockage model
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (1.0)));
  // Carrier frequency
  Config::SetDefault ("ns3::ThreeGppChannelModel::Frequency", DoubleValue (28.0e9));
  // by default, isotropic antennas are used. To use the 3GPP radiation pattern instead, use the <ThreeGppAntennaArrayModel>
  Config::SetDefault ("ns3::PhasedArrayModel::AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ())); 

  
  // select the beamforming model
  ptr_mmWave->SetBeamformingModelType("ns3::MmWaveCodebookBeamforming");
  
  // configure the UE antennas:
  // 1. specify the path of the file containing the codebook
  ptr_mmWave->SetUeBeamformingCodebookAttribute ("CodebookFilename", StringValue ("/home/ludwig/InatelRepos/xad_simu_beamforming/src/mmwave/model/Codebooks/1x2.txt"));
  // 2. set the antenna dimensions
  ptr_mmWave->SetUePhasedArrayModelAttribute ("NumRows", UintegerValue (1));
  ptr_mmWave->SetUePhasedArrayModelAttribute ("NumColumns", UintegerValue (2));
  
  // configure the BS antennas:
  // 1. specify the path of the file containing the codebook
  ptr_mmWave->SetEnbBeamformingCodebookAttribute ("CodebookFilename", StringValue ("/home/ludwig/InatelRepos/xad_simu_beamforming/src/mmwave/model/Codebooks/8x8.txt"));
  ptr_mmWave->SetEnbPhasedArrayModelAttribute ("NumRows", UintegerValue (8));
  // 2. set the antenna dimensions
  ptr_mmWave->SetEnbPhasedArrayModelAttribute ("NumColumns", UintegerValue (8));



  ptr_mmWave->Initialize ();

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (1);
 
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (gnbpos);
  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  
  MobilityHelper uemobility;
  uemobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  uemobility.Install (ueNodes);
  ueNodes.Get(0)->GetObject<MobilityModel> ()->SetPosition (uepos);
  ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (uevel);

  // Simulator::Schedule(Seconds(Simulator::Now().GetSeconds()+0.01), &SetPosition, ueNodes.Get(0));
  // Simulator::ScheduleNow(&SetPosition, ueNodes.Get(0));


  NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice (ueNodes);


  ptr_mmWave->AttachToClosestEnb (ueNetDev, enbNetDev);
  ptr_mmWave->EnableTraces ();

  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::NGBR_VOICE_VIDEO_GAMING;
  EpsBearer bearer (q);
  ptr_mmWave->ActivateDataRadioBearer (ueNetDev, bearer);


  Simulator::Stop (Seconds (time));
  Simulator::Run ();
  Simulator::Destroy ();
}


int main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Usage ("...");
  int radius=1;
  cmd.AddValue("radius", "Maximum radius for generating random numbers", radius);
  
  
  cmd.Parse (argc, argv); 
  // std:: cout << unsigned(radius) << std:: endl;
  
  double x0=0; double y0=0; double z0=1.5;
  Vector gnbpos (x0, y0, z0);
  double r=radius;
  double ri;
  double zue=1.5;
  Vector uepos;
  Vector uevel;
  if(radius==30)
  {
    ri=15;
  }
  else
  {
    ri=radius-10; 
  }
  



  int trials=1000;
  // double time=0.2;
  double time=0.02;
  for(int i=0; i<trials; i++)
  {
    uepos=randomposition(x0, y0, zue, r, ri);
    uevel=randomvelocity();
    std:: cout << "random position   " << ri << std:: endl;
    simulation(uepos, gnbpos, uevel, time);
  }

  return 0;
}
