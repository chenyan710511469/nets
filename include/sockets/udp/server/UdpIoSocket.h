/*
 * =====================================================================================
 *
 *       Filename:  UdpIoSocket.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2019年01月26日 11时01分49秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef UDP_SERVER_UDPIOSOCKET_H
#define UDP_SERVER_UDPIOSOCKET_H
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

#include "UdpIoSocket.h"
#include "UdpIoSchedule.h"
#include "UdpIoScheduleImpl.h"

namespace udpserver
{
#define ifreq_length   10
class UdpIoSocket
{
public:
    int descriptor();
    void close();

    UdpIoSocket(const UdpIoSocket & other) = delete;
    UdpIoSocket & operator=(const UdpIoSocket & other) = delete;

private:
    friend class UdpIoRequest;
    friend class UdpIoScheduleImpl;

    UdpIoSocket(UdpIoSchedule * sched, int fd);
    ~UdpIoSocket();
    bool recvfrom(UdpIoRequest * request);
    bool sendto(UdpIoRequest * request);

    void initIfconf();
    void getSelfAddress(UdpIoRequest * request);
    bool getPeerAddressAndPort(UdpIoRequest * request);
    bool setIsBlocking(bool isBlocking);
    bool requestRecv();
    void releaseRecv();
    void notifyRelease();
    bool handleError(int theErrno);
private:
    int                     mFd;
    UdpIoSchedule           *mSched;

    bool                    mSending;

    pthread_mutex_t         mRecvMutex;
    pthread_cond_t          mRecvCond;
    bool                    mRecving;

    bool                    mReleasing;

    struct ifreq            mIfreq[ifreq_length];
    struct ifconf           mIfconf;
};
};
#endif // UDP_SERVER_UDPIOSOCKET_H
