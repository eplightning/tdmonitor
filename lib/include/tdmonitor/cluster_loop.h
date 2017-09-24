#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/event.h>
#include <tdmonitor/token.h>
#include <tdmonitor/tcp_manager.h>
#include <tdmonitor/packets/core.h>
#include <set>

TDM_NAMESPACE

class ClusterLoop {
public:
    ClusterLoop(int nodes, u32 m_ourNodeId, TcpManager *tcp, DataMarshaller *marshaller);

    bool handle(Event *event);

private:
    void handlePacket(EventPacket *event);
    void handleConnect(EventConnected *event);
    void handleDisconnect(EventDisconnected *event);
    void handleRequest(EventRequest *event);
    void handleNew(EventNewMonitor *event);

    void packetClusterHandshake(CorePackets::ClusterHandshake *packet, u32 nodeId);
    void packetGiveToken(CorePackets::GiveToken *packet, u32 nodeId);
    void packetRequestToken(CorePackets::RequestToken *packet, u32 nodeId);
    void packetSignal(CorePackets::Signal *packet, u32 nodeId);

    void requestLock(const String &monitor);
    void requestUnlock(const String &monitor);
    void requestSignal(const String &monitor, const String &condVar);

    void broadcast(const Packet *packet);
    SystemToken *systemToken();
    void requestToken(const String &id);

    void grant(const String &id);
    void grant(const String &id, TokenPrivateData *priv);
    void grant(const String &id, TokenPrivateData *priv, SharedPtr<Token> &pub);
    void grant(const String &id, SharedPtr<Token> &pub);
    void sendToken(const String &id, u32 node, TokenPrivateData *priv);

    bool m_started;
    bool m_startedCluster;
    int m_clusterHandshakes;
    int m_nodeCount;
    u32 m_ourNodeId;
    TcpManager *m_tcp;
    DataMarshaller *m_marshaller;
    HashMap<u32, SharedPtr<Client>> m_nodes;
    HashMap<String, UniquePtr<TokenPrivateData>> m_tokenPrivate;
    HashMap<String, SharedPtr<Token>> m_tokens;
    std::set<String> m_pendingRequests;
    std::set<String> m_pendingGrants;
};

END_NAMESPACE
