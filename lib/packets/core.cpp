#include <tdmonitor/packets/core.h>

#include <tdmonitor/packet.h>
#include <tdmonitor/types.h>

TDM_NAMESPACE COREPACKETS_NAMESPACE

EdgeHandshake::EdgeHandshake() :
    Packet(Packet::Type::EdgeHandshake)
{

}

bool EdgeHandshake::decodePayload(const Vector<char> &payload)
{
    return read(payload, m_id);
}

void EdgeHandshake::encodePayload(Vector<char> &payload) const
{
    write(payload, m_id);
}

u32 EdgeHandshake::id() const
{
    return m_id;
}

void EdgeHandshake::setId(u32 id)
{
    m_id = id;
}

ClusterHandshake::ClusterHandshake() :
    Packet(Packet::Type::ClusterHandshake)
{

}

bool ClusterHandshake::decodePayload(const Vector<char> &payload)
{
    return read(payload, m_id);
}

void ClusterHandshake::encodePayload(Vector<char> &payload) const
{
    write(payload, m_id);
}

u32 ClusterHandshake::id() const
{
    return m_id;
}

void ClusterHandshake::setId(u32 id)
{
    m_id = id;
}

RequestToken::RequestToken() :
    Packet(Packet::Type::RequestToken)
{

}

bool RequestToken::decodePayload(const Vector<char> &payload)
{
    bool result = read(payload, m_tokenId);
    result &= read(payload, m_sequenceId);

    return result;
}

void RequestToken::encodePayload(Vector<char> &payload) const
{
    write(payload, m_tokenId);
    write(payload, m_sequenceId);
}

const String &RequestToken::tokenId() const
{
    return m_tokenId;
}

void RequestToken::setTokenId(const String &tokenId)
{
    m_tokenId = tokenId;
}

u32 RequestToken::sequenceId() const
{
    return m_sequenceId;
}

void RequestToken::setSequenceId(u32 sequenceId)
{
    m_sequenceId = sequenceId;
}

GiveToken::GiveToken() :
    Packet(Packet::Type::GiveToken)
{

}

bool GiveToken::decodePayload(const Vector<char> &payload)
{
    bool result = read(payload, m_tokenId);
    result &= read(payload, m_data);

    return result;
}

void GiveToken::encodePayload(Vector<char> &payload) const
{
    write(payload, m_tokenId);
    write(payload, m_data);
}

const String &GiveToken::tokenId() const
{
    return m_tokenId;
}

void GiveToken::setTokenId(const String &tokenId)
{
    m_tokenId = tokenId;
}

const Vector<char> &GiveToken::data() const
{
    return m_data;
}

void GiveToken::setData(const Vector<char> &data)
{
    m_data = data;
}

Signal::Signal() :
    Packet(Packet::Type::Signal)
{

}

bool Signal::decodePayload(const Vector<char> &payload)
{
    bool result = read(payload, m_tokenId);
    result &= read(payload, m_variableId);

    return result;
}

void Signal::encodePayload(Vector<char> &payload) const
{
    write(payload, m_tokenId);
    write(payload, m_variableId);
}

const String &Signal::tokenId() const
{
    return m_tokenId;
}

void Signal::setTokenId(const String &tokenId)
{
    m_tokenId = tokenId;
}

const String &Signal::variableId() const
{
    return m_variableId;
}

void Signal::setVariableId(const String &variableId)
{
    m_variableId = variableId;
}



END_NAMESPACE END_NAMESPACE
