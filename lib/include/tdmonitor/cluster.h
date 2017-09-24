#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/marshaller.h>
#include <tdmonitor/event.h>

TDM_NAMESPACE

class Cluster {
public:
    Cluster(const Vector<String> &nodes, u32 nodeId, SharedPtr<DataMarshaller> &marshaller);
    Cluster(const Vector<String> &nodes, u32 nodeId);
    ~Cluster();

    void sendLockRequest(const String &monitor);
    void sendUnlockRequest(const String &monitor);
    void sendSignalRequest(const String &monitor, const String &condVar);
    void sendCreateRequest(const SharedPtr<Token> &token);

private:
    void initCluster();

    Vector<String> m_nodeAddresses;
    u32 m_ourNodeId;
    SharedPtr<DataMarshaller> m_marshaller;
    UniquePtr<EventQueue> m_evq;
};

END_NAMESPACE
