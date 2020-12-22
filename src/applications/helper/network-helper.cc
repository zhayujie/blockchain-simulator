#include "ns3/core-module.h" 
#include "network-helper.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

// 2.need changed to the the specific header file
#include "../model/raft-node.h"
// #include "../model/paxos-node.h"
// #include "../model/pbft-node.h"

namespace ns3 {
    NetworkHelper::NetworkHelper(uint32_t totalNoNodes) {
        // 3.need changed to a specific typeId
        m_factory.SetTypeId ("ns3::RaftNode");
        m_nodeNo = totalNoNodes;
    }
    
    ApplicationContainer
    NetworkHelper::Install (NodeContainer c)
    { 
        ApplicationContainer apps;
        for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
        {
            // 4.need changed to a specific protocol class
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