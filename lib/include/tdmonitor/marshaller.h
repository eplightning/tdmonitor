#pragma once

#include <tdmonitor/types.h>

#include <arpa/inet.h>
#include <functional>

TDM_NAMESPACE

struct PropertyValue {
    bool isSharedPtr;
    union {
        u8 byte;
        u16 word;
        u32 dword;
        u64 qword;
        float singleFloat;
        double doubleFloat;
        SharedPtr<void> ptr;
    };

    PropertyValue(const SharedPtr<void> &ptr2);
    PropertyValue(u64 qword);
    PropertyValue(u32 dword);
    PropertyValue(u16 word);
    PropertyValue(u8 byte);
    PropertyValue(float singleFloat);
    PropertyValue(double doubleFloat);
    PropertyValue(const PropertyValue &copy);
    ~PropertyValue();
};

struct Property {
    String id;
    String type;
    PropertyValue value;

    Property(const String &id, const String &type, const PropertyValue &value);
};

typedef HashMap<String, Property> PropertyMap;
typedef std::function<Vector<char>(const PropertyValue&)> MarshallDelegate;
typedef std::function<PropertyValue(const Vector<char>&)> UnmarshallDelegate;

struct MarshallerTypeDefinition {
    MarshallDelegate marshall;
    UnmarshallDelegate unmarshall;
};

class MarshallHelper {
public:
    MarshallHelper();
    Vector<char> vector();

    void write(u8 data);
    void write(s8 data);
    void write(u16 data);
    void write(s16 data);
    void write(u32 data);
    void write(s32 data);
    void write(u64 data);
    void write(s64 data);
    void write(float data);
    void write(double data);
    void write(const String &data);
    void write(const Vector<char> &data);
    void writeVectorSize(size_t data);

    u64 hostToNetwork(u64 data) const;
    s64 hostToNetwork(s64 data) const;

protected:
    Vector<char> m_vector;
};

class UnmarshallHelper {
public:
    UnmarshallHelper(const Vector<char> &bytes);

    int index() const;

    bool read(u8 &data);
    bool read(s8 &data);
    bool read(s16 &data);
    bool read(u16 &data);
    bool read(s32 &data);
    bool read(u32 &data);
    bool read(s64 &data);
    bool read(u64 &data);
    bool read(float &data);
    bool read(double &data);
    bool read(String &data);
    bool read(Vector<char> &data);
    bool readVectorSize(u32 &data);

    u64 networkToHost(u64 data) const;
    s64 networkToHost(s64 data) const;

protected:
    const Vector<char> &m_bytes;
    uint m_index;
};

class DataMarshaller {
public:
    DataMarshaller();

    void registerType(const String& type, MarshallDelegate marshall, UnmarshallDelegate unmarshall);

    Vector<char> marshallValue(const String &type, const PropertyValue &value);
    PropertyValue unmarshallValue(const String &type, const Vector<char> &bytes);
    Vector<char> marshallPrimitive(const String &type, const PropertyValue &value);
    PropertyValue unmarshallPrimitive(const String &type, const Vector<char> &bytes);
    Vector<char> marshallPrimitiveVector(const String &type, const PropertyValue &value);
    PropertyValue unmarshallPrimitiveVector(const String &type, const Vector<char> &bytes);

    Vector<char> marshallMap(const PropertyMap &map);
    PropertyMap unmarshallMap(const Vector<char> &bytes);

protected:
    HashMap<String, MarshallerTypeDefinition> m_types;
};

END_NAMESPACE
