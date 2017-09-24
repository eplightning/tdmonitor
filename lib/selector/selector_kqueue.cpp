#include <selector/selector_kqueue.h>

#include <tdmonitor/types.h>
#include <tdmonitor/selector.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

TDM_NAMESPACE

SelectorApiKqueue::SelectorApiKqueue(int bufsize) :
    m_bufsize(bufsize), m_kqueuefd(kqueue())
{

}

SelectorApiKqueue::~SelectorApiKqueue()
{
    for (auto &kv : m_info)
        delete kv.second;

    close(m_kqueuefd);
}

void SelectorApiKqueue::add(int fd, int type, void *data, int eventType)
{
    // prevents leak in case of invalid calls
    auto it = m_info.find(fd);
    if (it != m_info.end())
        delete (*it).second;

    SelectorInfo *info = new SelectorInfo(fd, type, data, eventType);

    if (eventType & SelectorInfo::ReadEvent)
        appendChange(fd, EVFILT_READ, EV_ADD, 0, info);

    if (eventType & SelectorInfo::WriteEvent)
        appendChange(fd, EVFILT_WRITE, EV_ADD, 0, info);

    m_info[fd] = info;
}

void SelectorApiKqueue::close(int fd)
{
    m_closedFds.insert(fd);
    auto it = m_info.find(fd);
    if (it == m_info.end())
        return;

    delete (*it).second;
    m_info.erase(it);
}

void SelectorApiKqueue::modify(int fd, int eventType)
{
    auto it = m_info.find(fd);
    if (it == m_info.end())
        return;

    SelectorInfo *info = (*it).second;

    int oldEventType = info->eventType();

    if (info->eventType() & SelectorInfo::ReadEvent && !(eventType & SelectorInfo::ReadEvent)) {
        appendChange(fd, EVFILT_READ, EV_DELETE, 0, info);
        info->setEventType(oldEventType & ~(SelectorInfo::ReadEvent));
    } else if (!(info->eventType() & SelectorInfo::ReadEvent) && eventType & SelectorInfo::ReadEvent) {
        appendChange(fd, EVFILT_READ, EV_ADD, 0, info);
        info->setEventType(oldEventType | SelectorInfo::ReadEvent);
    }

    if (info->eventType() & SelectorInfo::WriteEvent && !(eventType & SelectorInfo::WriteEvent)) {
        appendChange(fd, EVFILT_WRITE, EV_DELETE, 0, info);
        info->setEventType(oldEventType & ~(SelectorInfo::WriteEvent));
    } else if (!(info->eventType() & SelectorInfo::WriteEvent) && eventType & SelectorInfo::WriteEvent) {
        appendChange(fd, EVFILT_WRITE, EV_ADD, 0, info);
        info->setEventType(oldEventType | SelectorInfo::WriteEvent);
    }
}

void SelectorApiKqueue::remove(int fd)
{
    auto it = m_info.find(fd);
    if (it == m_info.end())
        return;

    modify(fd, 0);

    delete (*it).second;
    m_info.erase(it);
}

bool SelectorApiKqueue::wait(Vector<SelectorEvent> &events)
{
    events.clear();

    struct kevent kevents[m_bufsize];
    struct kevent *changelist = 0;
    int nchanges = 0;

    Vector<struct kevent> tmpEvents;
    if (m_changes.size() > 0) {
        if (m_closedFds.size() > 0) {
            tmpEvents.reserve(m_changes.size());

            for (auto &x : m_changes) {
                if (m_closedFds.find(x.ident) == m_closedFds.end())
                    tmpEvents.push_back(x);
            }

            changelist = &(tmpEvents[0]);
            nchanges = tmpEvents.size();
        } else {
            changelist = &(m_changes[0]);
            nchanges = m_changes.size();
        }
    }

    // zmiany oraz eventy
    int nevents = kevent(m_kqueuefd, changelist, nchanges, kevents, m_bufsize, 0);
    if (nevents == -1)
        return false;

    m_changes.clear();
    m_closedFds.clear();

    for (int i = 0; i < nevents; i++) {
        if (kevents[i].flags & EV_ERROR)
            continue;

        SelectorInfo *info = static_cast<SelectorInfo*>(kevents[i].udata);

        if (kevents[i].filter == EVFILT_READ) {
            events.emplace_back(info, SelectorInfo::ReadEvent);
        } else if (kevents[i].filter == EVFILT_WRITE) {
            events.emplace_back(info, SelectorInfo::WriteEvent);
        }

        if (kevents[i].flags & EV_EOF && !info->closed())
            info->setClosed(true);
    }

    return true;
}

void SelectorApiKqueue::appendChange(int fd, int filter, int flags, int fflags, SelectorInfo *info)
{
    m_changes.emplace_back();
    struct kevent &kev = m_changes.back();
    EV_SET(&kev, fd, filter, flags, fflags, 0, info);
}

END_NAMESPACE
