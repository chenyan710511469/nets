/*
 * =====================================================================================
 *
 *       Filename:  TcpIoSocket.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月05日 15时19分53秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef TCP_SERVER_TCPIOSOCKET_H
#define TCP_SERVER_TCPIOSOCKET_H

//#include <stropts.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>
#include <linux/if_arp.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include <stdexcept>
//#include <thread>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <chrono>
//#include <mutex>
//#include <condition_variable>
//#include <atomic>
#include <vector>
#include <list>
#include <deque>
#include <cstddef>
#include <memory>
#include <new>

#include "TcpIoRequest.h"
#include "TcpIoSchedule.h"

using namespace std;

namespace tcpserver
{
#define ifreq_length    10
class TcpIoRequest;
class TcpIoSchedule;

class TcpIoSocket
{
public:
    int descriptor();
    void close();
    bool setSendBlocking(bool isBlocking, int timeOut);

    TcpIoSocket(const TcpIoSocket & other) = delete;
    TcpIoSocket & operator=(const TcpIoSocket & other) = delete;

private:
    explicit TcpIoSocket(TcpIoSchedule * sched, int fd);
    explicit TcpIoSocket(TcpIoSchedule * sched, int family, int type, int protocol);
    ~TcpIoSocket();

    friend class TcpIoRequest;
    friend class TcpIoScheduleImpl;

    bool send(TcpIoRequest * request);

    TcpIoSchedule::ReturnType recv(TcpIoRequest * request);

    bool accept(TcpIoRequest * request);

    bool setIsBlocking(bool isBlocking);

    void setSocketOpt();

    /**
     * isBlocking is true or false,
     * true, blocking, timeOut is millisecond.
     * false is non-blocking, timeOut is invalid.
     * */
    bool setRecvBlocking(bool isBlocking, int timeOut);

    bool handleError(int theErrno);

    bool requestRecv();
    void releaseRecv();

    bool getPeerAddressAndPort(TcpIoRequest * request);
    void initIfconf();

private:
    TcpIoSchedule           * mSched;
    int                     mFd;

    pthread_mutex_t         mAcceptMutex;

    pthread_mutex_t         mRecvMutex;
    pthread_cond_t          mRecvCond;
    bool                    mRecving;

    struct ifreq            mIfreq[ifreq_length];
    struct ifconf           mIfconf;
};
};  // tcpserver
#endif // TCP_SERVER_TCPIOSOCKET_H

