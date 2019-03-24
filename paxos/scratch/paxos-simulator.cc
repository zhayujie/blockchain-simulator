#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PaxosSimulator");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("PaxosNode", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("5ms"));

  // 默认pointToPint只能连接两个节点，需要手动连接
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  

  PaxosHelper paxosHelper (2);

  // 给节点设置ip地址，安装应用程序
  ApplicationContainer paxosApp = paxosHelper.Install (interfaces, nodes);

  paxosApp.Start (Seconds (0.0));
  paxosApp.Stop (Seconds (10.0));





  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}