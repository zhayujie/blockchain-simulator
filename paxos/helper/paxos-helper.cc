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
    }

    ApplicationContainer
    PaxosHelper::Install (Ipv4InterfaceContainer interfece, NodeContainer c)
    { 
        ApplicationContainer apps;
        int j = 0;
        for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
        {
            apps.Add (InstallPriv (*i, interfece, j));
            j++;
        }

        return apps;
    }

    Ptr<Application>
    PaxosHelper::InstallPriv (Ptr<Node> node, Ipv4InterfaceContainer interfece, int j)
    {
        Ptr<PaxosNode> app = m_factory.Create<PaxosNode> ();
        //app->SetPeersAddresses(m_peersAddresses);
        uint32_t nodeId = node->GetId(); 
        app->m_id = nodeId;

        auto ip0 = interfece.GetAddress(0);
        auto ip1 = interfece.GetAddress(1);

        if (j == 0) 
        {
            app->m_local = ip0;
            app->m_peersAddresses.push_back(ip1);
        } 
        else if (j == 1)
        {
            app->m_local = ip1;
            app->m_peersAddresses.push_back(ip0);
        }
        
        node->AddApplication (app);

        return app;
    }
}