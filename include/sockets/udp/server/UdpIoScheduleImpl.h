/*
 * =====================================================================================
 *
 *       Filename:  UdpIoScheduleImpl.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2019年01月20日 20时07分44秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef UDP_SERVER_UDPIOSCHEDULEIMPL_H
#define UDP_SERVER_UDPIOSCHEDULEIMPL_H

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
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdexcept>
#include <time.h>
#include <thread>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <chrono>
#include <vector>
#include <list>
#include <deque>
#include <cstddef>
#include <memory>
#include <map>
#include <new>

#include "UdpIoRequest.h"
#include "UdpIoSocket.h"
#include "UdpIoSchedule.h"
#include "UdpConfig.h"


using namespace std;

namespace udpserver
{
class UdpIoScheduleImpl : public UdpIoSchedule
{
public:
    UdpIoScheduleImpl(
            bool (* callbackRecvfrom)(UdpIoRequest *request, void *owner),
            bool (* callbackRelease)(UdpIoRequest *request, void *owner),
            void *owner);
    ~UdpIoScheduleImpl();
    UdpIoScheduleImpl(const UdpIoScheduleImpl & other) = delete;
    UdpIoScheduleImpl & operator=(const UdpIoScheduleImpl & other) = delete;

private:
    bool start(UdpIoRequest *request, int family = AF_INET, int type = SOCK_DGRAM, int protocol = 0);
    bool stop(UdpIoRequest *request);
    bool stopAll();

    bool close(UdpIoRequest *request);

    bool post(UdpIoRequest * request);

    UdpIoRequest * allocRequest();

    UdpIoSocket * associate(int socketFd);

    void recvThread();
    void epollThread();

    bool removeRequest(int clientfd = 0);
    bool sendingOrRecving(UdpIoRequest *request);
    void updateHeartbeatPacket(UdpIoRequest *request);
    void setHeartbeatPacketIntervalTime(UdpIoRequest *request, int heartbeatPacketIntervalTime);
    bool recvfromCallback(UdpIoRequest *request);
    void freeRequest(UdpIoRequest * request);
    bool isReleasing(UdpIoRequest * request);

private:
    std::map<int, UdpIoRequest *>       mMapUdpIoRequest;
    std::vector<UdpIoRequest *>         mVecReleaseRequest;
    std::vector<UdpIoRequest *>         mDequeRecvUdpIoRequest;
    pthread_mutex_t                     mDequeMutex;
    int                                 mEpollFd;
    UdpIoSchedule::Status               mStatus;
    pthread_mutex_t                     mRequestMutex;

    std::thread                         mEpollThread;
    std::thread                         mRecvThreads[UdpConfig::mRecvThreadNumber];
    pthread_mutex_t                     mRecvMutex;
    pthread_cond_t                      mRecvCond;
    pthread_mutex_t                     mReleaseMutex;
    bool                                (* mRecvfromCallback)(UdpIoRequest * request, void * owner);
    bool                                (* mReleaseCallback)(UdpIoRequest * request, void * owner);
    void                                * mOwner;

};  // UdpIoScheduleImpl
};  // udpserver

#endif // UDP_SERVER_UDPIOSCHEDULEIMPL_H
