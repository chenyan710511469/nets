/*
 * =====================================================================================
 *
 *       Filename:  TcpIoScheduleImpl.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月05日 15时46分47秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */

#include "sockets/tcp/client/TcpIoScheduleImpl.h"

namespace tcpclient
{
TcpIoScheduleImpl::TcpIoScheduleImpl(
        bool (* recvCallback)(TcpIoRequest * request, void * owner),
        bool (* connectCallback)(TcpIoRequest * request, bool isConnected, void * owner),
        bool (* releaseCb)(TcpIoRequest * request, void * owner),
        void * owner)
 : mRecvCallback(recvCallback)
 , mConnectCallback(connectCallback)
 , mReleaseCallback(releaseCb)
 , mCallbackOwner(owner)
 , mRecvThreadNumber(TWO)
 , mEpollFd(epoll_create(TcpConfig::mEpollSize))
 , mStatus(TcpIoScheduleImpl::eStopped)
 , mIsBlocking(false)
 , mTimeout(0)
 , mHeartbeatPacketIntervalTime(120000)
{
    if (0 >= mEpollFd) {
        printf("%s:%d %s", __FUNCTION__, __LINE__, strerror(errno));
        exit(-1);
    }
    pthread_mutex_init(&mRecvMutex, NULL);
    pthread_cond_init(&mRecvCond, NULL);
    pthread_mutex_init(&mDequeMutex, NULL);
}

TcpIoScheduleImpl::~TcpIoScheduleImpl()
{
    pthread_mutex_destroy(&mRecvMutex);
    pthread_cond_destroy(&mRecvCond);
    pthread_mutex_destroy(&mDequeMutex);

    if (0 < mEpollFd) {
        ::close(mEpollFd);
        mEpollFd = 0;
    }
    stopAll();
    pthread_mutex_destroy(&mRecvMutex);
    pthread_cond_destroy(&mRecvCond);
    pthread_mutex_destroy(&mDequeMutex);
}

bool TcpIoScheduleImpl::preStart(int recvThreadNumber) {
    mRecvThreadNumber = recvThreadNumber;
    return true;
}

bool TcpIoScheduleImpl::start()
{
    try
    {
        if (mStatus == TcpIoSchedule::eStarted) {
            return true;
        }
        if (mStatus == TcpIoSchedule::eStopping) {
            return false;
        }
        mStatus = TcpIoSchedule::eStarted;
        size_t iRet = 0, i = 0;
        for(; i < mRecvThreadNumber; ++i) {
            mRecvThreads.push_back(new std::thread(&TcpIoScheduleImpl::recvThread, this));
        }
        mEpollThread = std::thread(&TcpIoScheduleImpl::epollThread, this);
        return true;
    }
    catch(std::exception & ex)
    {
        std::cout << ex.what() << std::endl;
        stopAll();
    }
    catch(...)
    {
        stopAll();
    }
    return false;
}

bool TcpIoScheduleImpl::stopAll()
{
    if (mStatus == TcpIoSchedule::eStarted) {
        mStatus = TcpIoSchedule::eStopping;
        mRequestMutex.lock();
        std::map<int, TcpIoRequest *>::iterator itM = mMapTcpIoRequest.begin();
        for(; itM != mMapTcpIoRequest.end(); ) {
            TcpIoRequest * request = itM->second;
            int clientFd = request->Args.Connect.mSocket->descriptor();
            while (sendingOrRecving(request)) {
                continue;
            }
            request->Args.Connect.mSocket->close();
            releaseCallback(request);
            mVecReleaseRequest.push_back(request);
            itM = mMapTcpIoRequest.erase(itM);
            printf("%s:%d released fd=%d.\n", __FUNCTION__, __LINE__, clientFd);
        }
        mRequestMutex.unlock();
        std::vector<TcpIoRequest *>::iterator itV = mVecReleaseRequest.begin();
        for(; itV != mVecReleaseRequest.end(); ) {
            TcpIoRequest * request = *itV;
            releaseCallback(request);
            itV = mVecReleaseRequest.erase(itV);
            delete request;
            request = NULL;
        }
        pthread_mutex_unlock(&mRecvMutex);
        mRequestMutex.unlock();
        pthread_mutex_unlock(&mDequeMutex);
        pthread_cond_signal(&mRecvCond);
        mEpollThread.join();
        size_t i = 0;
        for(; i < mRecvThreads.size(); ++i) {
            mRecvThreads[i]->join();
            delete mRecvThreads[i];
        }
        mRecvThreads.clear();
        mStatus = TcpIoSchedule::eStopped;
        return true;
    }

    return false;
}

bool TcpIoScheduleImpl::stop(TcpIoRequest * request)
{
    if (NULL == request) {
        return false;
    }
    if (NULL == request->Args.Connect.mSocket) {
        delete request;
        return true;
    }
    return this->close(request);
}

bool TcpIoScheduleImpl::close(TcpIoRequest * request)
{
    if (NULL == request) {
        return false;
    }
    int clientFd = request->Args.Connect.mSocket->descriptor();
    mRequestMutex.lock();
    mMapTcpIoRequest.erase(clientFd);
    struct epoll_event ev;
    memset(&ev, 0, sizeof(struct epoll_event));
    ev.events = EPOLLIN | EPOLLET;  // 使用epoll边缘触发
    ev.data.fd = clientFd;
    epoll_ctl(mEpollFd, EPOLL_CTL_DEL, clientFd, &ev);
    while (sendingOrRecving(request)) {
        continue;
    }
    request->Args.Connect.mSocket->close();
    releaseCallback(request);
    mVecReleaseRequest.push_back(request);
    mRequestMutex.unlock();
    return true;
}

bool TcpIoScheduleImpl::post(TcpIoRequest * request)
{
    return request->Args.Connect.mSocket->send(request);
}

TcpIoRequest * TcpIoScheduleImpl::allocRequest(int family)
{
    return new TcpIoRequest(family);
}

TcpIoSocket * TcpIoScheduleImpl::associate(int socketFd)
{
    TcpIoSocket *aTcpIoSocket = new TcpIoSocket(this, socketFd);
    return aTcpIoSocket;
}

TcpIoSocket * TcpIoScheduleImpl::socket(TcpIoRequest * request, int type, int protocol)
{
    TcpIoSocket *aTcpIoSocket = new TcpIoSocket(this, request, type, protocol);
    aTcpIoSocket->setIsBlocking(mIsBlocking);
    aTcpIoSocket->setRecvBlocking(mIsBlocking, mTimeout);
    return aTcpIoSocket;
}

void TcpIoScheduleImpl::recvThread()
{
    signal(SIGALRM, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    while (mStatus == TcpIoSchedule::eStarted)
    {
        std::list<TcpIoRequest *> aDequeTcpIoRequest;
        {
            pthread_mutex_lock(&mRecvMutex);
            if (mDequeRecvTcpIoRequest.empty())
            {
                struct timespec aTime;
                aTime.tv_sec = time(NULL) + 10;// 等待5秒钟, 继续下一次循环
                aTime.tv_nsec = 0;
                int iRet = pthread_cond_timedwait(&mRecvCond, &mRecvMutex, &aTime);
                if (iRet == ETIMEDOUT) {
                    if (mDequeRecvTcpIoRequest.empty()) {
                        pthread_mutex_unlock(&mRecvMutex);
                        continue;
                    }
                }
            }
            pthread_mutex_lock(&mDequeMutex);
            aDequeTcpIoRequest = std::move(mDequeRecvTcpIoRequest);
            pthread_mutex_unlock(&mDequeMutex);
            if (aDequeTcpIoRequest.empty()) {
                pthread_mutex_unlock(&mRecvMutex);
                continue;
            }
            pthread_mutex_unlock(&mRecvMutex);
        }
        std::list<TcpIoRequest *>::iterator it = aDequeTcpIoRequest.begin();
        for(; it != aDequeTcpIoRequest.end() && !aDequeTcpIoRequest.empty(); ++it)
        {
            TcpIoRequest * request = *it;
            if (NULL == request) {
                continue;
            }
            try
            {
                updateHeartbeatPacket(request);
                int aClientFd = request->Args.Connect.mSocket->descriptor();
                char aBuff[1] = {0};
                if (request->Args.Connect.mSocket->mRecving) {
                    continue;
                }
                if (!request->Args.Connect.mSocket->requestRecv())
                {// 已经被释放了, 不用release
                    continue;
                }
                ssize_t aPackageSize = ::recv(aClientFd, aBuff, 1, MSG_PEEK | MSG_TRUNC);
                if (0 == aPackageSize) {
                    aPackageSize = ::recv(aClientFd, aBuff, 1, 0);
                    if (0 == aPackageSize) {
                        request->Args.Connect.mSocket->releaseRecv();
                        continue;
                    }
                }
                // 因为TcpIoSocket::recv(request)有回调，以防万一有异常抛出
                TcpIoSchedule::ReturnType returnType = request->Args.Connect.mSocket->recv(request);
            }
            catch(std::exception & ex)
            {
                std::cout << ex.what() << std::endl;
            }
            catch(...)
            {}
            request->Args.Connect.mSocket->releaseRecv();
        }
        aDequeTcpIoRequest.clear();
    }
}

void TcpIoScheduleImpl::epollThread()
{
    int counter = 0;
    while (mStatus == TcpIoSchedule::eStarted)
    {
        struct epoll_event events[TcpConfig::mEpollSize];
        memset(events, 0, sizeof(events));
        int anFdsCount = epoll_wait(mEpollFd, events, sizeof(events) / sizeof(struct epoll_event), 1000 * 3);
        int i = 0;
        for(; i < anFdsCount; ++i)
        {
            int aClientFd = events[i].data.fd;
            TcpIoRequest * request = (TcpIoRequest *)events[i].data.ptr;
            if (request->Args.Connect.mSocket->mRecving)
            {// 已正在recv
                printf("%s:%d  已正在recv: %s:%d.\n", __FUNCTION__, __LINE__,
                request->Args.Connect.mIP.c_str(), request->Args.Connect.mPort);
                continue;
            }
            printf("%s:%d  %s:%d.\n", __FUNCTION__, __LINE__,
            request->Args.Connect.mIP.c_str(), request->Args.Connect.mPort);
            pthread_mutex_lock(&mDequeMutex);
            mDequeRecvTcpIoRequest.push_back(request);
            pthread_mutex_unlock(&mDequeMutex);
            pthread_cond_signal(&mRecvCond);
        }
        handleHeartbeatPacket();
        if (++counter >= 10) {
            removeRequest();
        }
    }
}

TcpIoSchedule::Status TcpIoScheduleImpl::getStatus()
{
    return mStatus;
}

void TcpIoScheduleImpl::setBlocking(bool isBlocking, int timeout)
{
    mIsBlocking = isBlocking;
    mTimeout = timeout;
}

void TcpIoScheduleImpl::handleHeartbeatPacket()
{
    mRequestMutex.lock();
    std::map<int, TcpIoRequest *>::iterator it = mMapTcpIoRequest.begin();
    for(; it != mMapTcpIoRequest.end(); )
    {
        int clientFd = it->first;
        TcpIoRequest *request = it->second;
        if (sendingOrRecving(request)) {
            updateHeartbeatPacket(request);
            ++it;
            continue;
        }
        struct timespec aTime;
        memset(&aTime, 0, sizeof(struct timespec));
        if (0 != clock_gettime(CLOCK_BOOTTIME, &aTime)) {
            std::cout << "clock_gettime() error." << std::endl;
            ++it;
            continue;
        }
        long long millisecond = (aTime.tv_sec * 1000) + (aTime.tv_nsec / 1000000);
        // 相差mHeartbeatPacketIntervalTime ms.
        if (millisecond <= request->HeartbeatPacket.mLastTime + mHeartbeatPacketIntervalTime) {
            ++it;
            continue;
        }
        request->HeartbeatPacket.mThreeTimes++;
        if (3 < request->HeartbeatPacket.mThreeTimes) {
            struct epoll_event anEv;
            memset(&anEv, 0, sizeof(struct epoll_event));
            anEv.events = EPOLLIN | EPOLLET;// 使用epoll边缘触发
            anEv.data.fd = clientFd;
            epoll_ctl(mEpollFd, EPOLL_CTL_DEL, clientFd, &anEv);
            it = mMapTcpIoRequest.erase(it);
            mVecReleaseRequest.push_back(request);
            continue;
        }
        ++it;
    }
    mRequestMutex.unlock();
}

bool TcpIoScheduleImpl::removeRequest()
{
    mRequestMutex.lock();
    std::vector<TcpIoRequest *>::iterator itVec = mVecReleaseRequest.begin();
    for(; itVec != mVecReleaseRequest.end(); )
    {
        TcpIoRequest *request = *itVec;
        itVec = mVecReleaseRequest.erase(itVec);
        if (NULL != request) {
            if (sendingOrRecving(request)) {
                updateHeartbeatPacket(request);
                ++itVec;
                continue;
            }
            releaseCallback(request);
            printf("%s:%d released fd=%d.\n", __FUNCTION__, __LINE__, request->Args.Connect.mSocket->descriptor());
            delete request;
            request = NULL;
        }
    }
    mRequestMutex.unlock();
    return true;
}

void TcpIoScheduleImpl::updateHeartbeatPacket(TcpIoRequest *request)
{
    if (NULL == request)
        return;
    try {
        request->init();
    }
    catch(std::exception & ex) {
        std::cout << ex.what() << std::endl;
    }
    catch(...)
    {}
}

void TcpIoScheduleImpl::setHeartbeatPacketIntervalTime(int heartbeatPacketIntervalTime)
{
    mHeartbeatPacketIntervalTime = heartbeatPacketIntervalTime;
}

bool TcpIoScheduleImpl::connect(TcpIoRequest * request, int type, int protocol)
{
    request->Args.Connect.mSocket = this->socket(request, type, protocol);
    updateHeartbeatPacket(request);
    int clientFd = request->Args.Connect.mSocket->descriptor();
    mRequestMutex.lock();
    std::map<int, TcpIoRequest *>::iterator it = mMapTcpIoRequest.find(clientFd);
    if (it != mMapTcpIoRequest.end()) {
        mMapTcpIoRequest.erase(clientFd);
        request->Args.Connect.mSocket->close();
        releaseCallback(request);
        mVecReleaseRequest.push_back(it->second);
        mVecReleaseRequest.push_back(request);  // 必须释放,否则析构时会close这个socketfd
        mRequestMutex.unlock();
        return false;
    }
    mMapTcpIoRequest.insert(std::pair<int, TcpIoRequest *>(clientFd, request));
    mRequestMutex.unlock();
    // 添加进epoll
    struct epoll_event ev;
    memset(&ev, 0, sizeof(struct epoll_event));
    ev.events = EPOLLIN | EPOLLET;  // 使用epoll边缘触发
    ev.data.fd = clientFd;
    ev.data.ptr = request;
    if (-1 == epoll_ctl(mEpollFd, EPOLL_CTL_ADD, clientFd, &ev)) {
        char *error = strerror(errno);
        mRequestMutex.lock();
        mMapTcpIoRequest.erase(clientFd);
        request->Args.Connect.mSocket->close();
        releaseCallback(request);
        mVecReleaseRequest.push_back(request);
        mRequestMutex.unlock();
        printf("%s:%d epoll add error: %s.\n", __FUNCTION__, __LINE__, error);
        return false;
    }
    request->Args.Connect.mSocket->getAddressAndPort(request);
    printf("successfully connected %s:%d, local address:%s:%d.\n",
            request->Args.Connect.mIP.c_str(),
            request->Args.Connect.mPort,
            request->Args.Connect.mLocalIp.c_str(),
            request->Args.Connect.mLocalPort);
    return true;
}

bool TcpIoScheduleImpl::recvCallback(TcpIoRequest *request)
{
    return mRecvCallback(request, mCallbackOwner);
}

bool TcpIoScheduleImpl::sendingOrRecving(TcpIoRequest *request)
{
    return request->mSending || request->Args.Connect.mSocket->mRecving;
}
bool TcpIoScheduleImpl::releaseCallback(TcpIoRequest * request) {
    return mReleaseCallback(request, mCallbackOwner);
}

};  // tcpclient

