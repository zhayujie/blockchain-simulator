#include "paxos-helper.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "../model/paxos-node.h"

namespace ns3 {
    // 工厂模式生成app实例， 在PaxosNode中必须要 GetTypeId方法，否则出错
    PaxosHelper::PaxosHelper(void) {
        m_factory.SetTypeId ("ns3::PaxosNode");
    }

    ApplicationContainer
    PaxosHelper::Install (NodeContainer c)
    { 
        ApplicationContainer apps;
        for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
        {
            apps.Add (InstallPriv (*i));
        }

        return apps;
    }

    Ptr<Application>
    PaxosHelper::InstallPriv (Ptr<Node> node)
    {
        Ptr<PaxosNode> app = m_factory.Create<PaxosNode> ();
        //app->SetPeersAddresses(m_peersAddresses);
        
        node->AddApplication (app);

        return app;
    }
}