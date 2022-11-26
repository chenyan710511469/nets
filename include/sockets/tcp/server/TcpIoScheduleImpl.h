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
#ifndef TCP_SERVER_TCPIOSCHEDULEIMPL_H
#define TCP_SERVER_TCPIOSCHEDULEIMPL_H

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
#include <condition_variable>
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

namespace tcpserver
{
class TcpIoScheduleImpl : public TcpIoSchedule
{
public:
    TcpIoScheduleImpl(
            void (* recvCallback)(TcpIoRequest * request, void * owner),
            bool (* releaseCb)(TcpIoRequest * request, void * owner),
            void * owner
        );
    ~TcpIoScheduleImpl();

private:
    TcpIoRequest * allocRequest(int family);

    TcpIoSocket * associate(int socketFd);

    TcpIoScheduleImpl::Status getStatus();

    void setBlocking(bool isBlocking = false, int timeout = 0);

    TcpIoScheduleImpl(const TcpIoScheduleImpl & other) = delete;
    TcpIoScheduleImpl & operator=(const TcpIoScheduleImpl & other) = delete;

    bool setHandleSignal();
    bool preStart(int acceptThreadNumber = ONE, int recvThreadNumber = TWO);
    bool start(TcpIoRequest *request, int type = SOCK_STREAM, int protocol = 0);
    bool stop(TcpIoRequest *request);
    bool stopAll();
    bool post(TcpIoRequest * request);

    TcpIoSocket * socket(int family, int type, int protocol);

    void acceptThread();

    void recvThread();

    void epollThread();
    void sendHeartbeatPackets();
    bool getRequestServer(TcpIoRequest **request, int serverFd);

    void handleHeartbeatPacket();
    void removeRequest();

    void updateHeartbeatPacket(TcpIoRequest *request);
    void setHeartbeatPacketIntervalTime(int heartbeatPacketIntervalTime = 120000);

    bool sendingOrRecving(TcpIoRequest *request);
    bool releaseCallback(TcpIoRequest * request);

    static void signalHandle(int sig);
    static void sigactionHandle(int sig, siginfo_t *info, void *secret);

private:
    void                                (* mRecvCallback)(TcpIoRequest * request, void * owner);
    bool                                (* mReleaseCallback)(TcpIoRequest * request, void * owner);
    void                                * mCallbackOwner;

    int                                 mAcceptThreadNumber;
    std::vector<std::thread *>          mAcceptThreads;
    int                                 mRecvThreadNumber;
    std::vector<std::thread *>          mRecvThreads;
    std::thread                         mEpollThread;

    pthread_mutex_t                     mRecvMutex;
    pthread_cond_t                      mRecvCond;

    std::mutex                          mMutexClient;

    std::map<int, TcpIoRequest *>       mMapRequestClient;
    std::vector<TcpIoRequest *>         mVecReleaseRequest;
    std::deque<TcpIoRequest *>          mDequeRecvRequest;
    pthread_mutex_t                     mDequeMutex;

    int                                 mEpollFd;
    TcpIoSchedule::Status               mStatus;
    bool                                mIsBlocking;
    int                                 mTimeout;
    int                                 mHeartbeatPacketIntervalTime;

    std::map<int, TcpIoRequest *>       mMapRequestServer;
    std::mutex                          mMutexRequestServer;
    std::mutex                          mMutexServer;
    std::condition_variable             mCondServer;
    std::deque<TcpIoRequest *>          mDequeAcceptRequest;
    // std::mutex                          mMutexAccept;
};
};  // tcpserver
#endif // TCP_SERVER_TCPIOSCHEDULEIMPL_H

