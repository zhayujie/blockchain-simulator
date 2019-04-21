#include "ns3/core-module.h" 
#include "network-helper.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

// 此处需要改为具体协议的头文件
#include "../model/raft-node.h"

namespace ns3 {
    
    // 工厂模式生成app实例， 在NetworkNode中必须要 GetTypeId方法，否则出错
    NetworkHelper::NetworkHelper(uint32_t totalNoNodes) {
        m_factory.SetTypeId ("ns3::RaftNode");
        m_nodeNo = totalNoNodes;
    }

    ApplicationContainer
    NetworkHelper::Install (NodeContainer c)
    { 
        ApplicationContainer apps;
        for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
        {
            // 此处需要改为具体协议的类
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