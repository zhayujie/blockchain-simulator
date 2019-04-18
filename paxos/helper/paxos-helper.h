#ifndef PAXOS_HELPER_H
#define PAXOS_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/uinteger.h"
#include <map>

namespace ns3 {

 
class PaxosHelper
{
public:
    PaxosHelper (uint32_t totalNoNodes);

    std::map<uint32_t, std::vector<Ipv4Address>>   m_nodesConnectionsIps;
    
    ApplicationContainer Install (NodeContainer c);
    //NetDeviceContainer InstallConnection (NodeContainer c);

private:
    ObjectFactory               m_factory;
    int                         m_nodeNo;              
};
}

#endif