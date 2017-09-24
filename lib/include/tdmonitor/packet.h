#pragma once

#include <tdmonitor/types.h>

#include <arpa/inet.h>

#define PACKETFACTORY_CASE(X, Y) case X: out = new Y; break;

TDM_NAMESPACE

struct PacketHeader {
    u16 type;
    u32 payloadSize;
};

class Packet {
public:
    const static uint MaxSize;
    const static uint HeaderSize;
    const static uint MaxPayloadSize;
    const static uint MaxVectorSize;

    enum class Direction : u8 {
        NodeToNode = 0
    };

    enum class Type : u16 {
        Unknown = 0,

        EdgeHandshake = 1,
        ClusterHandshake = 2,
        RequestToken = 3,
        GiveToken = 4,
        Signal = 5
    };

    static bool checkDirection(u16 rawType, Direction dir);
    static Packet *factory(PacketHeader header, const Vector<char> &data);

public:
    Packet(Type type);
    virtual ~Packet();

    virtual bool decodePayload(const Vector<char> &payload) = 0;
    virtual void encodePayload(Vector<char> &payload) const = 0;

    void encode(Vector<char> &packet) const;
    Vector<char> encode() const;
    Type packetType() const;

protected:
    void write(Vector<char> &payload, u8 data) const;
    void write(Vector<char> &payload, s8 data) const;
    void write(Vector<char> &payload, u16 data) const;
    void write(Vector<char> &payload, s16 data) const;
    void write(Vector<char> &payload, u32 data) const;
    void write(Vector<char> &payload, s32 data) const;
    void write(Vector<char> &payload, u64 data) const;
    void write(Vector<char> &payload, s64 data) const;
    void write(Vector<char> &payload, const String &data) const;
    void write(Vector<char> &payload, const Vector<char> &data) const;
    void writeVectorSize(Vector<char> &payload, size_t data) const;

    bool read(const Vector<char> &payload, u8 &data);
    bool read(const Vector<char> &payload, s8 &data);
    bool read(const Vector<char> &payload, s16 &data);
    bool read(const Vector<char> &payload, u16 &data);
    bool read(const Vector<char> &payload, s32 &data);
    bool read(const Vector<char> &payload, u32 &data);
    bool read(const Vector<char> &payload, s64 &data);
    bool read(const Vector<char> &payload, u64 &data);
    bool read(const Vector<char> &payload, String &data);
    bool read(const Vector<char> &payload, Vector<char> &data);
    bool readVectorSize(const Vector<char> &payload, u32 &data);

    u64 hostToNetwork(u64 data) const;
    s64 hostToNetwork(s64 data) const;
    u64 networkToHost(u64 data) const;
    s64 networkToHost(s64 data) const;

protected:
    Type m_packetType;
    uint m_packetReaderPos;
};

inline void Packet::write(Vector<char> &payload, u8 data) const
{
    payload.push_back(data);
}

inline void Packet::write(Vector<char> &payload, s8 data) const
{
    payload.push_back(data);
}

inline void Packet::write(Vector<char> &payload, u16 data) const
{
    data = htons(data);
    char *raw = reinterpret_cast<char*>(&data);
    payload.insert(payload.end(), raw, raw + 2);
}

inline void Packet::write(Vector<char> &payload, s16 data) const
{
    data = htons(data);
    char *raw = reinterpret_cast<char*>(&data);
    payload.insert(payload.end(), raw, raw + 2);
}

inline void Packet::write(Vector<char> &payload, u32 data) const
{
    data = htonl(data);
    char *raw = reinterpret_cast<char*>(&data);
    payload.insert(payload.end(), raw, raw + 4);
}

inline void Packet::write(Vector<char> &payload, s32 data) const
{
    data = htonl(data);
    char *raw = reinterpret_cast<char*>(&data);
    payload.insert(payload.end(), raw, raw + 4);
}

inline void Packet::write(Vector<char> &payload, u64 data) const
{
    data = hostToNetwork(data);
    char *raw = reinterpret_cast<char*>(&data);
    payload.insert(payload.end(), raw, raw + 8);
}

inline void Packet::write(Vector<char> &payload, s64 data) const
{
    data = hostToNetwork(data);
    char *raw = reinterpret_cast<char*>(&data);
    payload.insert(payload.end(), raw, raw + 8);
}

inline void Packet::write(Vector<char> &payload, const String &data) const
{
    write(payload, static_cast<u32>(data.size()));
    payload.insert(payload.end(), data.cbegin(), data.cend());
}

inline void Packet::write(Vector<char> &payload, const Vector<char> &data) const
{
    write(payload, static_cast<u32>(data.size()));
    payload.insert(payload.end(), data.cbegin(), data.cend());
}

inline void Packet::writeVectorSize(Vector<char> &payload, size_t data) const
{
    write(payload, static_cast<u32>(data));
}

inline bool Packet::read(const Vector<char> &payload, u8 &data)
{
    if (payload.size() < 1 + m_packetReaderPos)
        return false;

    data = static_cast<u8>(payload[m_packetReaderPos]);
    m_packetReaderPos += 1;
    return true;
}

inline bool Packet::read(const Vector<char> &payload, s8 &data)
{
    if (payload.size() < 1 + m_packetReaderPos)
        return false;

    data = static_cast<s8>(payload[m_packetReaderPos]);
    m_packetReaderPos += 1;
    return true;
}

inline bool Packet::read(const Vector<char> &payload, s16 &data)
{
    if (payload.size() < 2 + m_packetReaderPos)
        return false;

    data = ntohs(*(reinterpret_cast<const s16*>(&payload[m_packetReaderPos])));
    m_packetReaderPos += 2;
    return true;
}

inline bool Packet::read(const Vector<char> &payload, u16 &data)
{
    if (payload.size() < 2 + m_packetReaderPos)
        return false;

    data = ntohs(*(reinterpret_cast<const u16*>(&payload[m_packetReaderPos])));
    m_packetReaderPos += 2;
    return true;
}

inline bool Packet::read(const Vector<char> &payload, s32 &data)
{
    if (payload.size() < 4 + m_packetReaderPos)
        return false;

    data = ntohl(*(reinterpret_cast<const s32*>(&payload[m_packetReaderPos])));
    m_packetReaderPos += 4;
    return true;
}

inline bool Packet::read(const Vector<char> &payload, u32 &data)
{
    if (payload.size() < 4 + m_packetReaderPos)
        return false;

    data = ntohl(*(reinterpret_cast<const u32*>(&payload[m_packetReaderPos])));
    m_packetReaderPos += 4;
    return true;
}

inline bool Packet::read(const Vector<char> &payload, s64 &data)
{
    if (payload.size() < 8 + m_packetReaderPos)
        return false;

    data = networkToHost(*(reinterpret_cast<const s64*>(&payload[m_packetReaderPos])));
    m_packetReaderPos += 8;
    return true;
}

inline bool Packet::read(const Vector<char> &payload, u64 &data)
{
    if (payload.size() < 8 + m_packetReaderPos)
        return false;

    data = networkToHost(*(reinterpret_cast<const u64*>(&payload[m_packetReaderPos])));
    m_packetReaderPos += 8;
    return true;
}

inline bool Packet::read(const Vector<char> &payload, String &data)
{
    u32 len;
    if (!read(payload, len) || payload.size() < len + m_packetReaderPos)
        return false;

    data.reserve(len);
    data.assign(payload.cbegin() + m_packetReaderPos, payload.cbegin() + m_packetReaderPos + len);
    m_packetReaderPos += len;
    return true;
}

inline bool Packet::read(const Vector<char> &payload, Vector<char> &data)
{
    u32 len;
    if (!read(payload, len) || payload.size() < len + m_packetReaderPos)
        return false;

    data.reserve(len);
    data.assign(payload.cbegin() + m_packetReaderPos, payload.cbegin() + m_packetReaderPos + len);
    m_packetReaderPos += len;
    return true;
}

inline bool Packet::readVectorSize(const Vector<char> &payload, u32 &data)
{
    if (!read(payload, data))
        return false;

    return data <= MaxVectorSize;
}

inline u64 Packet::hostToNetwork(u64 data) const
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u32 high = htonl(static_cast<u32>(data >> 32));
    u32 low = htonl(static_cast<u32>(data & 0xFFFFFFFFLL));

    return (static_cast<u64>(low) << 32) | high;
#else
    return data;
#endif
}

inline s64 Packet::hostToNetwork(s64 data) const
{
    return static_cast<s64>(hostToNetwork(static_cast<u64>(data)));
}

inline u64 Packet::networkToHost(u64 data) const
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u32 high = ntohl(static_cast<u32>(data >> 32));
    u32 low = ntohl(static_cast<u32>(data & 0xFFFFFFFFLL));

    return (static_cast<u64>(low) << 32) | high;
#else
    return data;
#endif
}

inline s64 Packet::networkToHost(s64 data) const
{
    return static_cast<s64>(networkToHost(static_cast<u64>(data)));
}

END_NAMESPACE
