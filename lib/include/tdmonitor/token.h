#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/marshaller.h>

#include <condition_variable>
#include <functional>
#include <queue>
#include <set>

TDM_NAMESPACE

typedef std::function<void()> SignalDelegate;

typedef std::function<void(const PropertyMap &)> LoadPropertiesDelegate;
typedef std::function<void(PropertyMap &)> SavePropertiesDelegate;

class TokenPrivateData {
public:
    TokenPrivateData(int nodes, bool created, bool owned);
    virtual ~TokenPrivateData();

    u32 incrementRequestNumber(u32 node);
    void updateRequestNumber(u32 node, u32 request);
    bool release(u32 node);

    u32 rn(u32 node) const;
    u32 ln(u32 node) const;
    std::queue<u32> &queue();

    bool created() const;
    bool owned() const;
    bool locked() const;

    void setCreated(bool created);
    void setOwned(bool owned);
    void setLocked(bool locked);

    virtual void loadProperties(const PropertyMap &properties);
    virtual void saveProperties(PropertyMap &properties) const;

private:
    // LN(n), RN(n), Q algorytmu
    u32 *m_ln;
    u32 *m_rn;
    int m_nodes;
    std::queue<u32> m_queue;

    // czy token był utworzony (występuje w tokenie systemowym albo ktoś nam wysłał już requesty)
    bool m_created;

    // czy posiadamy obecnie token
    bool m_owned;

    // czy jesteśmy w sekcji
    bool m_locked;
};

class SystemToken : public TokenPrivateData {
public:
    SystemToken(int nodes, bool owned);
    ~SystemToken();

    void loadProperties(const PropertyMap &properties);
    void saveProperties(PropertyMap &properties) const;

    void addMonitor(const String &monitor);
    bool hasMonitor(const String &monitor);

private:
    std::set<String> m_monitors;
};

class Token {
public:
    friend class ClusterLoop;

    Token(const String &id, const HashMap<String, SignalDelegate> &condVars, LoadPropertiesDelegate load, SavePropertiesDelegate save);

    const String &id() const;

    void takeGrant();

private:
    void callSignal(const String &id) const;
    void callLoadProperties(const PropertyMap &properties) const;
    void callSaveProperties(PropertyMap &properties) const;
    void grant();

    String m_id;
    HashMap<String, SignalDelegate> m_condVars;
    LoadPropertiesDelegate m_load;
    SavePropertiesDelegate m_save;

    Mutex m_mutex;
    std::condition_variable m_grantedCondition;
    bool m_granted;
};

END_NAMESPACE
