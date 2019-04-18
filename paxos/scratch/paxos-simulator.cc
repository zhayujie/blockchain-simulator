#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PaxosSimulator");

// 创建网络
void startSimulator (int N)
{
  NodeContainer nodes;
  nodes.Create (N);

  PaxosHelper paxosHelper (N);
  // 默认pointToPint只能连接两个节点，需要手动连接
  NetDeviceContainer devices;
  PointToPointHelper pointToPoint;

  
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("15ms"));
  uint32_t nNodes = nodes.GetN ();

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("1.0.0.0", "255.255.255.0");

  // 网络节点两两建立连接
  for (int i = 0; i < N; i++) {
      for (int j = 0; j < N && j != i; j++) {
          Ipv4InterfaceContainer interface;
          Ptr<Node> p1 = nodes.Get (i);
          Ptr<Node> p2 = nodes.Get (j);
          NetDeviceContainer device = pointToPoint.Install(p1, p2);
          
          interface.Add(address.Assign (device.Get(0)));
          interface.Add(address.Assign (device.Get(1)));

          paxosHelper.m_nodesConnectionsIps[i].push_back(interface.GetAddress(1));
          paxosHelper.m_nodesConnectionsIps[j].push_back(interface.GetAddress(0));

          // 创建新的网络: 如果不增加网络的话, 所有ip都在一个字网，而最后一块device会覆盖之前的设置，导致无法通过ip访问到之前的邻居节点
          // 应该的设置：每个device连接的两个节点在一个字网内，所以每分配一次ip，地址应该增加一个网段
          address.NewNetwork();
      }
  }
  ApplicationContainer paxosApp = paxosHelper.Install (nodes);

  paxosApp.Start (Seconds (0.0));
  paxosApp.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
}


int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  int N = 10;
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("PaxosNode", LOG_LEVEL_INFO);

  // 启动模拟器
  startSimulator(N);

  return 0;
}