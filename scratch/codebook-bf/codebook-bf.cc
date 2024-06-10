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

#include "ns3/point-to-point-module.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/command-line.h"
#include <ns3/buildings-helper.h>
#include <ns3/buildings-module.h>
#include <ns3/packet.h>
#include <ns3/tag.h>
#include <ns3/queue-size.h>
#include "ns3/isotropic-antenna-model.h"
#include "ns3/opengym-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"

using namespace std;
using namespace ns3;
using namespace mmwave;

std::random_device rd{};
std::mt19937 gen1{rd()};

// ********************************************** Application ***************

class MyAppTag : public Tag
{
public:
  MyAppTag ()
  {
  }

  MyAppTag (Time sendTs) : m_sendTs (sendTs)
  {
  }

  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MyAppTag")
      .SetParent<Tag> ()
      .AddConstructor<MyAppTag> ();
    return tid;
  }

  virtual TypeId  GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  virtual void  Serialize (TagBuffer i) const
  {
    i.WriteU64 (m_sendTs.GetNanoSeconds ());
  }

  virtual void  Deserialize (TagBuffer i)
  {
    m_sendTs = NanoSeconds (i.ReadU64 ());
  }

  virtual uint32_t  GetSerializedSize () const
  {
    return sizeof (m_sendTs);
  }

  virtual void Print (std::ostream &os) const
  {
    std::cout << m_sendTs;
  }

  Time m_sendTs;
};


class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();
  void ChangeDataRate (DataRate rate);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);



private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::ChangeDataRate (DataRate rate)
{
  m_dataRate = rate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  MyAppTag tag (Simulator::Now ());

  m_socket->Send (packet);
  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}



void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void
RttChange (Ptr<OutputStreamWrapper> stream, Time oldRtt, Time newRtt)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRtt.GetSeconds () << "\t" << newRtt.GetSeconds () << std::endl;
}



static void Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << packet->GetSize () << std::endl;
}

// ************************************ Throughput *************************

// Before Run the simulation
double flowThr;
double avgTpt;
double totalRxBytes = 0;
double totalRxBytes_Alltime = 0;
double lastRx = 1e3;
double firstTx = 0;

void
MonitorFlow (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon)
{
  avgTpt = 0;

  flowMon->CheckForLostPackets();
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = flowStats.begin (); i != flowStats.end (); ++i)
  {
    totalRxBytes =+ i->second.rxBytes;

    if (i->second.timeFirstTxPacket.GetSeconds () > firstTx){
      firstTx = i->second.timeFirstTxPacket.GetSeconds ();
    }

    if (i->second.timeLastRxPacket.GetSeconds () < lastRx) {
      lastRx = i->second.timeLastRxPacket.GetSeconds ();
    }
  }

  avgTpt = (totalRxBytes - totalRxBytes_Alltime) * 8.0 / (firstTx - lastRx) / 1024 / 1024;
  // std::cout << avgTpt << " Mbps "  << Simulator::Now().GetSeconds() << std::endl;
  totalRxBytes_Alltime = totalRxBytes;
  
  Simulator::Schedule (Seconds (0.1), &MonitorFlow, fmhelper, flowMon);
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
  
  // const std::vector<double> velocities{sqrt(2)/2, 9*sqrt(2)/2, 22*sqrt(2)/2, 33*sqrt(2)/2};
  // std::uniform_int_distribution<> distr(0, velocities.size() - 1);
  // int index=distr(gen1);
  // double vx=velocities[index];
  // double vy=velocities[index];
  double vx=sqrt(2)/2;
  double vy=sqrt(2)/2;
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

  int scenario = 4;

  switch (scenario)
    {
    case 1:
      {
        Ptr < Building > building;
        building = Create<Building> ();
        building->SetBoundaries (Box (40.0,60.0,
                                      0.0, 6,
                                      0.0, 15.0));
        break;
      }
    case 2:
      {
        Ptr < Building > building1;
        building1 = Create<Building> ();
        building1->SetBoundaries (Box (60.0,64.0,
                                       0.0, 2.0,
                                       0.0, 1.5));

        Ptr < Building > building2;
        building2 = Create<Building> ();
        building2->SetBoundaries (Box (60.0,64.0,
                                       6.0, 8.0,
                                       0.0, 15.0));

        Ptr < Building > building3;
        building3 = Create<Building> ();
        building3->SetBoundaries (Box (60.0,64.0,
                                       10.0, 11.0,
                                       0.0, 15.0));
        break;
      }
    case 3:
      {
        Ptr < Building > building1;
        building1 = Create<Building> ();
        building1->SetBoundaries (Box (-180, -150,
                                       75, 100,
                                       0.0, 20));
        building1->SetNRoomsX (4);
        building1->SetNRoomsY (1);
        building1->SetNFloors (6);

        Ptr < Building > building2;
        building2 = Create<Building> ();
        building2->SetBoundaries (Box (-100,-70,
                                       75, 100,
                                       0.0, 30));
        building2->SetNRoomsX (4);
        building2->SetNRoomsY (1);
        building2->SetNFloors (8);

        Ptr < Building > building3;
        building3 = Create<Building> ();
        building3->SetBoundaries (Box (50, 80,
                                       20, 60,
                                       0.0, 40));
        building3->SetNRoomsX (4);
        building3->SetNRoomsY (1);
        building3->SetNFloors (10);

        Ptr < Building > building4;
        building4 = Create<Building> ();
        building4->SetBoundaries (Box (-100, -70,
                                       -40, -10,
                                       0.0, 30));
        building4->SetNRoomsX (4);
        building4->SetNRoomsY (1);
        building4->SetNFloors (8);

        Ptr < Building > building5;
        building5 = Create<Building> ();
        building5->SetBoundaries (Box (20, 50,
                                       -50, -20,
                                       0, 20));
        building5->SetNRoomsX (4);
        building5->SetNRoomsY (1);
        building5->SetNFloors (3);

        Ptr < Building > building6;
        building6 = Create<Building> ();
        building6->SetBoundaries (Box (100, 150,
                                       -100, -70,
                                       0.0, 30));
        building6->SetNRoomsX (4);
        building6->SetNRoomsY (1);
        building6->SetNFloors (8);
        break;
      }
      case 4:
      {
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Invalid scenario");
      }
    }


  // TCP settings
  bool tcp = false;
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpCubic::GetTypeId ()));
  Config::SetDefault ("ns3::TcpSocketBase::MinRto", TimeValue (MilliSeconds (1)));
  Config::SetDefault ("ns3::Ipv4L3Protocol::FragmentExpirationTimeout", TimeValue (MilliSeconds (1)));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (2500));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (131072*50));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (131072*50));

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024 * 1024));
  Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize", UintegerValue (1024 * 1024));
  Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (1024 * 1024));
  bool harqEnabled = true;
  bool rlcAmEnabled = true;
  Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue (rlcAmEnabled));
  Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue (harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (true));
  // Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (1)));
  Config::SetDefault ("ns3::LteRlcAm::PollRetransmitTimer", TimeValue (MilliSeconds (1.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReorderingTimer", TimeValue (MilliSeconds (1.0)));
  Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue (MilliSeconds (1.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MilliSeconds (1.0)));
  Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (20 * 1024 * 1024));

  // Set power and noise figure
  // Source: O-RAN.WG1.-MMIMO-USE-CASES-TR-v01.00, p. 75

  // 30 dBm = 1 W
  double gnbTxPower = 30;
  double ueTxPower = 20;
  double noiseFigure = 5;
  Config::SetDefault ("ns3::MmWavePhyMacCommon::Bandwidth", DoubleValue (100e6)); // 
  Config::SetDefault ("ns3::MmWaveEnbPhy::TxPower", DoubleValue (gnbTxPower));
  Config::SetDefault ("ns3::MmWaveEnbPhy::NoiseFigure", DoubleValue (noiseFigure));
  Config::SetDefault ("ns3::MmWaveUePhy::TxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::MmWaveUePhy::NoiseFigure", DoubleValue (noiseFigure));

  Config::SetDefault ("ns3::MmWaveHelper::UseCa",BooleanValue (false));
  Config::SetDefault ("ns3::MmWaveAmc::Ber", DoubleValue (0.001));
  

  Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();

  // choose the pathloss model to use
  // ptr_mmWave->SetChannelConditionModelType ("ns3::BuildingsChannelConditionModel");
  // ptr_mmWave->SetChannelConditionModelType ("ns3::MmWavePropagationLossModel");
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
  
  // configure the UE antennas:
  // 1. specify the path of the file containing the codebook
  ptr_mmWave->SetUeBeamformingCodebookAttribute ("CodebookFilename", StringValue ("/home/ludwing/Documentos/Inatel/XAD/repositories/xad_simu_beamforming/src/mmwave/model/Codebooks/1x2.txt"));
  // 2. set the antenna dimensions
  ptr_mmWave->SetUePhasedArrayModelAttribute ("NumRows", UintegerValue (1));
  ptr_mmWave->SetUePhasedArrayModelAttribute ("NumColumns", UintegerValue (2));
  
  // configure the BS antennas:
  // 1. specify the path of the file containing the codebook
  ptr_mmWave->SetEnbBeamformingCodebookAttribute ("CodebookFilename", StringValue ("/home/ludwing/Documentos/Inatel/XAD/repositories/xad_simu_beamforming/src/mmwave/model/Codebooks/2x2.txt"));
  ptr_mmWave->SetEnbPhasedArrayModelAttribute ("NumRows", UintegerValue (2));
  // 2. set the antenna dimensions
  ptr_mmWave->SetEnbPhasedArrayModelAttribute ("NumColumns", UintegerValue (2));



  ptr_mmWave->Initialize ();
  ptr_mmWave->SetHarqEnabled(true);

  Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  ptr_mmWave->SetEpcHelper (epcHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0.1)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr;
  remoteHostAddr = internetIpIfaces.GetAddress (1);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);



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

  bool random_walk = false;

  if (random_walk)
  {
    Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();

    // UE start position
    uePositionAlloc->Add (Vector (0, 25, 1.5));

    uemobility.SetMobilityModel("ns3::RandomWalk2dOutdoorMobilityModel",
                                "Bounds", RectangleValue (Rectangle (-75, 75, -75, 75)),
                                "Time", TimeValue (Seconds (0.1)),
                                "Distance", DoubleValue (3.0),
                                "Mode", EnumValue (RandomWalk2dOutdoorMobilityModel::MODE_TIME),
                                "Direction", StringValue ("ns3::UniformRandomVariable[Min=-0.2|Max=0.2]"),
                                "Speed", StringValue ("ns3::NormalRandomVariable[Mean=1|Variance=0.040401]"));
    
    uemobility.SetPositionAllocator (uePositionAlloc);
    uemobility.Install (ueNodes);
  }
  else
  {
    uemobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
    uemobility.Install (ueNodes);

    ueNodes.Get(0)->GetObject<MobilityModel> ()->SetPosition (uepos);
    ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (uevel);
  }



  NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  // Assign IP address to UEs, and install applications
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));


  ptr_mmWave->AttachToClosestEnb (ueNetDev, enbNetDev);
  ptr_mmWave->EnableTraces ();

  // Set the default gateway for the UE
  Ptr<Node> ueNode = ueNodes.Get (0);
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  if (tcp)
    {
      // Install and start applications on UEs and remote host
      uint16_t sinkPort = 20000;

      Address sinkAddress (InetSocketAddress (ueIpIface.GetAddress (0), sinkPort));
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      ApplicationContainer sinkApps = packetSinkHelper.Install (ueNodes.Get (0));

      sinkApps.Start (Seconds (0.));
      sinkApps.Stop (Seconds (time));

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (remoteHostContainer.Get (0), TcpSocketFactory::GetTypeId ());
      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3TcpSocket, sinkAddress, 1400, 5000000, DataRate ("500Mb/s"));

      remoteHostContainer.Get (0)->AddApplication (app);
      AsciiTraceHelper asciiTraceHelper;
      Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-window-newreno.txt");
      ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream1));

      Ptr<OutputStreamWrapper> stream4 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-rtt-newreno.txt");
      ns3TcpSocket->TraceConnectWithoutContext ("RTT", MakeBoundCallback (&RttChange, stream4));

      Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-data-newreno.txt");
      sinkApps.Get (0)->TraceConnectWithoutContext ("Rx",MakeBoundCallback (&Rx, stream2));

      //Ptr<OutputStreamWrapper> stream3 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-sstresh-newreno.txt");
      //ns3TcpSocket->TraceConnectWithoutContext("SlowStartThreshold",MakeBoundCallback (&Sstresh, stream3));
      app->SetStartTime (Seconds (0.1));
      app->SetStopTime (Seconds (time));
    }
  else
    {
      // Install and start applications on UEs and remote host
      uint16_t sinkPort = 20000;

      Address sinkAddress (InetSocketAddress (ueIpIface.GetAddress (0), sinkPort));
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      ApplicationContainer sinkApps = packetSinkHelper.Install (ueNodes.Get (0));

      sinkApps.Start (Seconds (0.));
      sinkApps.Stop (Seconds (time));

      Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (remoteHostContainer.Get (0), UdpSocketFactory::GetTypeId ());
      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3UdpSocket, sinkAddress, 1400, 5000000, DataRate ("500Mb/s"));

      remoteHostContainer.Get (0)->AddApplication (app);
      AsciiTraceHelper asciiTraceHelper;
      Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("mmWave-udp-data-am.txt");
      sinkApps.Get (0)->TraceConnectWithoutContext ("Rx",MakeBoundCallback (&Rx, stream2));

      app->SetStartTime (MilliSeconds (1));
      app->SetStopTime (Seconds (time));

    }

      // Adaptive Modulation and Coding instantiation
    Ptr<MmWaveAmc> amc = CreateObject <MmWaveAmc> (ptr_mmWave->GetCcPhyParams ().at (0).GetConfigurationParameters ());

    //p2ph.EnablePcapAll("mmwave-sgi-capture");
    Config::Set ("/NodeList/*/DeviceList/*/TxQueue/MaxSize", QueueSizeValue (QueueSize ("100000p")));

    // Connect to path to have the Rx tracing params callback
    // Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/MmWaveUePhy/DlSpectrumPhy/RxPacketTraceUe", MakeCallback (&TraceUeCallback));

    // Connect to path to have the directions tracing changes
    // Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChangeCallback));

    // Flow Monitor instatiation to evaluate throughput
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();
    flowMonitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    flowMonitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    flowMonitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));
    MonitorFlow (&flowHelper, flowMonitor); 

  int simSeed=1;
  RngSeedManager::SetSeed (simSeed);
  RngSeedManager::SetRun (simSeed);


  Simulator::Stop (Seconds (time));
  Simulator::Run ();
  Simulator::Destroy ();
}

int main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Usage ("...");
  double radius=1;
  cmd.AddValue("radius", "Maximum radius for generating random numbers", radius);
  
  
  cmd.Parse (argc, argv); 
  // std:: cout << unsigned(radius) << std:: endl;
  
  double x0=0; double y0=0; double z0=1.5;
  Vector gnbpos (x0, y0, z0);
  double r=radius; 
  double ri; 
  double zue=1.5;
  if(radius==30)
  {
    ri=15;
  }
  else
  {
    ri=radius-10; 
  }
  Vector uepos;
  Vector uevel;
  
  int trials=1;
  // double time=0.4;
  double time=0.1;
  for(int i=0; i<trials; i++)
  {
   
    uepos=randomposition(x0, y0, zue, r, ri);
    uevel=randomvelocity();
    std:: cout << "random position   " << uepos << std:: endl;
    simulation(uepos, gnbpos, uevel, time);
  }

  return 0;
}
