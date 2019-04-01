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
  int N = 1000;
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("PaxosNode", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (N);

  // 默认pointToPint只能连接两个节点，需要手动连接
  NetDeviceContainer devices;
  
  PaxosHelper paxosHelper (N);

  //PointToPointHelper pointToPoint;
  //devices = pointToPoint.Install(nodes);
  // devices = paxosHelper.InstallConnection (nodes);
  PointToPointHelper pointToPoint;

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("15ms"));
  uint32_t nNodes = nodes.GetN ();

  // 节点间建立两两通道
  /*
  for(int i = 0; i < nNodes-1; i += 2)
  {   
    Ptr<Node> p1 = nodes.Get (i);
    Ptr<Node> p2 = nodes.Get (i+1);
    devices.Add(pointToPoint.Install(p1, p2));
  }
  */
 
  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("1.0.0.0", "255.255.255.0");

  
/*
  for(int i = 0; i < nNodes-1; i ++)
  { 
    for (int j = i+1; j < nNodes; j++) {
        Ipv4InterfaceContainer interface;
        Ptr<Node> p1 = nodes.Get (i);
        Ptr<Node> p2 = nodes.Get (j);
        NetDeviceContainer device = pointToPoint.Install(p1, p2);
        interface.Add(address.Assign (device.Get(0)));
        interface.Add(address.Assign (device.Get(1)));

        paxosHelper.m_nodesConnectionsIps[i].push_back(interface.GetAddress(1));
        paxosHelper.m_nodesConnectionsIps[j].push_back(interface.GetAddress(0));
    }
  }
  */

    // node0和所有邻节点建立连接
    for (int j = 1; j < N; j++) {
      
        Ipv4InterfaceContainer interface;
        Ptr<Node> p1 = nodes.Get (0);
        Ptr<Node> p2 = nodes.Get (j);
        NetDeviceContainer device = pointToPoint.Install(p1, p2);
        
        interface.Add(address.Assign (device.Get(0)));
        interface.Add(address.Assign (device.Get(1)));

        paxosHelper.m_nodesConnectionsIps[0].push_back(interface.GetAddress(1));
        //paxosHelper.m_nodesConnectionsIps[j].push_back(interface.GetAddress(0));

        // 创建新的网络: 如果不增加网络的话, 所有ip都在一个字网，而最后一块device会覆盖之前的设置，导致无法通过ip访问到之前的邻居节点
        // 应该的设置：每个device连接的两个节点在一个字网内，所以每分配一次ip，地址应该增加一个网段
        address.NewNetwork();
    }

    // node1和所有邻节点建立连接
    for (int j = 0; j < N; j++) {
        if (j == 1) {
          continue;
        }
        Ipv4InterfaceContainer interface;
        Ptr<Node> p1 = nodes.Get (1);
        Ptr<Node> p2 = nodes.Get (j);
        NetDeviceContainer device = pointToPoint.Install(p1, p2);
        
        interface.Add(address.Assign (device.Get(0)));
        interface.Add(address.Assign (device.Get(1)));

        paxosHelper.m_nodesConnectionsIps[1].push_back(interface.GetAddress(1));
        //paxosHelper.m_nodesConnectionsIps[j].push_back(interface.GetAddress(0));

        // 创建新的网络: 如果不增加网络的话, 所有ip都在一个字网，而最后一块device会覆盖之前的设置，导致无法通过ip访问到之前的邻居节点
        // 应该的设置：每个device连接的两个节点在一个字网内，所以每分配一次ip，地址应该增加一个网段
        address.NewNetwork();
    }


    for (int j = 0; j < N; j++) {
        if (j == 2) {
          continue;
        }
        Ipv4InterfaceContainer interface;
        Ptr<Node> p1 = nodes.Get (2);
        Ptr<Node> p2 = nodes.Get (j);
        NetDeviceContainer device = pointToPoint.Install(p1, p2);
        
        interface.Add(address.Assign (device.Get(0)));
        interface.Add(address.Assign (device.Get(1)));

        paxosHelper.m_nodesConnectionsIps[2].push_back(interface.GetAddress(1));
        //paxosHelper.m_nodesConnectionsIps[j].push_back(interface.GetAddress(0));

        // 创建新的网络: 如果不增加网络的话, 所有ip都在一个字网，而最后一块device会覆盖之前的设置，导致无法通过ip访问到之前的邻居节点
        // 应该的设置：每个device连接的两个节点在一个字网内，所以每分配一次ip，地址应该增加一个网段
        address.NewNetwork();
    }

    /*
    for (int j = 0; j < N; j++) {
        if (j == 3) {
          continue;
        }
        Ipv4InterfaceContainer interface;
        Ptr<Node> p1 = nodes.Get (3);
        Ptr<Node> p2 = nodes.Get (j);
        NetDeviceContainer device = pointToPoint.Install(p1, p2);
        
        interface.Add(address.Assign (device.Get(0)));
        interface.Add(address.Assign (device.Get(1)));

        paxosHelper.m_nodesConnectionsIps[3].push_back(interface.GetAddress(1));
        //paxosHelper.m_nodesConnectionsIps[j].push_back(interface.GetAddress(0));

        // 创建新的网络: 如果不增加网络的话, 所有ip都在一个字网，而最后一块device会覆盖之前的设置，导致无法通过ip访问到之前的邻居节点
        // 应该的设置：每个device连接的两个节点在一个字网内，所以每分配一次ip，地址应该增加一个网段
        address.NewNetwork();
    }
    */


    
    Ptr<Node> p0 = nodes.Get(0);
    for (int i = 0; i < 10; i++) {
        //NS_LOG_INFO("device" << i << ": "<< p0->GetDevice(i)->GetReferenceCount());
        //std::cout << p0->GetDevice(i)->GetAddress() << std::endl;
      //std::cout << p0->GetNDevices();
    }
    
  // 给节点设置ip地址，安装应用程序
  ApplicationContainer paxosApp = paxosHelper.Install (nodes);

  for (int i = 0; i < nodes.GetN(); i++) {
      Ptr<Ipv4> ipv4 = nodes.Get(i)->GetObject<Ipv4> (); // Get Ipv4 instance of the node
      Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal (); 
      //NS_LOG_INFO(i << ": " << addr);
      //std::cout << addr << "\n";
      //std::cout << paxosHelper.m_nodesConnectionsIps[5][3] << "";
  }

  paxosApp.Start (Seconds (0.0));
  paxosApp.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}