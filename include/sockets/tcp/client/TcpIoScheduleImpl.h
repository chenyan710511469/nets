/*
 * =====================================================================================
 *
 *       Filename:  TcpIoScheduleImpl.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月05日 15时47分42秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef TCP_CLIENT_TCPIOSCHEDULEIMPL_H
#define TCP_CLIENT_TCPIOSCHEDULEIMPL_H

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
#include <mutex>
//#include <condition_variable>
//#include <atomic>
#include <vector>
#include <list>
#include <deque>
#include <cstddef>
#include <memory>
#include <map>
#include <new>

#include <pthread.h>

#include "TcpIoRequest.h"
#include "TcpIoSocket.h"
#include "TcpIoSchedule.h"
#include "TcpConfig.h"

using namespace std;

namespace tcpclient
{
class TcpIoScheduleImpl : public TcpIoSchedule
{
public:
    TcpIoScheduleImpl(
            bool (* recvCallback)(TcpIoRequest * request, void * owner),
            bool (* connectCallback)(TcpIoRequest * request, bool isConnected, void * owner),
            bool (* releaseCb)(TcpIoRequest * request, void * owner),
            void * owner
        );
    ~TcpIoScheduleImpl();

private:
    bool preStart(int recvThreadNumber = TWO);
    bool start();

    bool stop(TcpIoRequest * request);
    bool stopAll();
    bool close(TcpIoRequest * request);

    bool post(TcpIoRequest * request);

    TcpIoRequest * allocRequest(int family);

    TcpIoSocket * associate(int socketFd);

    TcpIoScheduleImpl::Status getStatus();

    void setBlocking(bool isBlocking = false, int timeout = 0);

    TcpIoScheduleImpl(const TcpIoScheduleImpl & other) = delete;
    TcpIoScheduleImpl & operator=(const TcpIoScheduleImpl & other) = delete;

    TcpIoSocket * socket(TcpIoRequest *request, int type, int protocol);

    bool connect(TcpIoRequest * request, int type = SOCK_STREAM, int protocol = 0);

    void recvThread();

    void epollThread();
    void sendHeartbeatPackets();
    void handleHeartbeatPacket();
    bool removeRequest();

    void updateHeartbeatPacket(TcpIoRequest *request);
    void setHeartbeatPacketIntervalTime(int heartbeatPacketIntervalTime = 120000);

    bool recvCallback(TcpIoRequest *request);
    bool sendingOrRecving(TcpIoRequest *request);
    bool releaseCallback(TcpIoRequest * request);

private:
    int                                 mFamily;
    int                                 mSocketType;
    int                                 mProtocol;
    int                                 mPort;
    std::string                         mIP;

    bool                                (* mRecvCallback)(TcpIoRequest * request, void * owner);
    bool                                (* mConnectCallback)(TcpIoRequest * request, bool isConnected, void * owner);
    bool                                (* mReleaseCallback)(TcpIoRequest * request, void * owner);
    void                                * mCallbackOwner;

    // std::thread                         mAcceptThreads[TcpConfig::mAcceptThreadNumber];
    std::vector<std::thread *>          mRecvThreads; // [TcpConfig::mRecvThreadNumber];
    int                                 mRecvThreadNumber;
    std::thread                         mEpollThread;

    pthread_mutex_t                     mRecvMutex;
    pthread_cond_t                      mRecvCond;

    std::mutex                          mRequestMutex;
    std::map<int, TcpIoRequest *>       mMapTcpIoRequest;
    std::vector<TcpIoRequest *>         mVecReleaseRequest;
    std::list<TcpIoRequest *>           mDequeRecvTcpIoRequest;
    pthread_mutex_t                     mDequeMutex;

    int                                 mEpollFd;
    TcpIoScheduleImpl::Status           mStatus;
    bool                                mIsBlocking;
    int                                 mTimeout;
    int                                 mHeartbeatPacketIntervalTime;
};
};  // tcpclient
#endif // TCP_CLIENT_TCPIOSCHEDULEIMPL_H

