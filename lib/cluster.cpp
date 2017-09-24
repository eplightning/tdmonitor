#include <tdmonitor/cluster.h>

#include <tdmonitor/types.h>

TDM_NAMESPACE

Cluster::Cluster(const Vector<String> &nodes, u32 nodeId, SharedPtr<DataMarshaller> &marshaller) :
    m_nodeAddresses(nodes), m_ourNodeId(nodeId), m_marshaller(marshaller), m_evq(new EventQueue)
{
    initCluster();
}

Cluster::Cluster(const Vector<String> &nodes, u32 nodeId) :
    m_nodeAddresses(nodes), m_ourNodeId(nodeId), m_marshaller(new DataMarshaller), m_evq(new EventQueue)
{
    initCluster();
}

Cluster::~Cluster()
{

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

void Cluster::initCluster()
{

}

END_NAMESPACE
