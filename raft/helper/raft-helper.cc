#include "ns3/core-module.h" 
#include "paxos-helper.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "../model/raft-node.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

namespace ns3 {
    
    // 工厂模式生成app实例， 在PaxosNode中必须要 GetTypeId方法，否则出错
    PaxosHelper::PaxosHelper(uint32_t totalNoNodes) {
        m_factory.SetTypeId ("ns3::RaftNode");
        m_nodeNo = totalNoNodes;
    }

    ApplicationContainer
    PaxosHelper::Install (NodeContainer c)
    { 
        ApplicationContainer apps;
        for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
        {
            Ptr<RaftNode> app = m_factory.Create<RaftNode> ();
            uint32_t nodeId = (*i)->GetId(); 
            app->m_id = nodeId;
            app->N = m_nodeNo;
            app->m_peersAddresses = m_nodesConnectionsIps[nodeId];
            (*i)->AddApplication (app);
            apps.Add (app);
        }
        return apps;
    }
}