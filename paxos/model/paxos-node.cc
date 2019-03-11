#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "paxos-node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PaxosNode");

NS_OBJECT_ENSURE_REGISTERED (PaxosNode);


TypeId
PaxosNode::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::PaxosNode")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<PaxosNode> ()
    ;

    return tid;
} 

PaxosNode::PaxosNode(void) {

}

PaxosNode::~PaxosNode(void) {
    NS_LOG_FUNCTION (this);

}

void 
PaxosNode::StartApplication ()              // Called at time specified by Start
{
    t_max = 0;
    command = "";
    t_store = 0;
    ticket = 0;
    std::cout << "start server ";
    NS_LOG_INFO("log info");
    // m_socket->SetRecvCallback (MakeCallback (&PaxosNode::HandleRead, this));
}

void 
PaxosNode::StopApplication ()             // Called at time specified by Stop
{
    
}

void 
PaxosNode::HandleRead (Ptr<Socket> socket, const Address& from)
{   

}

/*
void 
PaxosNode::SendMessage (enum Message responseMessage, std::string msg, Ptr<Socket> outgoingSocket)
{   

}
*/

} // namespace ns3