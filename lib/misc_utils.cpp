#include <tdmonitor/misc_utils.h>

#include <tdmonitor/types.h>

#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

TDM_NAMESPACE

void MiscUtils::blockSignals()
{
    sigset_t set;

    sigfillset(&set);
    pthread_sigmask(SIG_SETMASK, &set, NULL);
}

void MiscUtils::unblockSignals()
{
    sigset_t set;

    sigemptyset(&set);
    pthread_sigmask(SIG_SETMASK, &set, NULL);
}

String MiscUtils::systemError(int num)
{
    // YAIC custom errors
    switch (num) {
    case EYAICDISCONNECT: return "Requested disconnect";
    case EYAICINVPACKET: return "Invalid packet";
    }

    // Unix errors
    if (num && num != EAGAIN && num != EWOULDBLOCK) {
        char buf[256]{};

#ifdef _GNU_SOURCE
        char *str = strerror_r(num, buf, sizeof(buf));
        return str;
#else
        if (strerror_r(num, buf, sizeof(buf)) != -1)
            return buf;
#endif
    }

    return "";
}

END_NAMESPACE
