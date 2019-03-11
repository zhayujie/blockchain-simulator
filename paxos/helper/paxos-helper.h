#ifndef PAXOS_HELPER_H
#define PAXOS_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/uinteger.h"

namespace ns3 {

 
class PaxosHelper
{
public:
    PaxosHelper (void);

    std::vector<Ipv4Address>    m_peersAddresses;
    ApplicationContainer Install (NodeContainer c);


private:
    ObjectFactory               m_factory;
    Ptr<Application> InstallPriv (Ptr<Node> node);
};
}

#endif