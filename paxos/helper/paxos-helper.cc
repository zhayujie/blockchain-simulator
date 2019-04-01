 #include "paxos-helper.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "../model/paxos-node.h"
#include "ns3/internet-module.h"

namespace ns3 {
    

    // 工厂模式生成app实例， 在PaxosNode中必须要 GetTypeId方法，否则出错
    PaxosHelper::PaxosHelper(uint32_t totalNoNodes) {
        m_factory.SetTypeId ("ns3::PaxosNode");
        m_nodeNo = totalNoNodes;
    }

    ApplicationContainer
    PaxosHelper::Install (NodeContainer c)
    { 
        ApplicationContainer apps;
        int j = 0;
        for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
        {
            apps.Add (InstallPriv (*i, j));
            j++;
        }
        //std::cout<<"hello"<< interfece.GetN();

        return apps;
    }

    Ptr<Application>
    PaxosHelper::InstallPriv (Ptr<Node> node, int j)
    {
        Ptr<PaxosNode> app = m_factory.Create<PaxosNode> ();
        //app->SetPeersAddresses(m_peersAddresses);
        uint32_t nodeId = node->GetId(); 
        app->m_id = nodeId;
        app->N = m_nodeNo;

        //auto ip = interfece.GetAddress(j);
        /*
        app->m_local = ip;
        for (int i = 0; i < m_nodeNo; i++) {
            //if (i != j) {
                app->m_peersAddresses.push_back(interfece.GetAddress(i));
            //}
        }
        */
        app->m_peersAddresses = m_nodesConnectionsIps[nodeId];
        node->AddApplication (app);

        return app;
    }

    /*
    NetDeviceContainer 
    PaxosHelper::InstallConnection (NodeContainer c)
    {
        PointToPointHelper pointToPoint;
        pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
        pointToPoint.SetChannelAttribute ("Delay", StringValue ("5ms"));

        uint32_t nNodes = c.GetN ();
        NetDeviceContainer newDevices;

        for(int i = 0; i < nNodes-1; i++)
        {   
            for (int j = i+1; j < nNodes; j++) {
                Ptr<Node> p1 = c.Get (i);
                Ptr<Node> p2 = c.Get (j);
                newDevices.Add(pointToPoint.Install(p1, p2));
            } 
        }
        return newDevices;
    }
    */
}