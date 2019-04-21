#ifndef NETWORK_HELPER_H
#define NETWORK_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/uinteger.h"
#include <map>

namespace ns3 {

 
class NetworkHelper
{
public:
    NetworkHelper (uint32_t totalNoNodes);

    std::map<uint32_t, std::vector<Ipv4Address>>   m_nodesConnectionsIps;
    
    ApplicationContainer Install (NodeContainer c);

private:
    ObjectFactory               m_factory;
    int                         m_nodeNo;              
};
}

#endif