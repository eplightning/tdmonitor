#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/marshaller.h>

#include <condition_variable>

TDM_NAMESPACE

struct PropertyBinding {
    String type;
    void *ptr;

    PropertyBinding(const String &type, void *ptr);
};

class MonitorConditionVariable {
public:
    MonitorConditionVariable(const String &id);

    void signal();
    void wait();

private:
    std::condition_variable m_condVar;
    // klastr
};

class Monitor {
public:
    friend class MonitorConditionVariable;
    // friend Cluster

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

    void lock();
    void unlock();
    MonitorConditionVariable &condition(const String &id);

private:
    void loadProperties(const PropertyMap &properties);
    void saveProperties(PropertyMap &properties) const;

    Mutex m_mutex;
    HashMap<String, PropertyBinding> m_properties;
    // klastr
};

END_NAMESPACE
