#include <tdmonitor/tcp.h>

#include <tdmonitor/types.h>

#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

TDM_NAMESPACE

int SocketUtils::createListenSocket(const String &address, u16 port, ConnectionProtocol proto)
{
    int sock = ::socket((proto == CPIpv4) ? AF_INET : AF_INET6, SOCK_STREAM, 0);

    if (sock == -1)
        return -1;

    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        return -1;

    sockaddr_storage addr;
    socklen_t addrLen = (proto == CPIpv4) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);

    if (!fillSockaddr(&addr, address, port, proto))
        return -4;

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), addrLen) == -1)
        return -2;

    if (::listen(sock, SOMAXCONN) == -1)
        return -3;

    if (!makeNonBlocking(sock))
        return -6;

    return sock;
}

int SocketUtils::createListenSocket(const String &full, ConnectionProtocol &proto)
{
    String address;
    u16 port;
    proto = readAddress(full, port, address);

    if (proto == CPUnknown)
        return -5;

    return createListenSocket(address, port, proto);
}

bool SocketUtils::fillSockaddr(sockaddr_storage *sockaddr, const String &address, u16 port, ConnectionProtocol proto)
{
    void *inaddr;

    memset(sockaddr, 0, sizeof(sockaddr_storage));

    if (proto == CPIpv4) {
        sockaddr_in *addr4 = reinterpret_cast<sockaddr_in*>(sockaddr);

        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port);
        inaddr = &addr4->sin_addr;
    } else {
        sockaddr_in6 *addr6 = reinterpret_cast<sockaddr_in6*>(sockaddr);

        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(port);
        inaddr = &addr6->sin6_addr;
    }

    if (inet_pton((proto == CPIpv4) ? AF_INET : AF_INET6, address.c_str(), inaddr) != 1)
        return false;

    return true;
}

bool SocketUtils::makeNonBlocking(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);

    if (flags == -1)
        return false;

    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) != -1;
}

ConnectionProtocol SocketUtils::readAddress(const String &full, u16 &port, String &address)
{
    if (full.empty())
        return CPUnknown;

    size_t sep = full.find_last_of(':');

    if (sep == String::npos || sep == full.size() - 1 || sep == 0)
        return CPUnknown;

    try {
        port = static_cast<u16>(std::stoul(full.substr(sep + 1)));
    } catch (std::exception &e) {
        return CPUnknown;
    }

    // IPv6 ([ipv6address]:port)
    if (full.front() == '[') {
        if (sep <= 2 || full.at(sep - 1) != ']')
            return CPUnknown;

        address.assign(full, 1, sep - 2);

        return CPIpv6;
    // IPv4 (ipv4address:port)
    } else {
        address.assign(full, 0, sep);

        return CPIpv4;
    }
}

ConnectionProtocol SocketUtils::getProtoFromIp(const String &ip)
{
    if (ip.find('.') != String::npos)
        return CPIpv4;

    if (ip.find(':') != String::npos)
        return CPIpv6;

    return CPUnknown;
}

TcpReceiveBuffer::TcpReceiveBuffer()
    : received(0), data(Packet::MaxSize)
{

}

END_NAMESPACE
