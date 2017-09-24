#include <tdmonitor/monitor.h>

#include <tdmonitor/marshaller.h>
#include <tdmonitor/types.h>

TDM_NAMESPACE

void Monitor::property(const String &id, u8 &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$u8", &data));
}

void Monitor::property(const String &id, s8 &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$s8", &data));
}

void Monitor::property(const String &id, u16 &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$u16", &data));
}

void Monitor::property(const String &id, s16 &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$s16", &data));
}

void Monitor::property(const String &id, u32 &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$u32", &data));
}

void Monitor::property(const String &id, s32 &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$s32", &data));
}

void Monitor::property(const String &id, u64 &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$u64", &data));
}

void Monitor::property(const String &id, s64 &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$s64", &data));
}

void Monitor::property(const String &id, float &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$float", &data));
}

void Monitor::property(const String &id, double &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("$double", &data));
}

void Monitor::property(const String &id, Vector<u8> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#u8", &data));
}

void Monitor::property(const String &id, Vector<s8> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#s8", &data));
}

void Monitor::property(const String &id, Vector<u16> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#u16", &data));
}

void Monitor::property(const String &id, Vector<s16> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#s16", &data));
}

void Monitor::property(const String &id, Vector<u32> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#u32", &data));
}

void Monitor::property(const String &id, Vector<s32> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#s32", &data));
}

void Monitor::property(const String &id, Vector<u64> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#u64", &data));
}

void Monitor::property(const String &id, Vector<s64> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#s64", &data));
}

void Monitor::property(const String &id, Vector<float> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#float", &data));
}

void Monitor::property(const String &id, Vector<double> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple("#double", &data));
}

void Monitor::property(const String &id, const String &type, SharedPtr<void> &data)
{
    m_properties.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),
                         std::forward_as_tuple(type, &data));
}

void Monitor::loadProperties(const PropertyMap &properties)
{
    for (auto &x: m_properties) {
        auto match = properties.find(x.first);

        if (match == properties.cend()) {
            continue;
        }

        const Property &prop = match->second;

        if (prop.type.at(0) == '$') {
            if (prop.type == "$u8" || prop.type == "$s8") {
                *(reinterpret_cast<u8*>(x.second.ptr)) = prop.value.byte;
            } else if (prop.type == "$u16" || prop.type == "$s16") {
                *(reinterpret_cast<u16*>(x.second.ptr)) = prop.value.word;
            } else if (prop.type == "$u32" || prop.type == "$s32") {
                *(reinterpret_cast<u32*>(x.second.ptr)) = prop.value.dword;
            } else if (prop.type == "$u64" || prop.type == "$s64") {
                *(reinterpret_cast<u64*>(x.second.ptr)) = prop.value.qword;
            } else if (prop.type == "$float") {
                *(reinterpret_cast<float*>(x.second.ptr)) = prop.value.singleFloat;
            } else if (prop.type == "$double") {
                *(reinterpret_cast<double*>(x.second.ptr)) = prop.value.doubleFloat;
            }
        } else if (prop.type.at(0) == '#') {
            if (prop.type == "$u8" || prop.type == "$s8") {
                Vector<u8> *vector = reinterpret_cast<Vector<u8>*>(x.second.ptr);
                Vector<u8> *vectorRead = reinterpret_cast<Vector<u8>*>(prop.value.ptr.get());
                *vector = *vectorRead;
            } else if (prop.type == "$u16" || prop.type == "$s16") {
                Vector<u16> *vector = reinterpret_cast<Vector<u16>*>(x.second.ptr);
                Vector<u16> *vectorRead = reinterpret_cast<Vector<u16>*>(prop.value.ptr.get());
                *vector = *vectorRead;
            } else if (prop.type == "$u32" || prop.type == "$s32") {
                Vector<u32> *vector = reinterpret_cast<Vector<u32>*>(x.second.ptr);
                Vector<u32> *vectorRead = reinterpret_cast<Vector<u32>*>(prop.value.ptr.get());
                *vector = *vectorRead;
            } else if (prop.type == "$s64" || prop.type == "$u64") {
                Vector<u64> *vector = reinterpret_cast<Vector<u64>*>(x.second.ptr);
                Vector<u64> *vectorRead = reinterpret_cast<Vector<u64>*>(prop.value.ptr.get());
                *vector = *vectorRead;
            } else if (prop.type == "$float") {
                Vector<float> *vector = reinterpret_cast<Vector<float>*>(x.second.ptr);
                Vector<float> *vectorRead = reinterpret_cast<Vector<float>*>(prop.value.ptr.get());
                *vector = *vectorRead;
            } else if (prop.type == "$double") {
                Vector<double> *vector = reinterpret_cast<Vector<double>*>(x.second.ptr);
                Vector<double> *vectorRead = reinterpret_cast<Vector<double>*>(prop.value.ptr.get());
                *vector = *vectorRead;
            }
        } else {
            SharedPtr<void> *ptr = reinterpret_cast<SharedPtr<void>*>(x.second.ptr);
            *ptr = prop.value.ptr;
        }
    }
}

void Monitor::saveProperties(PropertyMap &properties) const
{
    for (auto &x: m_properties) {
        PropertyValue *val = nullptr;

        if (x.second.type.at(0) == '$') {
            if (x.second.type == "$s8" || x.second.type == "$u8") {
                val = new PropertyValue(*reinterpret_cast<u8*>(x.second.ptr));
            } else if (x.second.type == "$s16" || x.second.type == "$u16") {
                val = new PropertyValue(*reinterpret_cast<u16*>(x.second.ptr));
            } else if (x.second.type == "$s32" || x.second.type == "$u32") {
                val = new PropertyValue(*reinterpret_cast<u32*>(x.second.ptr));
            } else if (x.second.type == "$s64" || x.second.type == "$u64") {
                val = new PropertyValue(*reinterpret_cast<u64*>(x.second.ptr));
            } else if (x.second.type == "$float") {
                val = new PropertyValue(*reinterpret_cast<float*>(x.second.ptr));
            } else if (x.second.type == "$double") {
                val = new PropertyValue(*reinterpret_cast<double*>(x.second.ptr));
            }
        } else if (x.second.type.at(0) == '#') {
            if (x.second.type == "#s8" || x.second.type == "#u8") {
                Vector<u8> *vector = reinterpret_cast<Vector<u8>*>(x.second.ptr);
                SharedPtr<Vector<u8>> ptr(new Vector<u8>(*vector));
                val = new PropertyValue(ptr);
            } else if (x.second.type == "#s16" || x.second.type == "#u16") {
                Vector<u16> *vector = reinterpret_cast<Vector<u16>*>(x.second.ptr);
                SharedPtr<Vector<u16>> ptr(new Vector<u16>(*vector));
                val = new PropertyValue(ptr);
            } else if (x.second.type == "#s32" || x.second.type == "#u32") {
                Vector<u32> *vector = reinterpret_cast<Vector<u32>*>(x.second.ptr);
                SharedPtr<Vector<u32>> ptr(new Vector<u32>(*vector));
                val = new PropertyValue(ptr);
            } else if (x.second.type == "#s64" || x.second.type == "#u64") {
                Vector<u64> *vector = reinterpret_cast<Vector<u64>*>(x.second.ptr);
                SharedPtr<Vector<u64>> ptr(new Vector<u64>(*vector));
                val = new PropertyValue(ptr);
            } else if (x.second.type == "#float") {
                Vector<float> *vector = reinterpret_cast<Vector<float>*>(x.second.ptr);
                SharedPtr<Vector<float>> ptr(new Vector<float>(*vector));
                val = new PropertyValue(ptr);
            } else if (x.second.type == "#double") {
                Vector<double> *vector = reinterpret_cast<Vector<double>*>(x.second.ptr);
                SharedPtr<Vector<double>> ptr(new Vector<double>(*vector));
                val = new PropertyValue(ptr);
            }
        } else {
            SharedPtr<void> *ptr = reinterpret_cast<SharedPtr<void>*>(x.second.ptr);
            val = new PropertyValue(*ptr);
        }

        properties.emplace(std::piecewise_construct,
                           std::forward_as_tuple(x.first),
                           std::forward_as_tuple(x.first, x.second.type, *val));
    }
}

PropertyBinding::PropertyBinding(const String &type2, void *ptr2) :
    type(type2), ptr(ptr2)
{

}

END_NAMESPACE
