#include <tdmonitor/token.h>

#include <tdmonitor/types.h>

#include <queue>
#include <set>

TDM_NAMESPACE

TokenPrivateData::TokenPrivateData(int nodes, bool created, bool owned) :
    m_nodes(nodes), m_created(created), m_owned(owned), m_locked(false)
{
    m_ln = new u32[nodes]();
    m_rn = new u32[nodes]();
}

TokenPrivateData::~TokenPrivateData()
{
    delete[] m_ln;
    delete[] m_rn;
}

u32 TokenPrivateData::incrementRequestNumber(u32 node)
{
    return ++m_rn[node];
}

void TokenPrivateData::updateRequestNumber(u32 node, u32 request)
{
    m_rn[node] = std::max(request, m_rn[node]);
}

bool TokenPrivateData::release(u32 node)
{
    m_ln[node] = m_rn[node];

    std::set<u32> exists;
    std::queue<u32> newQueue;

    while (!m_queue.empty()) {
        u32 node = m_queue.front();
        m_queue.pop();
        exists.insert(node);
        newQueue.push(node);
    }

    for (u32 i = 0; i < static_cast<u32>(m_nodes); ++i) {
        if (m_rn[i] == m_ln[i] + 1 && !exists.count(i)) {
            newQueue.push(i);
        }
    }

    m_queue = newQueue;

    return !newQueue.empty();
}

u32 TokenPrivateData::rn(u32 node) const
{
    return m_rn[node];
}

u32 TokenPrivateData::ln(u32 node) const
{
    return m_ln[node];
}

std::queue<u32> &TokenPrivateData::queue()
{
    return m_queue;
}

bool TokenPrivateData::created() const
{
    return m_created;
}

bool TokenPrivateData::owned() const
{
    return m_owned;
}

bool TokenPrivateData::locked() const
{
    return m_locked;
}

void TokenPrivateData::setCreated(bool created)
{
    m_created = created;
}

void TokenPrivateData::setOwned(bool owned)
{
    m_owned = owned;
}

void TokenPrivateData::setLocked(bool locked)
{
    m_locked = locked;
}

void TokenPrivateData::loadProperties(const PropertyMap &properties)
{
    auto queueIt = properties.find("___token_queue");
    auto lnIt = properties.find("___token_ln");

    if (queueIt == properties.cend() || lnIt == properties.cend()) {
        // wyjątek
        return;
    }

    const Property &queueProp = queueIt->second;
    const Property &lnProp = lnIt->second;

    const SharedPtr<Vector<u32>> &queuePtr = *(reinterpret_cast<const SharedPtr<Vector<u32>>*>(&queueProp.value.ptr));
    Vector<u32> *queue = queuePtr.get();
    const SharedPtr<Vector<u32>> &lnPtr = *(reinterpret_cast<const SharedPtr<Vector<u32>>*>(&lnProp.value.ptr));
    Vector<u32> *ln = lnPtr.get();

    if (ln->size() != static_cast<size_t>(m_nodes)) {
        // wyjątek
        return;
    }

    // queue
    std::queue<u32> newQueue;

    for (auto x : *queue) {
        newQueue.push(x);
    }

    m_queue.swap(newQueue);

    // ln
    for (u32 i = 0; i < static_cast<u32>(m_nodes); ++i) {
        m_ln[i] = (*ln)[i];
    }
}

void TokenPrivateData::saveProperties(PropertyMap &properties) const
{
    SharedPtr<Vector<u32>> lnPtr(new Vector<u32>(m_nodes));
    SharedPtr<Vector<u32>> queuePtr(new Vector<u32>());
    Vector<u32> &ln = *lnPtr;
    Vector<u32> &queue = *queuePtr;

    // ln
    for (u32 i = 0; i < static_cast<u32>(m_nodes); ++i) {
        ln[i] = m_ln[i];
    }

    // queue
    std::queue<u32> queueCopy(m_queue);

    while (!queueCopy.empty()) {
        queue.push_back(queueCopy.front());
        queueCopy.pop();
    }

    PropertyValue lnValue(lnPtr);
    PropertyValue queueValue(queuePtr);

    properties.emplace(std::piecewise_construct,
                       std::forward_as_tuple("___token_ln"),
                       std::forward_as_tuple("___token_ln", "#u32", lnValue));

    properties.emplace(std::piecewise_construct,
                       std::forward_as_tuple("___token_queue"),
                       std::forward_as_tuple("___token_queue", "#u32", queueValue));
}

Token::Token(const String &id, const HashMap<String, SignalDelegate> &condVars, LoadPropertiesDelegate load, SavePropertiesDelegate save) :
    m_id(id), m_condVars(condVars), m_load(load), m_save(save), m_granted(false)
{

}

const String &Token::id() const
{
    return m_id;
}

void Token::takeGrant()
{
    UniqueLock lock(m_mutex);

    while (!m_granted) {
        m_grantedCondition.wait(lock);
    }

    m_granted = false;
}

void Token::callSignal(const String &id) const
{
    auto signal = m_condVars.find(id);

    if (signal == m_condVars.cend()) {
        return;
    }

    signal->second();
}

void Token::callLoadProperties(const PropertyMap &properties) const
{
    m_load(properties);
}

void Token::callSaveProperties(PropertyMap &properties) const
{
    m_save(properties);
}

void Token::grant()
{
    MutexLock lock(m_mutex);

    m_granted = true;
    m_grantedCondition.notify_one();
}

SystemToken::SystemToken(int nodes, bool owned) :
    TokenPrivateData(nodes, true, owned)
{

}

SystemToken::~SystemToken()
{

}

void SystemToken::loadProperties(const PropertyMap &properties)
{
    TokenPrivateData::loadProperties(properties);

    auto vectorIt = properties.find("monitors");

    if (vectorIt == properties.cend()) {
        // wyjątek
        return;
    }

    const Property &vectorProp = vectorIt->second;

    const SharedPtr<Vector<String>> &vectorPtr = *(reinterpret_cast<const SharedPtr<Vector<String>>*>(&vectorProp.value.ptr));
    Vector<String> *vector = vectorPtr.get();

    m_monitors.clear();

    for (auto &x : *vector)
        m_monitors.insert(x);
}

void SystemToken::saveProperties(PropertyMap &properties) const
{
    TokenPrivateData::saveProperties(properties);

    SharedPtr<Vector<String>> vectorPtr(new Vector<String>(m_monitors.size()));
    Vector<String> &vector = *vectorPtr;

    for (auto &x : m_monitors) {
        vector.push_back(x);
    }

    PropertyValue vectorValue(vectorPtr);

    properties.emplace(std::piecewise_construct,
                       std::forward_as_tuple("monitors"),
                       std::forward_as_tuple("monitors", "Vector<String>", vectorValue));
}

void SystemToken::addMonitor(const String &monitor)
{
    m_monitors.insert(monitor);
}

bool SystemToken::hasMonitor(const String &monitor)
{
    return m_monitors.count(monitor) > 0;
}

END_NAMESPACE
