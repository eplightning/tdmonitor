#include <tdmonitor/event.h>

#include <tdmonitor/types.h>
#include <tdmonitor/packet.h>

TDM_NAMESPACE

Event::~Event()
{

}

EventQueue::EventQueue()
    : m_cond(), m_mutex(), m_events(), m_stopped(false)
{

}

EventQueue::~EventQueue()
{
    while (!m_events.empty()) {
        Event *ev = m_events.front();
        delete ev;
        m_events.pop();
    }
}

void EventQueue::append(Event *event)
{
    m_mutex.lock();
    m_events.push(event);
    m_mutex.unlock();

    m_cond.notify_one();
}

Event *EventQueue::pop()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    while (!m_stopped && m_events.empty())
        m_cond.wait(lock);

    if (m_stopped)
        return nullptr;

    Event *ev = m_events.front();
    m_events.pop();

    return ev;
}

void EventQueue::stop()
{
    m_mutex.lock();
    m_stopped = true;
    m_mutex.unlock();

    m_cond.notify_all();
}

EventLoop::EventLoop(int workers, EventQueue *evq, EventLoopDelegate func)
    : m_workers(workers), m_evq(evq), m_handler(func)
{

}

EventLoop::~EventLoop()
{
    waitForThreads();
}

void EventLoop::run()
{
    Event *ev;

    do {
        ev = m_evq->pop();

        if (ev == nullptr)
            break;
    } while (m_handler(ev));
}

void EventLoop::startThreads()
{
    if (!m_threads.empty())
        waitForThreads();

    for (int i = 0; i < m_workers; i++)
        m_threads.emplace_back(&EventLoop::run, this);
}

void EventLoop::waitForThreads()
{
    for (auto &x : m_threads) {
        if (x.joinable())
            x.join();
    }

    m_threads.clear();
}

EventPacket::EventPacket(Packet *packet, u32 nodeId) :
    m_packet(packet), m_nodeId(nodeId)
{

}

Event::Type EventPacket::type() const
{
    return Event::Type::Packet;
}

Packet *EventPacket::packet() const
{
    return m_packet;
}

u32 EventPacket::nodeId() const
{
    return m_nodeId;
}

EventConnected::EventConnected(const SharedPtr<Client> &client, u32 nodeId) :
    m_client(client), m_nodeId(nodeId)
{

}

Event::Type EventConnected::type() const
{
    return Event::Type::Connected;
}

const SharedPtr<Client> &EventConnected::client() const
{
    return m_client;
}

u32 EventConnected::nodeId() const
{
    return m_nodeId;
}

EventDisconnected::EventDisconnected(u32 nodeId) :
    m_nodeId(nodeId)
{

}

Event::Type EventDisconnected::type() const
{
    return Event::Type::Disconnected;
}

u32 EventDisconnected::nodeId() const
{
    return m_nodeId;
}

EventRequest::EventRequest(EventRequest::RequestId requestType, const String &monitor) :
    m_requestType(requestType), m_monitor(monitor), m_condVar()
{

}

EventRequest::EventRequest(EventRequest::RequestId requestType, const String &monitor, const String &condVar) :
    m_requestType(requestType), m_monitor(monitor), m_condVar(condVar)
{

}

Event::Type EventRequest::type() const
{
    return Event::Type::Request;
}

EventRequest::RequestId EventRequest::requestType() const
{
    return m_requestType;
}

const String &EventRequest::monitor() const
{
    return m_monitor;
}

const String &EventRequest::condVar() const
{
    return m_condVar;
}

EventNewMonitor::EventNewMonitor(const SharedPtr<Token> &token) :
    m_token(token)
{

}

Event::Type EventNewMonitor::type() const
{
    return Event::Type::NewMonitor;
}

const SharedPtr<Token> &EventNewMonitor::token() const
{
    return m_token;
}

END_NAMESPACE
