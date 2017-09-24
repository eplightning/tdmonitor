#include <tdmonitor/cluster.h>

#include <tdmonitor/types.h>
#include <tdmonitor/packets/core.h>

TDM_NAMESPACE

Cluster::Cluster(const Vector<String> &nodes, u32 nodeId, const String &listen, SharedPtr<DataMarshaller> &marshaller) :
    m_nodeAddresses(nodes), m_ourNodeId(nodeId), m_marshaller(marshaller), m_evq(new EventQueue), m_tcp(new TcpManager)
{
    initCluster(listen);
}

Cluster::Cluster(const Vector<String> &nodes, u32 nodeId, const String &listen) :
    m_nodeAddresses(nodes), m_ourNodeId(nodeId), m_marshaller(new DataMarshaller), m_evq(new EventQueue), m_tcp(new TcpManager)
{
    initCluster(listen);
}

Cluster::~Cluster()
{
    m_tcp->stopLoop();
    m_evq->stop();

    if (m_tcpThread.joinable())
        m_tcpThread.join();

    if (m_eventLoop)
        m_eventLoop->waitForThreads();
}

void Cluster::sendLockRequest(const String &monitor)
{
    EventRequest *req = new EventRequest(EventRequest::RequestId::Lock, monitor);
    m_evq->append(req);
}

void Cluster::sendUnlockRequest(const String &monitor)
{
    EventRequest *req = new EventRequest(EventRequest::RequestId::Unlock, monitor);
    m_evq->append(req);
}

void Cluster::sendSignalRequest(const String &monitor, const String &condVar)
{
    EventRequest *req = new EventRequest(EventRequest::RequestId::Signal, monitor, condVar);
    m_evq->append(req);
}

void Cluster::sendCreateRequest(const SharedPtr<Token> &token)
{
    EventNewMonitor *req = new EventNewMonitor(token);
    m_evq->append(req);
}

void Cluster::initCluster(const String &listen)
{
    initTcp(listen);
    initLoop();
}

void Cluster::initTcp(const String &listen)
{
    ListenTcpPool *pool = new ListenTcpPool(
        BIND_TCP_STATE(this, &Cluster::tcpState),
        BIND_TCP_NEW(this, &Cluster::tcpNew),
        BIND_TCP_RECV(this, &Cluster::tcpReceive)
    );

    pool->listenSockets()->reserve(1);

    ConnectionProtocol proto;
    int sock = SocketUtils::createListenSocket(listen, proto);

    if (sock >= 0) {
        pool->appendListenSocket(proto, sock);
    } else {
        // błąd
    }

    m_tcp->createPool("cluster", pool);

    uint idx = 0;

    while (idx < m_ourNodeId) {
        m_tcp->connect(m_nodeAddresses[idx], "cluster", idx);
        ++idx;
    }

    m_tcpThread = std::thread([this] {
        m_tcp->runLoop(5);
        // błąd
    });
}

void Cluster::initLoop()
{
    m_clusterLoop.reset(new ClusterLoop(m_nodeAddresses.size(), m_ourNodeId, m_tcp.get()));

    m_eventLoop.reset(new EventLoop(1, m_evq.get(), [this](Event *ev) {
        return m_clusterLoop->handle(ev);
    }));

    m_eventLoop->startThreads();
}

bool Cluster::tcpNew(SharedPtr<Client> &client)
{
    m_clients[client->id()] = client;

    return true;
}

void Cluster::tcpState(Client *client, TcpClientState state, int error)
{
    UNUSED(error);

    if (state == TCSConnected) {
        m_clients[client->id()] = m_tcp->client(client->id());
    } else if (state == TCSDisconnected) {
        auto nodeIt = m_clientNodeMapping.find(client->id());

        if (nodeIt != m_clientNodeMapping.end()) {
            Event *dc = new EventDisconnected(nodeIt->second);
            m_evq->append(dc);
        }
    } else if (state == TCSFailedToEstablish) {
        // nie udało się połączyć
    }
}

void Cluster::tcpReceive(Client *client, PacketHeader header, const Vector<char> &data)
{
    auto clientIt = m_clients.find(client->id());
    if (clientIt == m_clients.end()) {
        // ?
        return;
    }
    SharedPtr<Client> clientPtr = clientIt->second;

    auto nodeIt = m_clientNodeMapping.find(client->id());

    Packet *packet;

    if ((packet = Packet::factory(header, data)) == nullptr) {
        m_tcp->disconnect(clientPtr, true);
        return;
    }

    if (packet->packetType() == Packet::Type::EdgeHandshake) {
        if (nodeIt != m_clientNodeMapping.end()) {
            // duplikat
            return;
        }

        u32 nodeId = (static_cast<CorePackets::EdgeHandshake*>(packet))->id();

        m_clientNodeMapping[client->id()] = nodeId;

        Event *newNode = new EventConnected(clientPtr, nodeId);
        m_evq->append(newNode);
        return;
    }

    if (nodeIt == m_clientNodeMapping.end()) {
        // node się nie przedstawił
        m_tcp->disconnect(clientPtr, true);
        return;
    }

    Event *packetEvent = new EventPacket(packet, nodeIt->second);
    m_evq->append(packetEvent);
}

END_NAMESPACE
