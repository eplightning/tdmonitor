#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/marshaller.h>
#include <tdmonitor/cluster.h>
#include <tdmonitor/token.h>

#include <condition_variable>

TDM_NAMESPACE

struct PropertyBinding {
    String type;
    void *ptr;

    PropertyBinding(const String &type, void *ptr);
};

class Monitor;

class MonitorConditionVariable {
public:
    MonitorConditionVariable(const String &id, Monitor &monitor);

    void signal();
    void wait();

private:
    String m_id;
    Monitor &m_monitor;
};

class Monitor {
public:
    // friend Cluster
    Monitor(Cluster &cluster, const String &id);

    // setup methods
    void property(const String &id, u8 &data);
    void property(const String &id, s8 &data);
    void property(const String &id, u16 &data);
    void property(const String &id, s16 &data);
    void property(const String &id, u32 &data);
    void property(const String &id, s32 &data);
    void property(const String &id, u64 &data);
    void property(const String &id, s64 &data);
    void property(const String &id, float &data);
    void property(const String &id, double &data);
    void property(const String &id, Vector<u8> &data);
    void property(const String &id, Vector<s8> &data);
    void property(const String &id, Vector<u16> &data);
    void property(const String &id, Vector<s16> &data);
    void property(const String &id, Vector<u32> &data);
    void property(const String &id, Vector<s32> &data);
    void property(const String &id, Vector<u64> &data);
    void property(const String &id, Vector<s64> &data);
    void property(const String &id, Vector<float> &data);
    void property(const String &id, Vector<double> &data);
    void property(const String &id, const String &type, SharedPtr<void> &data);
    MonitorConditionVariable condition(const String &id);
    void create();

    void lock();
    void unlock();
    void signal(const String &condVar);
    void wait(const String &condVar);

private:
    void loadProperties(const PropertyMap &properties);
    void saveProperties(PropertyMap &properties) const;

    String m_id;
    Mutex m_mutex;
    HashMap<String, std::condition_variable> m_conditions;
    HashMap<String, PropertyBinding> m_properties;
    Cluster &m_cluster;
    SharedPtr<Token> m_token;
};

END_NAMESPACE
