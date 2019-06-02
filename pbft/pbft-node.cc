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
#include "pbft-node.h"
#include "stdlib.h"
#include "ns3/ipv4.h"
#include <ctime>
#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PbftNode");

NS_OBJECT_ENSURE_REGISTERED (PbftNode);

TypeId
PbftNode::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::PbftNode")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<PbftNode> ()
    ;

    return tid;
}

PbftNode::PbftNode(void) {

}

PbftNode::~PbftNode(void) {
    NS_LOG_FUNCTION (this);
}

static char intToChar(int a) {
    return a + '0';
}

static int charToInt(char a) {
    return a - '0';
}

// 信息接收延迟 0 - 3 ms
float 
getRandomDelay() {
  return (rand() % 3) * 1.0 / 1000;
}


void 
PbftNode::StartApplication ()            
{
    // 初始化变量

    // 初始化socket
    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket (GetNode (), tid);

        // 注意 相当于监听所有网卡ip
        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 7071);
        m_socket->Bind (local);           // 绑定本机的ip和port
        m_socket->Listen ();
    }
    m_socket->SetRecvCallback (MakeCallback (&PbftNode::HandleRead, this));
    m_socket->SetAllowBroadcast (true);

    std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();
    // 与所有节点建立连接
    while(iter != m_peersAddresses.end()) {
        // NS_LOG_INFO("node"<< m_id << *iter << "\n");
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        Ptr<Socket> socketClient = Socket::CreateSocket (GetNode (), tid);
        socketClient->Connect (InetSocketAddress(*iter, 7071));
        m_peersSockets[*iter] = socketClient;
        iter++;
    }
}

void 
PbftNode::StopApplication ()
{
}

void 
PbftNode::HandleRead (Ptr<Socket> socket)
{   
    Ptr<Packet> packet;
    Address from;
    Address localAddress;

    while ((packet = socket->RecvFrom (from)))
    {
        socket->SendTo(packet, 0, from);
        if (packet->GetSize () == 0)
        {   //EOF
            break;
        }
        if (InetSocketAddress::IsMatchingType (from))
        {
            std::string msg = getPacketContent(packet, from);

            // NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s, Node " << GetNode ()->GetId () << " received " << packet->GetSize () << " bytes, msg[0]: "<< msg[0]);
            //     InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
            //     InetSocketAddress::ConvertFrom (from).GetPort ());

            // // 打印接收到的结果
            // NS_LOG_INFO("Node " << GetNode ()->GetId () << " Total Received Data: " << msg);
            uint8_t data[4];
            switch (charToInt(msg[0]))
            {
                switch (0)
                {
                  // 消息处理逻辑
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
PbftNode::getPacketContent(Ptr<Packet> packet, Address from) 
{ 
    // NS_LOG_INFO("包大小" << packet->GetSize ());
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
SendPacket(Ptr<Socket> socketClient,Ptr<Packet> p) {
    socketClient->Send(p);
}

// 向某个指定地址的节点发送消息
void 
PbftNode::Send(uint8_t data[], Address from)
{
     Ptr<Packet> p;
    p = Create<Packet> (data, 3);
    //NS_LOG_INFO("packet: " << p);
    
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  

    Ptr<Socket> socketClient;
    if (!m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4 ()]) {
        socketClient = Socket::CreateSocket (GetNode (), tid);
        socketClient->Connect (InetSocketAddress(InetSocketAddress::ConvertFrom(from).GetIpv4 (), 7071));
        m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4 ()] = socketClient;
    }
    socketClient = m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4 ()];
    Simulator::Schedule(Seconds(getRandomDelay()), SendPacket, socketClient, p);
}

// 向所有邻居节点广播消息
void 
PbftNode::Send (uint8_t data[])
{   
  // NS_LOG_INFO("广播消息");
  Ptr<Packet> p;
  p = Create<Packet> (data, 3);
  
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();

  while(iter != m_peersAddresses.end()) {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    
    Ptr<Socket> socketClient = m_peersSockets[*iter];
    double delay = getRandomDelay();
    Simulator::Schedule(Seconds(delay), SendPacket, socketClient, p);
    iter++;
  }
}

// 向所有邻居节点广播区块
void 
PbftNode::SendBlock (uint8_t data[], int num)
{   
  NS_LOG_INFO("广播区块: time: " << Simulator::Now ().GetSeconds () << " s");
  Ptr<Packet> p;
  // TODO: 广播的内容包 p
  // p = Create<Packet> (data, size);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");


  std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();

  while(iter != m_peersAddresses.end()) {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    
    Ptr<Socket> socketClient = m_peersSockets[*iter];
    double delay = getRandomDelay();
    Simulator::Schedule(Seconds(delay), SendPacket, socketClient, p);
    iter++;
  }
}
} 