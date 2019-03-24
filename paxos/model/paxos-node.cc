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
#include "stdlib.h"

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


static char intToChar(int a) {
    return a + '0';
}

static int charToInt(char a) {
    return a - '0';
}


void 
PaxosNode::StartApplication ()              // Called at time specified by Start
{
    // 初始化paxos参数
    t_max = 0;
    command = 'e';
    t_store = 0;
    ticket = 0;
    isCommit = 0;
    proposal = intToChar(m_id);

    N = 5;
    vote = 0;
    round = 0;


    std::cout << "start server ";
    // NS_LOG_INFO("log info");
    // m_socket->SetRecvCallback (MakeCallback (&PaxosNode::HandleRead, this));
    NS_LOG_INFO("Node " << m_id << " start, IP: " << m_local); //<< " neighbor: " << m_peersAddresses[0]);
    //NS_LOG_INFO("id: " << m_id);
    //NS_LOG_INFO("typeId: " <<GetTypeId());


    // 初始化socket
    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket (GetNode (), tid);
        //NS_LOG_INFO("m_socket1: " << m_socket);
        m_socket->Bind (InetSocketAddress(m_local, 7071));           // 绑定本机的ip和port
        m_socket->Listen ();

        //NS_LOG_INFO("m_socket: " << m_socket);
        // m_socket->ShutdownSend ();
        // m_socket->Connect (InetSocketAddress(m_peersAddresses[0], 7071));
    }
    m_socket->SetRecvCallback (MakeCallback (&PaxosNode::HandleRead, this));
    m_socket->SetAllowBroadcast (true);


    // 节点0向节点1请求票
    if (m_id == 0) {
        Simulator::Schedule (Seconds(0), &PaxosNode::requireTicket, this);
    }
}


void 
PaxosNode::StopApplication ()             // Called at time specified by Stop
{
    
}

void 
PaxosNode::HandleRead (Ptr<Socket> socket)
{   
    NS_LOG_FUNCTION (this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom (from)))
    {
        if (packet->GetSize () == 0)
        {   //EOF
            break;
        }
        if (InetSocketAddress::IsMatchingType (from))
        {
            std::string msg = getPacketContent(packet, from);

            //NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s, Node " << GetNode ()->GetId () << " received " << packet->GetSize () << " bytes from " <<
                //InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                //InetSocketAddress::ConvertFrom (from).GetPort ());

            // 打印接收到的结果
            //NS_LOG_INFO("Node " << GetNode ()->GetId () << " Total Received Data: " << msg);

            uint8_t data[4];
            switch (charToInt(msg[0]))
            {
                case REQUEST_TICKET:            // ['type', 'ticket']
                {   
                    data[0] = intToChar(RESPONSE_TICKET);
                    int t = charToInt(msg[1]);
                    if (t > t_max) {
                        t_max = t;              
                        data[1] = intToChar(SUCCESS);
                        data[2] = command;              // ['type', 'success', 'command']
                        
                        NS_LOG_INFO ("### Server receive Ticket ###");
                        NS_LOG_INFO("At time " << Simulator::Now ().GetSeconds () << "s");
                        NS_LOG_INFO("client ticket: " << t << ", server max ticket: "<< t_max << "\n");
                    } 
                    else
                    {
                        data[1] = intToChar(FAILED);     // ['type', 'fail']
                    }
                    Send(data);                          
                    break;
                }
                case REQUEST_PROPOSE:                   // ['type', 'ticket', 'command']
                {
                    data[0] = intToChar(RESPONSE_PROPOSE);
                    int t = charToInt(msg[1]);
                    //NS_LOG_INFO("t: " << t << " server tmax: " << t_max);
                    if (t == t_max) {                 // 请求中的票等于当前最大票
                        
                        char c = msg[2];              
                        command = c;                  // 接受提案
                        t_store = t;                  // 更新存储命令的票
                        data[1] = intToChar(SUCCESS);
                        NS_LOG_INFO ("### Server receive Proposal ###");
                        NS_LOG_INFO("At time " << Simulator::Now ().GetSeconds () << "s");
                        NS_LOG_INFO("client c: " << command <<"\n");
                    } else {
                        data[1] = intToChar(FAILED);
                    }
                    Send(data);

                    break;
                }
                case REQUEST_COMMIT:
                {
                    data[0] = intToChar(RESPONSE_COMMIT);

                    int t = charToInt(msg[1]);
                    int c = msg[2];

                    if (t == t_store && c == command) {     
                        // 执行命令
                        if (isCommit == 0) {
                            NS_LOG_INFO ("### Server commit ###");
                            NS_LOG_INFO("At time " << Simulator::Now ().GetSeconds () << "s");
                            NS_LOG_INFO("Server commit success: " << command << " t_store: " << t_store << "\n");
                        
                        }
                        isCommit = 1;           // 提交成功，不接收该
                        data[1] = intToChar(SUCCESS);
                    } 
                    else
                    {
                         data[1] = intToChar(FAILED);
                    }
                    Send(data);
                    break;
                }
                case RESPONSE_TICKET:
                {
                    int state = charToInt(msg[1]);
                    if (state == SUCCESS) {
                        data[0] = intToChar(REQUEST_PROPOSE);
                        if (msg[2] != 'e') {           // 空的提案
                            proposal = msg[2];         // 后面应该改为超过半数接收的提案
                        } 
                        data[1] = intToChar(ticket);
                        data[2] = proposal;        
                        Send(data);
                        NS_LOG_INFO ("### Client acquire ticket ###");
                        NS_LOG_INFO("At time " << Simulator::Now ().GetSeconds () << "s");
                        NS_LOG_INFO("command: " << msg[2] << "\n");
                    }
                    else {
                        requireTicket();
                    }
                    break;
                }
                case RESPONSE_PROPOSE:
                {
                    int state = charToInt(msg[1]);
                    if (state == SUCCESS) {
                        data[0] = intToChar(REQUEST_COMMIT);
                        data[1] = intToChar(ticket);
                        data[2] = proposal;        
                        Send(data);
                        NS_LOG_INFO ("### Client get proposal response ###");
                        NS_LOG_INFO("At time " << Simulator::Now ().GetSeconds () << "s");
                        NS_LOG_INFO("ACCEPT\n");
                    }
                    else {
                        requireTicket();
                    }
                    break;
                }
                case RESPONSE_COMMIT:
                {
                    int state = charToInt(msg[1]);
                    if (state == SUCCESS) {
                        
                        NS_LOG_INFO ("### Client get commit response ###");
                        NS_LOG_INFO("At time " << Simulator::Now ().GetSeconds () << "s");
                        NS_LOG_INFO("CLIENT COMMIT SUCCESS\n");
                        //NS_LOG_INFO("client " << m_id << " commit succcess");
                    }
                    else {
                        requireTicket();
                    }
                    break;
                }

                // 客户端请求
                case CLIENT_PROPOSE:
                {
                    requireTicket();
                    break;
                }

                default:
                {
                    NS_LOG_INFO("wrong msg");
                    break;
                }
            }
        }
        socket->GetSockName (localAddress);
    }
}


std::string 
PaxosNode::getPacketContent(Ptr<Packet> packet, Address from) 
{
    char *packetInfo = new char[packet->GetSize () + 1];
    std::ostringstream totalStream;
    packet->CopyData (reinterpret_cast<uint8_t*>(packetInfo), packet->GetSize ());
    packetInfo[packet->GetSize ()] = '\0'; // ensure that it is null terminated to avoid bugs
    /**
     * Add the buffered data to complete the packet
     */
    totalStream << m_bufferedData[from] << packetInfo; 
    std::string totalReceivedData(totalStream.str());

    return totalReceivedData;
}  


void 
PaxosNode::Send (uint8_t data[])
{   
    Ptr<Packet> p;
    //uint8_t data[] = "hello";

    p = Create<Packet> (data, 3);
    //NS_LOG_INFO("packet: " << p);
    if (!m_socketClient) {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        m_socketClient = Socket::CreateSocket (GetNode (), tid);
        //NS_LOG_INFO("m_socketClient: " << m_socketClient);
        m_socketClient->Connect (InetSocketAddress(m_peersAddresses[0], 7071));
        
    }
    m_socketClient->Send(p);
   // NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s, node"<< GetNode ()->GetId () << " send packet ");
}

// 第一步 请求票
void 
PaxosNode::requireTicket(void) {
    uint8_t data[3];
    ticket += 1;
    data[0] = intToChar(REQUEST_TICKET);
    data[1] = intToChar(ticket);
    //NS_LOG_INFO("require_data: "<< data);
    Send(data);

    NS_LOG_INFO ("### Client request ticket ###");
    NS_LOG_INFO("At time " << Simulator::Now ().GetSeconds () << "s");
    NS_LOG_INFO("client ticket: " << ticket <<  "\n");
}


} // namespace ns3