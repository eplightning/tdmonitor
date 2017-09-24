#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/packet.h>
#include <tdmonitor/tcp_manager.h>
#include <tdmonitor/token.h>

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>

TDM_NAMESPACE

/**
 * @brief Reprezentuje wydarzenie w kolejce
 */
class Event {
public:
    enum class Type {
        Packet = 1,
        Connected,
        Disconnected,
        Request,
        NewMonitor
    };

    virtual ~Event();
    virtual Type type() const = 0;
};

/**
 * @brief Kolejka wydarzeń
 */
class EventQueue {
public:
    EventQueue();
    ~EventQueue();

    void append(Event *event);
    Event *pop();
    void stop();

protected:
    std::condition_variable m_cond;
    std::mutex m_mutex;
    std::queue<Event*> m_events;
    bool m_stopped;
};

#define BIND_EVENT_LOOP_HANDLER(O, F) std::bind(F, O, std::placeholders::_1)

typedef std::function<bool(Event*)> EventLoopDelegate;

/**
 * @brief Główna pętla aplikacji obsługująca wydarzenia pojawiające się w kolejce
 */
class EventLoop {
public:
    EventLoop(int workers, EventQueue *evq, EventLoopDelegate func);
    ~EventLoop();

    void run();
    void startThreads();
    void waitForThreads();

protected:
    int m_workers;
    EventQueue *m_evq;
    EventLoopDelegate m_handler;
    Vector<std::thread> m_threads;
};

class EventPacket : public Event {
public:
    EventPacket(Packet *packet, u32 nodeId);

    Type type() const;

    Packet *packet() const;
    u32 nodeId() const;

protected:
    Packet *m_packet;
    u32 m_nodeId;
};

class EventConnected : public Event {
public:
    EventConnected(const SharedPtr<Client> &client, u32 nodeId);

    Type type() const;

    const SharedPtr<Client> &client() const;
    u32 nodeId() const;

protected:
    SharedPtr<Client> m_client;
    u32 m_nodeId;
};

class EventDisconnected : public Event {
public:
    EventDisconnected(u32 nodeId);

    Type type() const;

    u32 nodeId() const;

protected:
    u32 m_nodeId;
};

class EventRequest : public Event {
public:
    enum class RequestId {
        Lock = 0,
        Unlock = 1,
        Signal = 2
    };

    EventRequest(RequestId requestType, const String &monitor);
    EventRequest(RequestId requestType, const String &monitor, const String &condVar);

    Type type() const;

    RequestId requestType() const;
    const String &monitor() const;
    const String &condVar() const;

protected:
    RequestId m_requestType;
    String m_monitor;
    String m_condVar;
};

class EventNewMonitor : public Event {
public:
    EventNewMonitor(const SharedPtr<Token> &token);

    Type type() const;

    const SharedPtr<Token> &token() const;

protected:
    SharedPtr<Token> m_token;
};


END_NAMESPACE
