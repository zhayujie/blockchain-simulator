#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#include <algorithm>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/boolean.h"
#include <map>

namespace ns3 {

class Address;
class Socket;
class Packet;

class RaftNode : public Application 
{
  public:
    static TypeId GetTypeId (void);

    void SetPeersAddresses (const std::vector<Ipv4Address> &peers);         // 设置所有邻节点的地址

    RaftNode (void);

    virtual ~RaftNode (void);

    uint32_t        m_id;                               // node id
    Ptr<Socket>     m_socket;                           // 监听的socket
    Ptr<Socket>     m_socketClient;                     // 客户端socket
    std::map<Ipv4Address, Ptr<Socket>>      m_peersSockets;            // 邻节点的socket列表
    std::map<Address, std::string>          m_bufferedData;            // map holding the buffered data from previous handleRead events
    
    Address         m_local;                            // 本节点地址
    std::vector<Ipv4Address>  m_peersAddresses;         // 邻节点列表

    int             N;                                  // 总节点数
    int             is_leader;                          // 自己是否是leader
    int             has_voted;                          // 是否已经投票
    int             m_value;                            // 通过共识修改的变量
    char            proposal;                           // 要发送的命令
    int             vote_success;                       // 投票响应成功数
    int             vote_failed;                        // 投票响应失败数
    float           heartbeat_timeout;                  // 心跳超时时间
    int             add_change_value;                   // 是否在心跳中加入提案
    EventId         m_nextElection;                     // 下一次成为candidate广播投票的事件
    EventId         m_nextHeartbeat;                    // 下一次发送心跳的事件
    // int             tx_speed;                           // 交易生成的速率，单位为op/s
    int             blockNum;                           // 区块个数
    int             round;                              // 共识轮数
    uint8_t *       tx;                                 // 交易
    // int             tx_size;                            // 一个交易的大小

    // 继承 Application 类必须实现的虚函数
    virtual void StartApplication (void);    
    virtual void StopApplication (void); 

    // 处理消息
    void HandleRead (Ptr<Socket> socket);

    void Send (uint8_t data[]);

    std::string getPacketContent(Ptr<Packet> packet, Address from); 
    // 发送消息
    //void SendMessage(enum Message responseMessage, std::string msg, Ptr<Socket> outgoingSocket);

    void sendVote(void);
    
    void sendHeartBeat(void);

    void Send(uint8_t data[], Address from);

    void SendTX(uint8_t data[], int num);

    // 设置发送提案
    void setProposal(void);
};

enum Message
{
    CLIENT_REQ,        // 0       客户端请求
    CLIENT_RES,        // 1       给客户端的响应
    VOTE_REQ,          // 2       请求投票
    VOTE_RES,          // 3       对请求投票的响应
    HEARTBEAT,         // 4       心跳
    HEARTBEAT_RES,     // 5       心跳回复
};

enum HeartBeatType
{
    HEART_BEAT,       // 0        普通心跳
    PROPOSAL,         // 1        包含提案的心跳
};

enum State
{
    SUCCESS,                   // 0      成功
    FAILED,                    // 1      失败
};

}
#endif