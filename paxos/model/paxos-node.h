#ifndef PAXOS_NODE_H
#define PAXOS_NODE_H

#include <algorithm>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/boolean.h"

namespace ns3 {

class Address;
class Socket;
class Packet;


class PaxosNode : public Application 
{
    public:
        static TypeId GetTypeId (void);

        void SetPeersAddresses (const std::vector<Ipv4Address> &peers);         // 设置所有邻节点的地址

        PaxosNode (void);

        virtual ~PaxosNode (void);

        uint32_t        m_id;                               // node id
        Ptr<Socket>     m_socket;                           // 监听的socket
        Ptr<Socket>     m_socketClient;                     // 客户端socket
        std::map<Ipv4Address, Ptr<Socket>>      m_peersSockets;            // 邻节点的socket列表
        std::map<Address, std::string>          m_bufferedData;            // map holding the buffered data from previous handleRead events


        Address     m_local;                                // 本节点地址
        std::vector<Ipv4Address>  m_peersAddresses;         // 邻节点列表

        int             t_max;                // 当前已经发布的最大票号
        char            command;              // 当前存储的命令
        int             t_store;              // 存储当前命令的票号               
        int             ticket;               // 作为客户端时当前尝试的票号        （如果并发，需改为map）



        int             isCommit;                           // 是否成功提交
        char            proposal;                           // 要发送的命令
        int             round;                              
        int             vote_success;                       // 响应成功数
        int             vote_failed;                        // 响应失败数
        int             N;                                  // 总节点数

        // 继承 Application 类必须实现的虚函数
        virtual void StartApplication (void);    
        virtual void StopApplication (void); 

        // 处理消息
        void HandleRead (Ptr<Socket> socket);

        void Send (uint8_t data[]);

        std::string getPacketContent(Ptr<Packet> packet, Address from); 
        // 发送消息
        //void SendMessage(enum Message responseMessage, std::string msg, Ptr<Socket> outgoingSocket);

        void requireTicket(void);

        void Send(uint8_t data[], Address from);
};

enum Message
{
    REQUEST_TICKET,           // 0       请求票
    REQUEST_PROPOSE,          // 1       请求提案
    REQUEST_COMMIT,           // 2       请求提交
    RESPONSE_TICKET,          // 3       对请求票的响应
    RESPONSE_PROPOSE,         // 4       对发出提案的响应
    RESPONSE_COMMIT,          // 5       对发起提交的响应
    CLIENT_PROPOSE,           // 6       客户端发起的提案
};

enum State
{
    SUCCESS,                   // 0      成功
    FAILED,                    // 1      失败
};

}
#endif