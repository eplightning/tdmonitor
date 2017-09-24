#include <tdmonitor/marshaller.h>

#include <tdmonitor/types.h>

TDM_NAMESPACE

PropertyValue::PropertyValue(const SharedPtr<void> &ptr2) :
    isSharedPtr(true), ptr(ptr2)
{
}

PropertyValue::PropertyValue(u64 qword2) :
    isSharedPtr(false), qword(qword2)
{
}

PropertyValue::PropertyValue(u32 dword2) :
    isSharedPtr(false), dword(dword2)
{
}

PropertyValue::PropertyValue(u16 word2) :
    isSharedPtr(false), word(word2)
{
}

PropertyValue::PropertyValue(u8 byte2) :
    isSharedPtr(false), byte(byte2)
{
}

PropertyValue::PropertyValue(float singleFloat2) :
    isSharedPtr(false), singleFloat(singleFloat2)
{

}

PropertyValue::PropertyValue(double doubleFloat2) :
    isSharedPtr(false), doubleFloat(doubleFloat2)
{

}

PropertyValue::PropertyValue(const PropertyValue &copy)
{
    isSharedPtr = copy.isSharedPtr;

    if (isSharedPtr) {
        new (&ptr) auto(copy.ptr);
    } else {
        qword = copy.qword;
    }
}

PropertyValue::~PropertyValue()
{
    if (isSharedPtr)
        ptr.~shared_ptr();
}

DataMarshaller::DataMarshaller()
{

}

void DataMarshaller::registerType(const String &type, MarshallDelegate marshall, UnmarshallDelegate unmarshall)
{
    m_types[type] = { marshall, unmarshall };
}

Vector<char> DataMarshaller::marshallValue(const String &type, const PropertyValue &value)
{
    auto custom = m_types.find(type);

    if (custom == m_types.cend()) {
        if (type.at(0) == '$') {
            return marshallPrimitive(type, value);
        } else if (type.at(0) == '#') {
            return marshallPrimitiveVector(type, value);
        }

        // TODO: błąd
    }

    return custom->second.marshall(value);
}

PropertyValue DataMarshaller::unmarshallValue(const String &type, const Vector<char> &bytes)
{
    auto custom = m_types.find(type);

    if (custom == m_types.cend()) {
        if (type.at(0) == '$') {
            return unmarshallPrimitive(type, bytes);
        } else if (type.at(0) == '#') {
            return unmarshallPrimitiveVector(type, bytes);
        }

        // TODO: błąd
    }

    return custom->second.unmarshall(bytes);
}

Vector<char> DataMarshaller::marshallPrimitive(const String &type, const PropertyValue &value)
{
    MarshallHelper helper;

    if (type == "$u8" || type == "$s8") {
        helper.write(value.byte);
    } else if (type == "$u16" || type == "$s16") {
        helper.write(value.word);
    } else if (type == "$u32" || type == "$s32") {
        helper.write(value.dword);
    } else if (type == "$u64" || type == "$s64") {
        helper.write(value.qword);
    } else if (type == "$float") {
        helper.write(value.singleFloat);
    } else if (type == "$double") {
        helper.write(value.doubleFloat);
    }

    return helper.vector();
}

PropertyValue DataMarshaller::unmarshallPrimitive(const String &type, const Vector<char> &bytes)
{
    UnmarshallHelper helper(bytes);

    if (type == "$u8" || type == "$s8") {
        u8 data;
        helper.read(data);
        return PropertyValue(data);
    } else if (type == "$u16" || type == "$s16") {
        u16 data;
        helper.read(data);
        return PropertyValue(data);
    } else if (type == "$u32" || type == "$s32") {
        u32 data;
        helper.read(data);
        return PropertyValue(data);
    } else if (type == "$u64" || type == "$s64") {
        u64 data;
        helper.read(data);
        return PropertyValue(data);
    } else if (type == "$float") {
        float data;
        helper.read(data);
        return PropertyValue(data);
    } else if (type == "$double") {
        float data;
        helper.read(data);
        return PropertyValue(data);
    }

    return PropertyValue(static_cast<u64>(0));
}

Vector<char> DataMarshaller::marshallPrimitiveVector(const String &type, const PropertyValue &value)
{
    MarshallHelper helper;

    void *rawPtr = value.ptr.get();

    if (type == "#u8" || type == "#s8") {
        Vector<u8> *vectorPtr = reinterpret_cast<Vector<u8>*>(rawPtr);
        helper.write(static_cast<u32>(vectorPtr->size()));

        for (auto x : *vectorPtr)
            helper.write(x);
    } else if (type == "#u16" || type == "#s16") {
        Vector<u16> *vectorPtr = reinterpret_cast<Vector<u16>*>(rawPtr);
        helper.write(static_cast<u32>(vectorPtr->size()));

        for (auto x : *vectorPtr)
            helper.write(x);
    } else if (type == "#u32" || type == "#s32") {
        Vector<u32> *vectorPtr = reinterpret_cast<Vector<u32>*>(rawPtr);
        helper.write(static_cast<u32>(vectorPtr->size()));

        for (auto x : *vectorPtr)
            helper.write(x);
    } else if (type == "#u64" || type == "#s64") {
        Vector<u64> *vectorPtr = reinterpret_cast<Vector<u64>*>(rawPtr);
        helper.write(static_cast<u32>(vectorPtr->size()));

        for (auto x : *vectorPtr)
            helper.write(x);
    } else if (type == "#float") {
        Vector<float> *vectorPtr = reinterpret_cast<Vector<float>*>(rawPtr);
        helper.write(static_cast<u32>(vectorPtr->size()));

        for (auto x : *vectorPtr)
            helper.write(x);
    } else if (type == "#double") {
        Vector<double> *vectorPtr = reinterpret_cast<Vector<double>*>(rawPtr);
        helper.write(static_cast<u32>(vectorPtr->size()));

        for (auto x : *vectorPtr)
            helper.write(x);
    }

    return helper.vector();
}

PropertyValue DataMarshaller::unmarshallPrimitiveVector(const String &type, const Vector<char> &bytes)
{
    UnmarshallHelper helper(bytes);

    u32 size;
    helper.read(size);

    if (type == "#u8" || type == "#s8") {
        SharedPtr<Vector<u8>> vectorPtr(new Vector<u8>);
        vectorPtr->reserve(size);

        while (size > 0) {
            u8 data;
            helper.read(data);
            vectorPtr->push_back(data);
            --size;
        }

        return PropertyValue(vectorPtr);
    } else if (type == "#u16" || type == "#s16") {
        SharedPtr<Vector<u16>> vectorPtr(new Vector<u16>);
        vectorPtr->reserve(size);

        while (size > 0) {
            u16 data;
            helper.read(data);
            vectorPtr->push_back(data);
            --size;
        }

        return PropertyValue(vectorPtr);
    } else if (type == "#u32" || type == "#s32") {
        SharedPtr<Vector<u32>> vectorPtr(new Vector<u32>);
        vectorPtr->reserve(size);

        while (size > 0) {
            u32 data;
            helper.read(data);
            vectorPtr->push_back(data);
            --size;
        }

        return PropertyValue(vectorPtr);
    } else if (type == "#u64" || type == "#s64") {
        SharedPtr<Vector<u64>> vectorPtr(new Vector<u64>);
        vectorPtr->reserve(size);

        while (size > 0) {
            u64 data;
            helper.read(data);
            vectorPtr->push_back(data);
            --size;
        }

        return PropertyValue(vectorPtr);
    } else if (type == "#float") {
        SharedPtr<Vector<float>> vectorPtr(new Vector<float>);
        vectorPtr->reserve(size);

        while (size > 0) {
            float data;
            helper.read(data);
            vectorPtr->push_back(data);
            --size;
        }

        return PropertyValue(vectorPtr);
    } else if (type == "#double") {
        SharedPtr<Vector<double>> vectorPtr(new Vector<double>);
        vectorPtr->reserve(size);

        while (size > 0) {
            double data;
            helper.read(data);
            vectorPtr->push_back(data);
            --size;
        }

        return PropertyValue(vectorPtr);
    }

    return PropertyValue(static_cast<u64>(0));
}

Vector<char> DataMarshaller::marshallMap(const PropertyMap &map)
{
    MarshallHelper helper;

    helper.write(map.size());

    for (auto &x : map) {
        helper.write(x.second.id);
        helper.write(x.second.type);
        helper.write(marshallValue(x.second.type, x.second.value));
    }

    return helper.vector();
}

PropertyMap DataMarshaller::unmarshallMap(const Vector<char> &bytes)
{
    UnmarshallHelper helper(bytes);
    PropertyMap output;

    u32 propertiesNum;
    helper.read(propertiesNum);

    while (propertiesNum > 0) {
        String id;
        String type;
        Vector<char> rawValue;

        helper.read(id);
        helper.read(type);
        helper.read(rawValue);

        PropertyValue value = unmarshallValue(type, rawValue);

        output.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(id, type, value));

        --propertiesNum;
    }

    return output;
}

MarshallHelper::MarshallHelper() :
    m_vector()
{

}

Vector<char> MarshallHelper::vector()
{
    return m_vector;
}

void MarshallHelper::write(u8 data)
{
    m_vector.push_back(data);
}

void MarshallHelper::write(s8 data)
{
    m_vector.push_back(data);
}

void MarshallHelper::write(u16 data)
{
    data = htons(data);
    char *raw = reinterpret_cast<char*>(&data);
    m_vector.insert(m_vector.end(), raw, raw + 2);
}

void MarshallHelper::write(s16 data)
{
    data = htons(data);
    char *raw = reinterpret_cast<char*>(&data);
    m_vector.insert(m_vector.end(), raw, raw + 2);
}

void MarshallHelper::write(u32 data)
{
    data = htonl(data);
    char *raw = reinterpret_cast<char*>(&data);
    m_vector.insert(m_vector.end(), raw, raw + 4);
}

void MarshallHelper::write(s32 data)
{
    data = htonl(data);
    char *raw = reinterpret_cast<char*>(&data);
    m_vector.insert(m_vector.end(), raw, raw + 4);
}

void MarshallHelper::write(u64 data)
{
    data = hostToNetwork(data);
    char *raw = reinterpret_cast<char*>(&data);
    m_vector.insert(m_vector.end(), raw, raw + 8);
}

void MarshallHelper::write(s64 data)
{
    data = hostToNetwork(data);
    char *raw = reinterpret_cast<char*>(&data);
    m_vector.insert(m_vector.end(), raw, raw + 8);
}

void MarshallHelper::write(float data)
{
    write(*(reinterpret_cast<u32*>(&data)));
}

void MarshallHelper::write(double data)
{
    write(*(reinterpret_cast<u64*>(&data)));
}

void MarshallHelper::write(const String &data)
{
    write(static_cast<u32>(data.size()));
    m_vector.insert(m_vector.end(), data.cbegin(), data.cend());
}

void MarshallHelper::write(const Vector<char> &data)
{
    write(static_cast<u32>(data.size()));
    m_vector.insert(m_vector.end(), data.cbegin(), data.cend());
}

void MarshallHelper::writeVectorSize(size_t data)
{
    write(static_cast<u32>(data));
}

u64 MarshallHelper::hostToNetwork(u64 data) const
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u32 high = htonl(static_cast<u32>(data >> 32));
    u32 low = htonl(static_cast<u32>(data & 0xFFFFFFFFLL));

    return (static_cast<u64>(low) << 32) | high;
#else
    return data;
#endif
}

s64 MarshallHelper::hostToNetwork(s64 data) const
{
    return static_cast<s64>(hostToNetwork(static_cast<u64>(data)));
}

UnmarshallHelper::UnmarshallHelper(const Vector<char> &bytes) :
    m_bytes(bytes), m_index(0)
{

}

int UnmarshallHelper::index() const
{
    return m_index;
}

bool UnmarshallHelper::read(u8 &data)
{
    if (m_bytes.size() < 1 + m_index)
        return false;

    data = static_cast<u8>(m_bytes[m_index]);
    m_index += 1;
    return true;
}

bool UnmarshallHelper::read(s8 &data)
{
    if (m_bytes.size() < 1 + m_index)
        return false;

    data = static_cast<s8>(m_bytes[m_index]);
    m_index += 1;
    return true;
}

bool UnmarshallHelper::read(s16 &data)
{
    if (m_bytes.size() < 2 + m_index)
        return false;

    data = ntohs(*(reinterpret_cast<const s16*>(&m_bytes[m_index])));
    m_index += 2;
    return true;
}

bool UnmarshallHelper::read(u16 &data)
{
    if (m_bytes.size() < 2 + m_index)
        return false;

    data = ntohs(*(reinterpret_cast<const u16*>(&m_bytes[m_index])));
    m_index += 2;
    return true;
}

bool UnmarshallHelper::read(s32 &data)
{
    if (m_bytes.size() < 4 + m_index)
        return false;

    data = ntohl(*(reinterpret_cast<const s32*>(&m_bytes[m_index])));
    m_index += 4;
    return true;
}

bool UnmarshallHelper::read(u32 &data)
{
    if (m_bytes.size() < 4 + m_index)
        return false;

    data = ntohl(*(reinterpret_cast<const u32*>(&m_bytes[m_index])));
    m_index += 4;
    return true;
}

bool UnmarshallHelper::read(s64 &data)
{
    if (m_bytes.size() < 8 + m_index)
        return false;

    data = networkToHost(*(reinterpret_cast<const s64*>(&m_bytes[m_index])));
    m_index += 8;
    return true;
}

bool UnmarshallHelper::read(u64 &data)
{
    if (m_bytes.size() < 8 + m_index)
        return false;

    data = networkToHost(*(reinterpret_cast<const u64*>(&m_bytes[m_index])));
    m_index += 8;
    return true;
}

bool UnmarshallHelper::read(float &data)
{
    return read(reinterpret_cast<u32&>(data));
}

bool UnmarshallHelper::read(double &data)
{
    return read(reinterpret_cast<u64&>(data));
}

bool UnmarshallHelper::read(String &data)
{
    u32 len;
    if (!read(len) || m_bytes.size() < len + m_index)
        return false;

    data.reserve(len);
    data.assign(m_bytes.cbegin() + m_index, m_bytes.cbegin() + m_index + len);
    m_index += len;
    return true;
}

bool UnmarshallHelper::read(Vector<char> &data)
{
    u32 len;
    if (!read(len) || m_bytes.size() < len + m_index)
        return false;

    data.reserve(len);
    data.assign(m_bytes.cbegin() + m_index, m_bytes.cbegin() + m_index + len);
    m_index += len;
    return true;
}

bool UnmarshallHelper::readVectorSize(u32 &data)
{
    return read(data);
}

u64 UnmarshallHelper::networkToHost(u64 data) const
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u32 high = ntohl(static_cast<u32>(data >> 32));
    u32 low = ntohl(static_cast<u32>(data & 0xFFFFFFFFLL));

    return (static_cast<u64>(low) << 32) | high;
#else
    return data;
#endif
}

s64 UnmarshallHelper::networkToHost(s64 data) const
{
    return static_cast<s64>(networkToHost(static_cast<u64>(data)));
}

Property::Property(const String &id2, const String &type2, const PropertyValue &value2) :
    id(id2), type(type2), value(value2)
{

}

END_NAMESPACE


