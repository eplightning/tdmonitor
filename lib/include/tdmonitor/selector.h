#pragma once

#include <tdmonitor/types.h>

TDM_NAMESPACE

/**
 * @brief Klasa reprezentująca obserwowany deskryptor
 */
class SelectorInfo {
public:
    const static int ReadEvent;
    const static int WriteEvent;

    SelectorInfo(int fd, int type, void *data, int evtype, bool timer);

    int fd() const;
    int type() const;
    void *data() const;
    int eventType() const;
    bool closed() const;
    bool timer() const;

    void setEventType(int value);
    void setClosed(bool closed);

protected:
    int m_fd;
    int m_type;
    void *m_data;
    int m_eventType;
    bool m_closed;
    bool m_timer;
};

/**
 * @brief Klasa reprezentująca wydarzenie deskryptora
 */
class SelectorEvent {
public:
    SelectorEvent(const SelectorInfo *info, int type);

    const SelectorInfo *info() const;
    int type() const;

protected:
    const SelectorInfo *m_info;
    int m_type;
};

/**
 * @brief Abstrakcja na selektory
 */
class Selector {
public:
    static Selector *factory();

    virtual ~Selector();

    virtual void add(int fd, int type, void *data, int eventType) = 0;
    virtual int addTimer(int type, void *data, int seconds) = 0;
    virtual void close(int fd) = 0;
    virtual void modify(int fd, int eventType) = 0;
    virtual void remove(int fd) = 0;
    virtual bool wait(Vector<SelectorEvent> &events) = 0;
};

END_NAMESPACE
