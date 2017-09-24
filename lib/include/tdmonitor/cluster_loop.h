#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/event.h>
#include <tdmonitor/tcp_manager.h>
#include <tdmonitor/packets/core.h>

TDM_NAMESPACE

class ClusterLoop {
public:
    ClusterLoop(int nodes, u32 m_ourNodeId, TcpManager *tcp);

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

    bool m_started;
    int m_nodeCount;
    u32 m_ourNodeId;
    TcpManager *m_tcp;
    HashMap<u32, SharedPtr<Client>> m_nodes;
    HashMap<String, TokenPrivateData> m_tokenPrivate;
    HashMap<String, SharedPtr<Token>> m_tokens;
};

END_NAMESPACE
