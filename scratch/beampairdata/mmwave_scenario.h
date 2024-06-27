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


using namespace std;
using namespace ns3;
using namespace mmwave;


void mmwave_simulation(Vector uepos, Vector gnbpos, Vector uevel, std::vector<int> seeds, double time, int trial, std::string path, int mimo)
{

  // 30 dBm = 1 W
  double gnbTxPower = 20;
  double ueTxPower = 15;
  double noiseFigure = 7;
  Config::SetDefault ("ns3::MmWavePhyMacCommon::Bandwidth", DoubleValue (100e6)); // 
  Config::SetDefault ("ns3::MmWaveEnbPhy::TxPower", DoubleValue (gnbTxPower));
  Config::SetDefault ("ns3::MmWaveEnbPhy::NoiseFigure", DoubleValue (noiseFigure));
  Config::SetDefault ("ns3::MmWaveUePhy::TxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::MmWaveUePhy::NoiseFigure", DoubleValue (noiseFigure));

  Config::SetDefault ("ns3::MmWaveHelper::UseCa",BooleanValue (false));
  Config::SetDefault ("ns3::MmWaveAmc::Ber", DoubleValue (0.001));

  Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
  // ptr_mmWave->SetChannelConditionModelType ("ns3::BuildingsChannelConditionModel");
  //ptr_mmWave->SetChannelConditionModelType ("ns3::MmWavePropagationLossModel");
  ptr_mmWave->SetChannelConditionModelType ("ns3::AlwaysLosChannelConditionModel");

  // choose the spectrum propagation loss model
  ptr_mmWave->SetChannelModelType ("ns3::ThreeGppSpectrumPropagationLossModel");

  Config::SetDefault ("ns3::MmWaveHelper::PathlossModel", StringValue ("ns3::ThreeGppUmaPropagationLossModel"));
  Config::SetDefault ("ns3::ThreeGppChannelModel::Scenario", StringValue ("UMa"));
  Config::SetDefault ("ns3::ThreeGppChannelModel::Blockage", BooleanValue (false)); // Enable/disable the blockage model
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (1)));
  // Carrier frequency
  Config::SetDefault ("ns3::ThreeGppChannelModel::Frequency", DoubleValue (28.0e9));
  // by default, isotropic antennas are used. To use the 3GPP radiation pattern instead, use the <ThreeGppAntennaArrayModel>
  Config::SetDefault ("ns3::PhasedArrayModel::AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ())); 

  
  // select the beamforming model
  ptr_mmWave->SetBeamformingModelType("ns3::MmWaveCodebookBeamforming");
  Config::SetDefault ("ns3::MmWaveCodebookBeamforming::UpdatePeriod", TimeValue (MilliSeconds (1)));
  Config::SetDefault ("ns3::MmWaveCodebookBeamforming::Trial", UintegerValue (trial));
  // configure the UE antennas:
  // 1. specify the path of the file containing the codebook
  ptr_mmWave->SetUeBeamformingCodebookAttribute ("CodebookFilename", StringValue (path + "/src/mmwave/model/Codebooks/1x2.txt"));
  // 2. set the antenna dimensions
  ptr_mmWave->SetUePhasedArrayModelAttribute ("NumRows", UintegerValue (1));
  ptr_mmWave->SetUePhasedArrayModelAttribute ("NumColumns", UintegerValue (2));
  
  // configure the BS antennas:
  // 1. specify the path of the file containing the codebook
  ptr_mmWave->SetEnbBeamformingCodebookAttribute ("CodebookFilename", StringValue (path + "/src/mmwave/model/Codebooks/" + std::to_string(mimo) +"x" + std::to_string(mimo) +".txt"));
  ptr_mmWave->SetEnbPhasedArrayModelAttribute ("NumRows", UintegerValue (mimo));
  // 2. set the antenna dimensions
  ptr_mmWave->SetEnbPhasedArrayModelAttribute ("NumColumns", UintegerValue (mimo));
  


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


  NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice (ueNodes);


  ptr_mmWave->AttachToClosestEnb (ueNetDev, enbNetDev);
  ptr_mmWave->EnableTraces ();

  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::NGBR_VOICE_VIDEO_GAMING;
  EpsBearer bearer (q);
  ptr_mmWave->ActivateDataRadioBearer (ueNetDev, bearer);

  RngSeedManager::SetSeed (seeds[0]);
  RngSeedManager::SetRun (seeds[1]);

  Simulator::Stop (Seconds (time));
  Simulator::Run ();
  Simulator::Destroy ();
}