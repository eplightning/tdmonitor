#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/packet.h>
#include <tdmonitor/selector.h>
#include <tdmonitor/tcp.h>

#include <functional>
#include <queue>
#include <mutex>

#include <arpa/inet.h>

TDM_NAMESPACE

class TcpPool;
class TcpManager;
class ListenTcpPool;
struct TcpManagerConnectInfo;

/**
 * @brief Klasa reprezentująca połączenie TCP
 */
class Client {
public:
    friend class TcpManager;

    Client(u32 id, int fd, const sockaddr *addr, TcpPool *pool, u64 userData);
    ~Client();

    u32 id() const { return m_id; }
    int socket() const { return m_socket; }
    ConnectionProtocol proto() const { return m_proto; }
    u16 port() const { return m_port; }
    TcpPool *pool() const { return m_pool; }
    const String &address() const { return m_address; }
    bool operator==(sockaddr_storage &saddr) const;
    void attachSendBuffer(TcpSendBuffer *buffer);
    u64 userData() const { return m_userData; }
    void setUserData(u64 userData) { m_userData = userData; }

protected:
    TcpReceiveBuffer &receiveBuffer() { return m_receiveBuffer; }
    TcpSendBuffer *sendBuffer();
    TcpClientState state() const { return m_state; }
    void setState(TcpClientState state) { m_state = state; }
    int writeError() const { return m_writeError; }
    void setWriteError(int error) { m_writeError = error; }
    int readError() const { return m_readError; }
    void setReadError(int error) { m_readError = error; }
    int connectError() const { return m_connectError; }
    void setConnectError(int error) { m_connectError = error; }
    bool isDead() const { return m_dead; }
    void setDead(bool dead) { m_dead = dead; }

    u32 m_id;
    int m_socket;
    ConnectionProtocol m_proto;
    u16 m_port;
    String m_address;
    sockaddr_storage m_sockaddr;
    TcpPool *m_pool;
    TcpClientState m_state;
    int m_writeError;
    int m_readError;
    int m_connectError;
    bool m_dead;
    u64 m_userData;

    TcpSendBuffer *m_sendBuffer;
    std::queue<TcpSendBuffer*> m_sendBufferQueue;
    std::mutex m_sendBufferMutex;
    TcpReceiveBuffer m_receiveBuffer;
};

struct ListenTcpPoolSocket {
    ConnectionProtocol protocol;
    int socket;
    ListenTcpPool *pool;
};

#define BIND_TCP_STATE(O, F) std::bind(F, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define BIND_TCP_NEW(O, F) std::bind(F, O, std::placeholders::_1)
#define BIND_TCP_RECV(O, F) std::bind(F, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

// zmiana stanu
typedef std::function<void(Client *clientPtr, TcpClientState state, int error)> ClientStateDelegate;

// nowe połączenie (można odrzucić)
typedef std::function<bool(SharedPtr<Client> &client)> NewConnectionDelegate;

// nowy pakiet
typedef std::function<void(Client *clientPtr, PacketHeader header, const Vector<char> &data)> ReceiveDataDelegate;

/**
 * @brief Pula, logiczny podział połączeń w TcpManagerze
 */
class TcpPool {
public:
    TcpPool(ClientStateDelegate clientState, NewConnectionDelegate newConnection, ReceiveDataDelegate receive);
    virtual ~TcpPool();

    void callClientState(Client *clientPtr, TcpClientState state, int error) const;
    bool callNewConnection(SharedPtr<Client> &client) const;
    void callReceive(Client *clientPtr, PacketHeader header, const Vector<char> &data) const;

    virtual Vector<ListenTcpPoolSocket> *listenSockets();

protected:
    ClientStateDelegate m_clientState;
    NewConnectionDelegate m_newConnection;
    ReceiveDataDelegate m_receive;
};

/**
 * @brief Pula posiadająca dodatkowo sockety nasłuchujące
 */
class ListenTcpPool : public TcpPool {
public:
    ListenTcpPool(ClientStateDelegate clientState, NewConnectionDelegate newConnection, ReceiveDataDelegate receive);
    ~ListenTcpPool();

    Vector<ListenTcpPoolSocket> *listenSockets();

    void appendListenSocket(ConnectionProtocol proto, int socket);

protected:
    Vector<ListenTcpPoolSocket> m_sockets;
};

/**
 * @brief Zarządca połączeń TCP, praktycznie cała właściwa część sieciowa jest tutaj
 */
class TcpManager {
public:
    explicit TcpManager(int rounds = 4);
    ~TcpManager();

    bool connect(ConnectionProtocol proto, const String &ip, u16 port, const String &pool, u64 userData);
    bool connect(const String &address, const String &pool, u64 userData);
    // note: poole powinny być utworzone zanim odpalimy runLoop
    void createPool(const String &name, TcpPool *pool);
    void disconnect(SharedPtr<Client> &client, bool force = false);
    void disconnectAll(bool force = false);
    void sendTo(SharedPtr<Client> &client, TcpSendBuffer *buffer);
    void sendTo(SharedPtr<Client> &client, const Packet *packet);
    void sendTo(const Vector<SharedPtr<Client>*> &clients, const Packet *packet);
    void sendTo(const Vector<SharedPtr<Client>> &clients, const Packet *packet);

    void runLoop(int timerSeconds);
    void stopLoop();

    // ONLY CALL INSIDE CALLBACK!
    SharedPtr<Client> client(u32 clientid) const;

protected:
    void newConnection(ListenTcpPoolSocket *listen, Selector *select);
    bool pipeNotification(Selector *select, Vector<Client*> &eraseList);
    void readEvent(Client *client, SelectorEvent &event, Selector *select, Vector<Client*> &eraseList, int rounds = 0);
    void writeEvent(Client *client, Selector *select, Vector<Client*> &eraseList, int rounds = 0);
    bool connectEvent(Client *client, Vector<Client*> &eraseList);
    u32 createConnection(TcpManagerConnectInfo *info, Selector *select);

    void stopReading(Client *client, Selector *select, Vector<Client*> &eraseList, int error);
    void stopWriting(Client *client, Selector *select, Vector<Client*> &eraseList, int error);

    int m_pipe[2];
    Map<String, TcpPool*> m_pools;
    HashMap<u32, SharedPtr<Client>> m_clients;
    HashMap<u32, TcpManagerConnectInfo*> m_connectionsInProgress;
    Vector<TcpManagerConnectInfo*> m_plannedConnections;
    u32 m_nextClientId;
    int m_rrRounds;
};

END_NAMESPACE
