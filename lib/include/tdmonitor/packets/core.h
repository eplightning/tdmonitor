#pragma once

#include <tdmonitor/packet.h>
#include <tdmonitor/types.h>

#define COREPACKETS_NAMESPACE namespace CorePackets {

TDM_NAMESPACE COREPACKETS_NAMESPACE

class EdgeHandshake : public Packet {
public:
    EdgeHandshake();

    bool decodePayload(const Vector<char> &payload);
    void encodePayload(Vector<char> &payload) const;

    u32 id() const;
    void setId(u32 id);

protected:
    u32 m_id;
};

class ClusterHandshake : public Packet {
public:
    ClusterHandshake();

    bool decodePayload(const Vector<char> &payload);
    void encodePayload(Vector<char> &payload) const;

    u32 id() const;
    void setId(u32 id);

protected:
    u32 m_id;
};

class RequestToken : public Packet {
public:
    RequestToken();

    bool decodePayload(const Vector<char> &payload);
    void encodePayload(Vector<char> &payload) const;

    const String &tokenId() const;
    void setTokenId(const String &tokenId);

    u32 sequenceId() const;
    void setSequenceId(u32 sequenceId);

protected:
    String m_tokenId;
    u32 m_sequenceId;
};

class GiveToken : public Packet {
public:
    GiveToken();

    bool decodePayload(const Vector<char> &payload);
    void encodePayload(Vector<char> &payload) const;

    const String &tokenId() const;
    void setTokenId(const String &tokenId);

    const Vector<char> &data() const;
    void setData(const Vector<char> &data);

protected:
    String m_tokenId;
    Vector<char> m_data;
};

class Signal : public Packet {
public:
    Signal();

    bool decodePayload(const Vector<char> &payload);
    void encodePayload(Vector<char> &payload) const;

    const String &tokenId() const;
    void setTokenId(const String &tokenId);

    const String &variableId() const;
    void setVariableId(const String &variableId);

protected:
    String m_tokenId;
    String m_variableId;
};

END_NAMESPACE END_NAMESPACE
