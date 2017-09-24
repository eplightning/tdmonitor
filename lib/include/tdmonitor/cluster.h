#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/marshaller.h>
#include <tdmonitor/event.h>
#include <tdmonitor/cluster_loop.h>

#include <thread>

TDM_NAMESPACE

class Cluster {
public:
    Cluster(const Vector<String> &nodes, u32 nodeId, const String &listen, SharedPtr<DataMarshaller> &marshaller);
    Cluster(const Vector<String> &nodes, u32 nodeId, const String &listen);
    ~Cluster();

    void sendLockRequest(const String &monitor);
    void sendUnlockRequest(const String &monitor);
    void sendSignalRequest(const String &monitor, const String &condVar);
    void sendCreateRequest(const SharedPtr<Token> &token);

private:
    void initCluster(const String &listen);
    void initTcp(const String &listen);
    void initLoop();

    bool tcpNew(SharedPtr<Client> &client);
    void tcpState(Client *client, TcpClientState state, int error);
    void tcpReceive(Client *client, PacketHeader header, const Vector<char> &data);

    Vector<String> m_nodeAddresses;
    u32 m_ourNodeId;
    SharedPtr<DataMarshaller> m_marshaller;
    UniquePtr<EventQueue> m_evq;
    UniquePtr<TcpManager> m_tcp;
    UniquePtr<EventLoop> m_eventLoop;
    UniquePtr<ClusterLoop> m_clusterLoop;
    std::thread m_tcpThread;

    HashMap<u32, SharedPtr<Client>> m_clients;
    HashMap<u32, u32> m_clientNodeMapping;
};

END_NAMESPACE
