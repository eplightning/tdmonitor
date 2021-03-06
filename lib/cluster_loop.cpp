#include <tdmonitor/cluster_loop.h>

#include <tdmonitor/types.h>

#include <set>

TDM_NAMESPACE

ClusterLoop::ClusterLoop(int nodes, u32 ourNodeId, TcpManager *tcp, DataMarshaller *marshaller) :
    m_started(false), m_startedCluster(false), m_clusterHandshakes(0),
    m_nodeCount(nodes), m_ourNodeId(ourNodeId), m_tcp(tcp), m_marshaller(marshaller)
{
    if (nodes == 1) {
        m_started = true;
        m_startedCluster = true;
        SystemToken *token = new SystemToken(m_nodeCount, m_ourNodeId == 0);
        m_tokenPrivate.emplace(std::piecewise_construct, std::forward_as_tuple("___system_token"), std::forward_as_tuple(token));
    }
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

    delete event;

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
        TDM_LOG("All nodes connected, advancing to cluster handshake");
        m_started = true;

        CorePackets::ClusterHandshake *handshake = new CorePackets::ClusterHandshake;
        handshake->setId(m_ourNodeId);
        broadcast(handshake);
        delete handshake;

        SystemToken *token = new SystemToken(m_nodeCount, m_ourNodeId == 0);
        m_tokenPrivate.emplace(std::piecewise_construct, std::forward_as_tuple("___system_token"), std::forward_as_tuple(token));

        if (m_ourNodeId == 0) {
            for (auto &x : m_tokenPrivate) {
                if (x.first != "___system_token") {
                    token->addMonitor(x.first);
                }
            }
        }
    }
}

void ClusterLoop::handleDisconnect(EventDisconnected *event)
{
    UNUSED(event);

    TDM_LOG("Disconnected node ", event->nodeId());
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
    TDM_LOG("Creating new monitor: ", event->token()->id());

    auto publicIt = m_tokens.find(event->token()->id());

    if (publicIt != m_tokens.end()) {
        // ?
        return;
    }

    m_tokens[event->token()->id()] = event->token();

    auto privateIt = m_tokenPrivate.find(event->token()->id());

    if (privateIt != m_tokenPrivate.end()) {
        // TokenPrivateData *priv = privateIt->second.get();
        // cool
        return;
    }

    if (!m_started) {
        TDM_LOG("EdgeHandshake not finished");

        TokenPrivateData *newPriv = new TokenPrivateData(m_nodeCount, m_ourNodeId == 0, m_ourNodeId == 0);
        m_tokenPrivate.emplace(std::piecewise_construct, std::forward_as_tuple(event->token()->id()), std::forward_as_tuple(newPriv));

        if (m_ourNodeId != 0) {
            requestToken("___system_token");
        }

        return;
    }

    SystemToken *system = systemToken();

    if (system->hasMonitor(event->token()->id())) {
        TDM_LOG("Monitor already exists in the cluster");

        TokenPrivateData *newPriv = new TokenPrivateData(m_nodeCount, true, false);

        m_tokenPrivate.emplace(std::piecewise_construct, std::forward_as_tuple(event->token()->id()), std::forward_as_tuple(newPriv));
    } else if (system->owned()) {
        TDM_LOG("Monitor doesn't exist but we have systemToken");

        system->addMonitor(event->token()->id());

        TokenPrivateData *newPriv = new TokenPrivateData(m_nodeCount, true, true);

        m_tokenPrivate.emplace(std::piecewise_construct, std::forward_as_tuple(event->token()->id()), std::forward_as_tuple(newPriv));
    } else {
        TDM_LOG("Monitor doesn't exist and we have no systemToken");

        TokenPrivateData *newPriv = new TokenPrivateData(m_nodeCount, false, false);

        m_tokenPrivate.emplace(std::piecewise_construct, std::forward_as_tuple(event->token()->id()), std::forward_as_tuple(newPriv));
        requestToken("___system_token");
    }
}

void ClusterLoop::packetClusterHandshake(CorePackets::ClusterHandshake *packet, u32 nodeId)
{
    UNUSED(nodeId);
    UNUSED(packet);
    m_clusterHandshakes++;

    TDM_LOG("Received cluster handshake from node: ", nodeId);

    if (m_clusterHandshakes >= m_nodeCount - 1 && !m_startedCluster) {
        TDM_LOG("ClusterHandshake finished, sending token requests and granting access if possible");

        m_startedCluster = true;

        for (auto &x : m_pendingGrants) {
            auto privateIt = m_tokenPrivate.find(x);

            if (privateIt == m_tokenPrivate.end()) {
                continue;
            }

            TokenPrivateData *priv = privateIt->second.get();

            if (priv->owned()) {
                grant(x, priv);
            }
        }

        for (auto &x : m_pendingRequests) {
            CorePackets::RequestToken *request = new CorePackets::RequestToken;
            request->setTokenId(x);
            request->setSequenceId(m_tokenPrivate[x]->incrementRequestNumber(m_ourNodeId));
            broadcast(request);
            delete request;
        }
    }
}

void ClusterLoop::packetGiveToken(CorePackets::GiveToken *packet, u32 nodeId)
{
    UNUSED(nodeId);

    TDM_LOG("Received token ", packet->tokenId(), " from ", nodeId);

    if (!m_started)
        return;

    auto privateIt = m_tokenPrivate.find(packet->tokenId());

    if (privateIt == m_tokenPrivate.end()) {
        // ???
        return;
    }

    TokenPrivateData *priv = privateIt->second.get();

    if (priv->owned()) {
        // ???
        return;
    }

    m_pendingRequests.erase(packet->tokenId());

    priv->setOwned(true);

    PropertyMap map = m_marshaller->unmarshallMap(packet->data());
    priv->loadProperties(map);

    if (packet->tokenId() == "___system_token") {
        SystemToken *sysToken = static_cast<SystemToken*>(priv);

        for (auto &x : m_tokenPrivate) {
            TokenPrivateData *token2 = x.second.get();

            if (!token2->created()) {
                token2->setCreated(true);

                if (sysToken->hasMonitor(x.first)) {
                    token2->setOwned(false);

                    if (m_pendingGrants.count(x.first) > 0) {
                        requestToken(x.first);
                    }
                } else {
                    token2->setOwned(true);
                    sysToken->addMonitor(x.first);
                    grant(x.first, token2);
                }
            }
        }

        TDM_LOG("systemToken processed, releasing");

        if (sysToken->release(m_ourNodeId)) {
            u32 target = sysToken->queue().front();
            sysToken->queue().pop();
            sendToken("___system_token", target, sysToken);
        }

        return;
    }

    auto publicIt = m_tokens.find(packet->tokenId());

    if (publicIt == m_tokens.end()) {
        return;
    }

    SharedPtr<Token> &pub = publicIt->second;

    pub->callLoadProperties(map);

    grant(packet->tokenId(), priv, pub);
}

void ClusterLoop::packetRequestToken(CorePackets::RequestToken *packet, u32 nodeId)
{
    if (!m_started)
        return;

    TDM_LOG("Received token request for ", packet->tokenId(), " from ", nodeId);

    auto privateIt = m_tokenPrivate.find(packet->tokenId());
    TokenPrivateData *priv = nullptr;

    if (privateIt == m_tokenPrivate.end()) {
        priv = new TokenPrivateData(m_nodeCount, true, false);

        m_tokenPrivate.emplace(std::piecewise_construct, std::forward_as_tuple(packet->tokenId()), std::forward_as_tuple(priv));
    } else {
        priv = privateIt->second.get();
    }

    priv->updateRequestNumber(nodeId, packet->sequenceId());

    if (priv->owned() && !priv->locked() && priv->rn(nodeId) == priv->ln(nodeId) + 1) {
        sendToken(packet->tokenId(), nodeId, priv);
    }
}

void ClusterLoop::packetSignal(CorePackets::Signal *packet, u32 nodeId)
{
    UNUSED(nodeId);

    TDM_LOG("Received signal for ", packet->tokenId());

    if (!m_started)
        return;

    auto publicIt = m_tokens.find(packet->tokenId());

    if (publicIt == m_tokens.end()) {
        return;
    }

    SharedPtr<Token> &pub = publicIt->second;

    pub->callSignal(packet->variableId());
}

void ClusterLoop::requestLock(const String &monitor)
{
    TDM_LOG("Lock requested for ", monitor);

    auto privateIt = m_tokenPrivate.find(monitor);
    auto publicIt = m_tokens.find(monitor);

    if (privateIt == m_tokenPrivate.end()) {
        return;
    }

    if (publicIt == m_tokens.end()) {
        return;
    }

    TokenPrivateData *priv = privateIt->second.get();
    SharedPtr<Token> &pub = publicIt->second;

    if (priv->locked()) {
        return;
    }

    m_pendingGrants.insert(monitor);

    if (m_startedCluster) {
        if (priv->owned()) {
            grant(monitor, priv, pub);
        } else {
            requestToken(monitor);
        }
    }
}

void ClusterLoop::requestUnlock(const String &monitor)
{
    TDM_LOG("Unlock requested for ", monitor);

    auto privateIt = m_tokenPrivate.find(monitor);

    if (privateIt == m_tokenPrivate.end()) {
        return;
    }

    TokenPrivateData *priv = privateIt->second.get();

    if (!priv->locked()) {
        return;
    }

    priv->setLocked(false);

    if (priv->release(m_ourNodeId)) {
        u32 target = priv->queue().front();
        priv->queue().pop();
        sendToken(monitor, target, priv);
    }
}

void ClusterLoop::requestSignal(const String &monitor, const String &condVar)
{
    TDM_LOG("Signal requested for ", monitor, ": ", condVar);

    CorePackets::Signal *signal = new CorePackets::Signal;
    signal->setTokenId(monitor);
    signal->setVariableId(condVar);
    broadcast(signal);
    delete signal;
}

void ClusterLoop::broadcast(const Packet *packet)
{
    Vector<SharedPtr<Client>> clients;

    for (auto &x : m_nodes) {
        clients.push_back(x.second);
    }

    m_tcp->sendTo(clients, packet);
}

SystemToken *ClusterLoop::systemToken()
{
    return static_cast<SystemToken*>(m_tokenPrivate["___system_token"].get());
}

void ClusterLoop::requestToken(const String &id)
{
    TDM_LOG("Requesting token ", id);

    if (m_pendingRequests.count(id) == 0) {
        TDM_LOG("No pending requests, sending ...");

        m_pendingRequests.insert(id);

        if (m_startedCluster) {
            CorePackets::RequestToken *request = new CorePackets::RequestToken;
            request->setTokenId(id);
            request->setSequenceId(m_tokenPrivate[id]->incrementRequestNumber(m_ourNodeId));
            broadcast(request);
            delete request;
        }
    }
}

void ClusterLoop::grant(const String &id)
{
    auto privateIt = m_tokenPrivate.find(id);

    if (privateIt == m_tokenPrivate.end()) {
        // ???
        return;
    }

    grant(id, privateIt->second.get());
}

void ClusterLoop::grant(const String &id, TokenPrivateData *priv)
{
    auto publicIt = m_tokens.find(id);

    if (publicIt == m_tokens.end()) {
        return;
    }

    grant(id, priv, publicIt->second);
}

void ClusterLoop::grant(const String &id, TokenPrivateData *priv, SharedPtr<Token> &pub)
{
    TDM_LOG("Trying to grant ", id);

    if (m_pendingGrants.count(id) > 0) {
        TDM_LOG("Granted");
        priv->setLocked(true);
        m_pendingGrants.erase(id);
        pub->grant();
    }
}

void ClusterLoop::grant(const String &id, SharedPtr<Token> &pub)
{
    auto privateIt = m_tokenPrivate.find(id);

    if (privateIt == m_tokenPrivate.end()) {
        // ???
        return;
    }

    grant(id, privateIt->second.get(), pub);
}

void ClusterLoop::sendToken(const String &id, u32 node, TokenPrivateData *priv)
{
    TDM_LOG("Sending token ", id, " to ", node);

    PropertyMap map;

    priv->saveProperties(map);

    auto publicIt = m_tokens.find(id);
    if (publicIt != m_tokens.end()) {
        SharedPtr<Token> &pub = publicIt->second;

        pub->callSaveProperties(map);
    }

    priv->setOwned(false);

    CorePackets::GiveToken *give = new CorePackets::GiveToken;
    give->setTokenId(id);
    give->setData(m_marshaller->marshallMap(map));
    m_tcp->sendTo(m_nodes[node], give);
    delete give;
}

END_NAMESPACE
