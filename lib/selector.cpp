#include <tdmonitor/selector.h>

#include <tdmonitor/types.h>

#if defined(__APPLE__) && defined(__MACH__)
    #define SELECTOR_API_KQUEUE
#elif defined(__linux__)
    #define SELECTOR_API_EPOLL
#else
    #error "Unknown OS"
#endif

#if defined(SELECTOR_API_KQUEUE)
    #include "selector/selector_kqueue.h"
#elif defined(SELECTOR_API_EPOLL)
    #include "selector/selector_epoll.h"
#endif

TDM_NAMESPACE

const int SelectorInfo::ReadEvent = 1 << 0;
const int SelectorInfo::WriteEvent = 1 << 1;

Selector *Selector::factory()
{
#if defined(SELECTOR_API_KQUEUE)
    return new SelectorApiKqueue(32);
#elif defined(SELECTOR_API_EPOLL)
    return new SelectorApiEpoll(32);
#endif
}

Selector::~Selector()
{

}

SelectorEvent::SelectorEvent(const SelectorInfo *info, int type) :
    m_info(info), m_type(type)
{

}

const SelectorInfo *SelectorEvent::info() const
{
    return m_info;
}

int SelectorEvent::type() const
{
    return m_type;
}

SelectorInfo::SelectorInfo(int fd, int type, void *data, int evtype, bool timer) :
    m_fd(fd), m_type(type), m_data(data), m_eventType(evtype), m_closed(false), m_timer(timer)
{

}

int SelectorInfo::fd() const
{
    return m_fd;
}

int SelectorInfo::type() const
{
    return m_type;
}

void *SelectorInfo::data() const
{
    return m_data;
}

int SelectorInfo::eventType() const
{
    return m_eventType;
}

bool SelectorInfo::closed() const
{
    return m_closed;
}

bool SelectorInfo::timer() const
{
    return m_timer;
}

void SelectorInfo::setEventType(int value)
{
    m_eventType = value;
}

void SelectorInfo::setClosed(bool closed)
{
    m_closed = closed;
}

END_NAMESPACE
