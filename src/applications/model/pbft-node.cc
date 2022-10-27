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

// 全局变量 是所有节点间共用的
int tx_size;
int tx_speed;                  // 交易生成的速率，单位为op/s
int n;
int v;
int val;
float timeout;                   // 发送区块的间隔时间
int n_round;

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

// 信息接收延迟 3 - 6 ms
float 
getRandomDelay() {
  return ((rand() % 3) * 1.0 + 3) / 1000;
}

void
printVector(std::vector<int> vec) {
    for (int i = 0; i < vec.size(); i++) {
        NS_LOG_INFO(vec[i] + " ");
    }
}

// 生成交易 每个交易 size KB
static uint8_t * generateTX (int num)
{
  int size = num * tx_size;
  uint8_t *data = (uint8_t *)std::malloc (size);
  int i;
  for (i = 0; i < size; i++) {
    data[i] = '1';
  }
  data[i] = '\0';
  // NS_LOG_INFO("初始化成功: " << data);
  data[0] = intToChar(PRE_PREPARE);
  data[1] = intToChar(v);
  data[2] = intToChar(n);
  data[3] = intToChar(n);

  return data;
}

void 
PbftNode::StartApplication ()            
{
    // 初始化全局变量
    v = 1;              // 视图数
    n = 0;              // 当前视图的交易序号
    leader = 0;
    tx_size = 1000;      // 1 KB
    tx_speed = 1000;    // 1000 tx/s
    timeout = 0.05;      // 50 ms

    block_num = 0;
    // 共识轮数
    n_round = 0;                      

    // 交易要更新的值
    val = intToChar(m_id);

    // 需要修改的value值
    // int value = 3; 

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
    // NS_LOG_INFO("Node " << GetNode ()->GetId () << "网络建立");
    // 如果是leader节点， 广播预准备消息
    // if (m_id == leader) {
        // is_leader = 1;
        // uint8_t data[4];
        // data[0] = intToChar(PRE_PREPARE);
        // data[1] = intToChar(v);
        // data[2] = intToChar(n);
        // data[3] = intToChar(value);
        // // leader 广播预准备消息

        // 发送区块
        // SendBlock(data, num);
    Simulator::Schedule(Seconds(timeout), &PbftNode::SendBlock, this);
        // n++;
    // }
}

void 
PbftNode::StopApplication ()
{
    // printVector(values);
}

void 
PbftNode::HandleRead (Ptr<Socket> socket)
{   
    Ptr<Packet> packet;
    Address from;
    Address localAddress;

    while ((packet = socket->RecvFrom (from)))
    {
        // socket->SendTo(packet, 0, from);
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
            // NS_LOG_INFO("Node " << GetNode ()->GetId () << " 接收到消息: " << msg);
            uint8_t data[4];
            switch (charToInt(msg[0]))
            {
                case PRE_PREPARE:           
                {   
                    // 收到预准备消息
                    data[0] = intToChar(PREPARE);
                    data[1] = msg[1];           // v
                    data[2] = msg[2];           // n
                    data[3] = msg[3];           // value
                    // 序号值
                    int num = charToInt(msg[2]);

                    // 存储交易中的value值
                    tx[num].val = charToInt(msg[3]);
                    // if (num > n) {
                    //     n = num;
                    // }
                    // 广播准备消息
                    Send(data);
                    break;
                }
                case PREPARE:           
                {   
                    // 收到准备消息
                    data[0] = intToChar(PREPARE_RES);
                    data[1] = msg[1];      // v
                    data[2] = msg[2];      // n
                    data[3] = intToChar(SUCCESS);     
                    // 回复准备消息响应
                    Send(data, from);
                    break;
                }
                case PREPARE_RES:           
                {   
                    // 收到准备消息响应
                    int index = charToInt(msg[2]);
                    if (charToInt(msg[3]) == 0) {
                        tx[index].prepare_vote++;
                    }
                    // if 超过半数SUCCESS, 则广播COMMIT
                    if (tx[index].prepare_vote >= 2 * N / 3) {
                        data[0] = intToChar(COMMIT);
                        data[1] = msg[1];       // v
                        data[2] = msg[2];       // n
                        Send(data);
                        // NS_LOG_INFO("node"<< m_id << "获得的准备投票: " << tx[index].prepare_vote);
                        tx[index].prepare_vote = 0;
                    }
                    break;
                }
                case COMMIT:           
                {   
                    // 收到提交消息
                    int index = charToInt(msg[2]);
                    // NS_LOG_INFO("node"<< m_id << "收到commit " << tx[index].val);
                    tx[index].commit_vote++;
                    // 超过半数则 回复提交消息响应
                    if (tx[index].commit_vote > 2 * N / 3) {
                        data[0] = intToChar(COMMIT_RES);
                        data[1] = intToChar(v);
                        data[2] = intToChar(n);
                        data[3] = SUCCESS;       // n
                        // Send(data);
                        tx[index].commit_vote = 0;

                        // 记录交易到队列中
                        values.push_back(tx[index].val);
                        // NS_LOG_INFO("node"<< m_id << "加入交易 " << tx[index].val);
                        NS_LOG_INFO("node "<< m_id << " in view " << v << " finish " << block_num << "th times submit, at time " << Simulator::Now ().GetSeconds () << "s, value is " << values[block_num] << "\n");
                        block_num++;
                        // n = n + 1;
                    }
                    // Send(data, from);
                    break;
                }
                // case COMMIT_RES:
                // {
                //     // 如果超过半数则表示提交成功，reply客户端成功
                // }

                case VIEW_CHANGE:
                {
                    int vt = charToInt(msg[1]);
                    int lt = charToInt(msg[2]);
                    v = vt;
                    leader = lt;
                    if (m_id == leader) {
                        NS_LOG_INFO("view-change完成, 当前主节点为 " << leader << "视图为 " << v);
                    }
                }

                default:
                {
                    NS_LOG_INFO("Wrong msg");
                    break;
                }
            }
        }
        socket->GetSockName (localAddress);
    }
}

void
PbftNode::viewChange (void)
{
    uint8_t data[4];
    leader = (leader + 1) % N;
    v += 1;
    data[0] = intToChar(VIEW_CHANGE);
    data[1] = intToChar(v);
    data[2] = intToChar(leader);
    Send(data);
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
    p = Create<Packet> (data, 4);
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
  p = Create<Packet> (data, 4);
  
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
PbftNode::SendBlock (void)
{   
  // NS_LOG_INFO("广播区块: time: " << Simulator::Now ().GetSeconds () << " s");
  Ptr<Packet> p;
  // TODO: 广播的内容包 p
  int num = tx_speed / (1000 / (timeout * 1000)); 
  uint8_t * data = generateTX(num);
  int size = tx_size * num;
  p = Create<Packet> (data, size);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();

  if (m_id == leader) {
    NS_LOG_INFO("Leader node"<< m_id << "start broadcast, at time " <<Simulator::Now ().GetSeconds () << "s");

    while(iter != m_peersAddresses.end()) {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        
        Ptr<Socket> socketClient = m_peersSockets[*iter];
        double delay = getRandomDelay();
        Simulator::Schedule(Seconds(delay), SendPacket, socketClient, p);
        iter++;
    }
    n_round++;
    n++;

    // view_change， 概率为1/10
    if (rand() % 10 == 5) {
        viewChange();
    }
  }

  blockEvent = Simulator::Schedule (Seconds(timeout), &PbftNode::SendBlock, this);
  if (n_round == 40) {
    NS_LOG_INFO(" 已经发送了第 "<< n_round << "个区块 at time: " << Simulator::Now ().GetSeconds () << "s");
    Simulator::Cancel(blockEvent);
  }
}

// // 向所有邻居节点广播交易
// void 
// RaftNode::SendTX (uint8_t data[], int num)
// {    
//   NS_LOG_INFO("广播区块: " << round << ", time: " << Simulator::Now ().GetSeconds () << " s");
//   Ptr<Packet> p;
//   int size = tx_size * num;
//   p = Create<Packet> (data, size);
  
//   TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");


//   std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();

//   while(iter != m_peersAddresses.end()) {
//     TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    
//     Ptr<Socket> socketClient = m_peersSockets[*iter];
//     double delay = getRandomDelay();
//     Simulator::Schedule(Seconds(delay), SendPacket, socketClient, p);
//     iter++;
//   }
//   round++;
//   if (round == 50) {
//     NS_LOG_INFO("node" << m_id << " 已经发送了 "<< round << "个区块 at time: " << Simulator::Now ().GetSeconds () << "s");
//     // Simulator::Cancel (m_nextHeartbeat);
//     add_change_value = 0;
//   }
// }
} 