#pragma once

#include <tdmonitor/types.h>
#include <tdmonitor/selector.h>

#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/time.h>

TDM_NAMESPACE

/**
 * @brief Implementacja selektora na Linux'ie korzystająca z epolla
 */
class SelectorApiEpoll : public Selector {
public:
    explicit SelectorApiEpoll(int bufsize);
    ~SelectorApiEpoll();

    void add(int fd, int type, void *data, int eventType);
    int addTimer(int type, void *data, int seconds);
    void close(int fd);
    void modify(int fd, int eventType);
    void remove(int fd);
    bool wait(Vector<SelectorEvent> &events);

protected:
    void addInfo(SelectorInfo *info);

    int m_bufsize;
    int m_epollfd;
    HashMap<int, SelectorInfo*> m_info;
    Vector<int> m_timers;
};

END_NAMESPACE
