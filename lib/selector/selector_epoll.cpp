#include <selector/selector_epoll.h>

#include <tdmonitor/types.h>
#include <tdmonitor/selector.h>

#include <stdlib.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/epoll.h>

TDM_NAMESPACE

SelectorApiEpoll::SelectorApiEpoll(int bufsize) :
    m_bufsize(bufsize)
{
    m_epollfd = epoll_create1(0);

    if (m_epollfd == -1)
        abort();
}

SelectorApiEpoll::~SelectorApiEpoll()
{
    for (auto &kv : m_info)
        delete kv.second;

    for (auto timer : m_timers)
        ::close(timer);

    ::close(m_epollfd);
}

void SelectorApiEpoll::add(int fd, int type, void *data, int eventType)
{
    // prevents leak in case of invalid calls
    auto it = m_info.find(fd);
    if (it != m_info.end())
        delete (*it).second;

    SelectorInfo *info = new SelectorInfo(fd, type, data, eventType, false);

    addInfo(info);
}

int SelectorApiEpoll::addTimer(int type, void *data, int seconds)
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);

    if (timerfd == -1)
        return -1;

    m_timers.push_back(timerfd);

    struct itimerspec itime {};
    itime.it_value.tv_sec = seconds;
    itime.it_interval.tv_sec = seconds;

    timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &itime, NULL);

    SelectorInfo *info = new SelectorInfo(timerfd, type, data, SelectorInfo::ReadEvent, true);

    addInfo(info);

    return timerfd;
}

void SelectorApiEpoll::close(int fd)
{
    auto it = m_info.find(fd);
    if (it == m_info.end())
        return;

    delete (*it).second;
    m_info.erase(it);
}

void SelectorApiEpoll::modify(int fd, int eventType)
{
    auto it = m_info.find(fd);
    if (it == m_info.end())
        return;

    SelectorInfo *info = (*it).second;
    info->setEventType(eventType);

    struct epoll_event event {};
    event.data.fd = fd;
    event.events = EPOLLRDHUP;

    if (eventType & SelectorInfo::ReadEvent)
        event.events |= EPOLLIN;
    if (eventType & SelectorInfo::WriteEvent)
        event.events |= EPOLLOUT;

    epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &event);
}

void SelectorApiEpoll::remove(int fd)
{
    auto it = m_info.find(fd);
    if (it == m_info.end())
        return;

    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, NULL);

    delete (*it).second;
    m_info.erase(it);
}

bool SelectorApiEpoll::wait(Vector<SelectorEvent> &events)
{
    events.clear();

    struct epoll_event returnedEvents[m_bufsize];
    int nevents = epoll_wait(m_epollfd, returnedEvents, m_bufsize, -1);
    if (nevents == -1)
        return false;

    for (int i = 0; i < nevents; i++)
    {
        auto it = m_info.find(returnedEvents[i].data.fd);
        if (it == m_info.end())
            continue;

        SelectorInfo *info = (*it).second;

        if (returnedEvents[i].events & EPOLLOUT)
            events.emplace_back(info, SelectorInfo::WriteEvent);

        if ((returnedEvents[i].events & EPOLLIN) || (returnedEvents[i].events & EPOLLERR)
                || (returnedEvents[i].events & EPOLLHUP) || (returnedEvents[i].events & EPOLLRDHUP))
            events.emplace_back(info, SelectorInfo::ReadEvent);

        if ((returnedEvents[i].events & EPOLLERR) || (returnedEvents[i].events & EPOLLHUP)
                || (returnedEvents[i].events & EPOLLRDHUP))
            info->setClosed(true);

        if ((returnedEvents[i].events & EPOLLIN) && info->timer()) {
            uint64_t expirations;
            if (read(info->fd(), &expirations, sizeof(expirations)) != sizeof(expirations))
                abort();
        }
    }

    return true;
}

void SelectorApiEpoll::addInfo(SelectorInfo *info)
{
    struct epoll_event event {};
    event.data.fd = info->fd();
    event.events = EPOLLRDHUP;

    if (info->eventType() & SelectorInfo::ReadEvent)
        event.events |= EPOLLIN;
    if (info->eventType() & SelectorInfo::WriteEvent)
        event.events |= EPOLLOUT;

    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, info->fd(), &event);

    m_info[info->fd()] = info;
}

END_NAMESPACE

