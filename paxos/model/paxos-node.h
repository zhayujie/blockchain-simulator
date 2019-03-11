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

    protected:
        Ptr<Socket>     m_socket;                           // 监听的socket
        Address         m_local;                            // 本节点地址
        std::vector<Ipv4Address> m_peersAddresses;          // 邻节点列表

        int             t_max;                              // 当前已经发布的最大票号
        std::string     command;                            // 当前存储的命令
        int             t_store;                            // 存储当前命令的票号               

        int             ticket;                             // 作为客户端时当前尝试的票号（如果并发，需改为map）

        // 继承 Application 类必须实现的虚函数
        virtual void StartApplication (void);    
        virtual void StopApplication (void); 

        // 处理消息
        void HandleRead (Ptr<Socket> socket, const Address& from);

        // 发送消息
        //void SendMessage(enum Message responseMessage, std::string msg, Ptr<Socket> outgoingSocket);

};
/*
enum Message
{
    REQUEST_TICKET,           // 0
    REQUEST_PROPOSE,          // 1
    REQUEST_COMMIT,           // 2
    RESPONSE_TICKET,          // 3
    RESPONSE_PROPOSE,         // 4  
    RESPONSE_COMMIT,          // 5  
};
*/
}
#endif