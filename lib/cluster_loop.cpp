#include <tdmonitor/cluster_loop.h>

#include <tdmonitor/types.h>

TDM_NAMESPACE

ClusterLoop::ClusterLoop(int nodes, u32 ourNodeId, TcpManager *tcp) :
    m_nodeCount(nodes), m_ourNodeId(ourNodeId), m_tcp(tcp)
{

}

bool ClusterLoop::handle(Event *event)
{
    switch (event->type()) {
    case Event::Type::Connected:
        handleConnect(static_cast<EventConnected*>(event));
        break;

    case Event::Type::Disconnected:
        handleDisconnect(static_cast<EventDisconnected*>(event));
        break;

    case Event::Type::NewMonitor:
        handleNew(static_cast<EventNewMonitor*>(event));
        break;

    case Event::Type::Packet:
        handlePacket(static_cast<EventPacket*>(event));
        break;

    case Event::Type::Request:
        handleRequest(static_cast<EventRequest*>(event));
        break;
    }

    return true;
}

void ClusterLoop::handlePacket(EventPacket *event)
{
    switch (event->packet()->packetType()) {
    case Packet::Type::ClusterHandshake:
        packetClusterHandshake(static_cast<CorePackets::ClusterHandshake*>(event->packet()), event->nodeId());
        break;

    case Packet::Type::GiveToken:
        packetGiveToken(static_cast<CorePackets::GiveToken*>(event->packet()), event->nodeId());
        break;

    case Packet::Type::RequestToken:
        packetRequestToken(static_cast<CorePackets::RequestToken*>(event->packet()), event->nodeId());
        break;

    case Packet::Type::Signal:
        packetSignal(static_cast<CorePackets::Signal*>(event->packet()), event->nodeId());
        break;

    default:
        // ?
        break;
    }
}

void ClusterLoop::handleConnect(EventConnected *event)
{
    m_nodes[event->nodeId()] = event->client();

    if (static_cast<int>(m_nodes.size()) >= m_nodeCount - 1 && !m_started) {
        m_started = true;

        CorePackets::ClusterHandshake *handshake = new CorePackets::ClusterHandshake;
        handshake->setId(m_ourNodeId);
        broadcast(handshake);
        delete handshake;


    }
}

void ClusterLoop::handleDisconnect(EventDisconnected *event)
{
    UNUSED(event);
}

void ClusterLoop::handleRequest(EventRequest *event)
{
    switch (event->requestType()) {
    case EventRequest::RequestId::Lock:
        requestLock(event->monitor());
        break;
    case EventRequest::RequestId::Signal:
        requestSignal(event->monitor(), event->condVar());
        break;
    case EventRequest::RequestId::Unlock:
        requestUnlock(event->monitor());
        break;
    }
}

void ClusterLoop::handleNew(EventNewMonitor *event)
{

}

void ClusterLoop::packetClusterHandshake(CorePackets::ClusterHandshake *packet, u32 nodeId)
{

}

void ClusterLoop::packetGiveToken(CorePackets::GiveToken *packet, u32 nodeId)
{

}

void ClusterLoop::packetRequestToken(CorePackets::RequestToken *packet, u32 nodeId)
{

}

void ClusterLoop::packetSignal(CorePackets::Signal *packet, u32 nodeId)
{

}

void ClusterLoop::requestLock(const String &monitor)
{

}

void ClusterLoop::requestUnlock(const String &monitor)
{

}

void ClusterLoop::requestSignal(const String &monitor, const String &condVar)
{

}

void ClusterLoop::broadcast(const Packet *packet)
{
    Vector<SharedPtr<Client>> clients;

    for (auto &x : m_nodes) {
        clients.push_back(x.second);
    }

    m_tcp->sendTo(clients, packet);
}

END_NAMESPACE
