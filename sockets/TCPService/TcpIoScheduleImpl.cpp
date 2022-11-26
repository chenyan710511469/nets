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

#include "sockets/tcp/server/TcpIoScheduleImpl.h"

namespace tcpserver
{
TcpIoScheduleImpl::TcpIoScheduleImpl(
        void (* recvCallback)(TcpIoRequest * request, void * owner),
        bool (* releaseCb)(TcpIoRequest * request, void * owner),
        void * owner
    )
 : mRecvCallback(recvCallback)
 , mReleaseCallback(releaseCb)
 , mCallbackOwner(owner)
 , mAcceptThreadNumber(ONE)
 , mRecvThreadNumber(TWO)
 , mEpollFd(::epoll_create(TcpConfig::mEpollSize))
 , mStatus(TcpIoSchedule::eStopped)
 , mIsBlocking(false)
 , mTimeout(0)
 , mHeartbeatPacketIntervalTime(120000)
{
    if(0 >= mEpollFd)
    {
        printf("error:%s. errno:%d\n", strerror(errno), errno);
        exit(-1);
    }
    pthread_mutex_init(&mRecvMutex, NULL);
    pthread_cond_init(&mRecvCond, NULL);
    pthread_mutex_init(&mDequeMutex, NULL);
}

TcpIoScheduleImpl::~TcpIoScheduleImpl()
{
    stopAll();
    pthread_mutex_destroy(&mRecvMutex);
    pthread_cond_destroy(&mRecvCond);
    pthread_mutex_destroy(&mDequeMutex);

    if(0 < mEpollFd)
    {
        ::close(mEpollFd);
        mEpollFd = 0;
    }
}

void TcpIoScheduleImpl::signalHandle(int sig)
{
    printf("sig: %d.\n", sig);
}

void TcpIoScheduleImpl::sigactionHandle(int sig, siginfo_t *info, void *secret)
{
    printf("sig: %d.\n", sig);
    std::cout << "secret: " << secret << std::endl;
    if (sig == SIGPIPE)
    {}
    else if (sig == SIGALRM)
    {}
}

bool TcpIoScheduleImpl::setHandleSignal()
{
    struct sigaction act, oldAct;
    memset(&act, 0, sizeof(act));
    sigemptyset(&act.sa_mask);
    act.sa_handler = signalHandle;
    act.sa_sigaction = sigactionHandle;
    act.sa_flags = SA_NODEFER | SA_SIGINFO;
    sigaction(SIGALRM, &act, &oldAct);
    sigaction(SIGPIPE, &act, &oldAct);
    return true;
}

bool TcpIoScheduleImpl::preStart(int acceptThreadNumber, int recvThreadNumber) {
    mAcceptThreadNumber = acceptThreadNumber;
    mRecvThreadNumber = recvThreadNumber;
    return true;
}

bool TcpIoScheduleImpl::start(TcpIoRequest *request, int type, int protocol)
{
    int flag = 0;
    try {
        if(mStatus != TcpIoSchedule::eStarted) {
            mStatus = TcpIoSchedule::eStarted;
            flag = ONE;
            size_t i = 0;
            for(; i < mRecvThreadNumber; ++i) {
                mRecvThreads.push_back(new std::thread(&TcpIoScheduleImpl::recvThread, this));
            }
            mEpollThread = std::thread(&TcpIoScheduleImpl::epollThread, this);
            i = 0;
            for(; i < mAcceptThreadNumber; ++i) {
                mAcceptThreads.push_back(new std::thread(&TcpIoScheduleImpl::acceptThread, this));
            }
            flag = TWO;
        }
        if(mStatus != eStarted) {
            return false;
        }
        TcpIoSocket *aSocket = NULL;
        while (NULL == (aSocket = this->socket(request->Args.Bind.mAddr->sa_family, type, protocol)));
        int fd = aSocket->descriptor();
        if(0 > ::bind(fd, request->Args.Bind.mAddr, request->Args.Bind.mAddrLen)) {
            std::string strError(strerror(errno));
            delete aSocket;
            throw std::runtime_error(strError);
        }
        if(0 > ::listen(fd, 200000)) {
            std::string strError(strerror(errno));
            delete aSocket;
            throw std::runtime_error(strError);
        }

        // 添加进epoll
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));
        ev.events = EPOLLIN | EPOLLET;// 使用epoll边缘触发
        ev.data.fd = fd;
        ev.data.ptr = request;
        if(0 > epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &ev)) {
            delete aSocket;
            return false;
        }
        request->Args.Bind.mSocket = aSocket;
        mMutexRequestServer.lock();
        mMapRequestServer.insert(std::pair<int, TcpIoRequest *>(aSocket->descriptor(), request));
        mMutexRequestServer.unlock();
        printf("successfully started tcp server: %s:%d.\n",
        request->Args.Bind.mLocalIp.c_str(), request->Args.Bind.mLocalPort);
        return true;
    }
    catch(std::exception & ex) {
        std::cout << ex.what() << std::endl;
    }
    catch(...) {
    }
    if(ONE == flag) {
        stopAll();
    }
    return false;
}

bool TcpIoScheduleImpl::stop(TcpIoRequest *request)
{
    if(NULL == request) {
        return false;
    }
    if(0 > request->mServerFd) {
        if (NULL == request->Args.Bind.mSocket) {
            return true;
        }
        int serverFd = request->Args.Bind.mSocket->descriptor();
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = serverFd;
        ev.data.ptr = request;
        ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, serverFd, &ev);
        request->Args.Bind.mSocket->close();
        mCondServer.notify_all();
        mMutexRequestServer.lock();
        mMapRequestServer.erase(serverFd);
        mMutexRequestServer.unlock();
        mMutexClient.lock();
        std::map<int, TcpIoRequest *>::iterator itM = mMapRequestClient.begin();
        for(; itM != mMapRequestClient.end(); ) {
            TcpIoRequest *requestClient = itM->second;
            if(requestClient->mServerFd == serverFd) {
                requestClient->Args.Accept.mSocket->close();
                itM = mMapRequestClient.erase(itM);
                while(sendingOrRecving(requestClient)) {
                    usleep(1000);
                }
                delete requestClient;
                continue;
            }
            ++itM;
        }
        mMutexClient.unlock();
        delete request;
        return true;
    }
    else {
        int clientFd = request->Args.Accept.mSocket->descriptor();
        if(0 < clientFd) {
            mMutexClient.lock();
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = clientFd;
            ev.data.ptr = request;
            ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, clientFd, &ev);
            request->Args.Accept.mSocket->close();
            releaseCallback(request);
            printf("%s:%d release fd=%d.\n", __FUNCTION__, __LINE__, clientFd);
            mVecReleaseRequest.push_back(request);
            mMapRequestClient.erase(clientFd);
            mMutexClient.unlock();
        }
        removeRequest();
        return true;
    }
    return false;
}

bool TcpIoScheduleImpl::stopAll()
{
    if(mStatus == TcpIoSchedule::eStarted) {
        mStatus = TcpIoSchedule::eStopping;
        mCondServer.notify_all();
        pthread_mutex_unlock(&mRecvMutex);
        mMutexClient.unlock();
        pthread_cond_signal(&mRecvCond);
        pthread_mutex_unlock(&mDequeMutex);
        size_t i = 0;
        for(; i < mAcceptThreads.size(); ++i) {
            mAcceptThreads[i]->join();
            delete mAcceptThreads[i];
        }
        mAcceptThreads.clear();
        mEpollThread.join();
        i  = 0;
        for(; i < mRecvThreads.size(); ++i) {
            mRecvThreads[i]->join();
            delete mRecvThreads[i];
        }
        mRecvThreads.clear();
        std::vector<TcpIoRequest *>::iterator itV = mVecReleaseRequest.begin();
        for(; itV != mVecReleaseRequest.end(); ) {
            TcpIoRequest *request = *itV;
            int fd = request->Args.Accept.mSocket->descriptor();
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = fd;
            ev.data.ptr = request;
            ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &ev);
            releaseCallback(request);
            printf("%s:%d release fd=%d.\n", __FUNCTION__, __LINE__, fd);
            delete request;
            itV = mVecReleaseRequest.erase(itV);
        }
        std::map<int, TcpIoRequest *>::iterator itM = mMapRequestClient.begin();
        for(; itM != mMapRequestClient.end(); ) {
            TcpIoRequest *request = itM->second;
            int fd = request->Args.Accept.mSocket->descriptor();
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = fd;
            ev.data.ptr = request;
            ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &ev);
            releaseCallback(request);
            printf("%s:%d release fd=%d.\n", __FUNCTION__, __LINE__, fd);
            delete request;
            itM = mMapRequestClient.erase(itM);
        }
        std::map<int, TcpIoRequest *>::iterator itS = mMapRequestServer.begin();
        for(; itS != mMapRequestServer.end(); ) {
            TcpIoRequest *request = itS->second;
            int fd = request->Args.Bind.mSocket->descriptor();
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = fd;
            ev.data.ptr = request;
            ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &ev);
            releaseCallback(request);
            printf("%s:%d release fd=%d.\n", __FUNCTION__, __LINE__, fd);
            delete request;
            itS = mMapRequestServer.erase(itS);
        }
        mStatus = TcpIoSchedule::eStopped;
        return true;
    }

    return false;
}

bool TcpIoScheduleImpl::post(TcpIoRequest * request)
{
    return request->Args.Accept.mSocket->send(request);
}

TcpIoRequest * TcpIoScheduleImpl::allocRequest(int family)
{
    return new TcpIoRequest(family, mRecvCallback, mCallbackOwner);
}

TcpIoSocket * TcpIoScheduleImpl::associate(int socketFd)
{
    TcpIoSocket *aTcpIoSocket = new TcpIoSocket(this, socketFd);
    return aTcpIoSocket;
}

TcpIoSocket * TcpIoScheduleImpl::socket(int family, int type, int protocol)
{
    TcpIoSocket *aTcpIoSocket = new TcpIoSocket(this, family, type, protocol);
    //aTcpIoSocket->setIsBlocking(false); // non-blocking(不阻塞).
    aTcpIoSocket->setIsBlocking(mIsBlocking);
    aTcpIoSocket->setRecvBlocking(mIsBlocking, mTimeout);
    return aTcpIoSocket;
}

void TcpIoScheduleImpl::acceptThread()
{
    while (mStatus == TcpIoSchedule::eStarted) {
        TcpIoRequest * request = NULL;
        try {
            TcpIoRequest *requestServer = NULL;
            try {
                if (mDequeAcceptRequest.empty()) {
                    std::unique_lock<std::mutex> lck(mMutexServer);
                    mCondServer.wait_for(lck, std::chrono::seconds(30));
                    if (mDequeAcceptRequest.empty()) {
                        continue;
                    }
                    requestServer = mDequeAcceptRequest.front();
                    mDequeAcceptRequest.pop_front();
                }
                // mMutexAccept.lock();
                // requestServer = mDequeAcceptRequest.front();
                // mDequeAcceptRequest.pop_front();
                // mMutexAccept.unlock();
            }
            catch (std::exception & e) {
                // mMutexAccept.unlock();
                continue;
            }
            if (NULL == requestServer) {
                continue;
            }
            TcpIoSocket *socketServer = requestServer->Args.Bind.mSocket;
            if (NULL == socketServer) {
                continue;
            }
            int family = requestServer->getFamily();
            while (NULL == (request = allocRequest(family)));
            socklen_t addrLength = 0;
            if (family == AF_INET6) {
                addrLength = sizeof(struct sockaddr_in6);
            } else {
                addrLength = sizeof(struct sockaddr_in);
            }
            while (NULL == (request->Args.Accept.mAddr = (struct sockaddr *)malloc((size_t)addrLength)));
            memset(request->Args.Accept.mAddr, 0, (size_t)addrLength);
            request->Args.Accept.mAddrLen = (int)addrLength;
            request->Args.Accept.mSocket = NULL;
            if (!socketServer->accept(request)) {
                delete request;
                request = NULL;
                continue;
            }
            updateHeartbeatPacket(request);
            socketServer->getPeerAddressAndPort(request);
            int clientSocketFd = request->Args.Accept.mSocket->descriptor();

            mMutexClient.lock();
            std::map<int, TcpIoRequest *>::iterator it = mMapRequestClient.find(clientSocketFd);
            if (it != mMapRequestClient.end()) {
                if (NULL != request->Args.Accept.mSocket) {
                    request->Args.Accept.mSocket->close();
                }
                releaseCallback(request);
                printf("%s:%d release fd=%d.\n", __FUNCTION__, __LINE__, clientSocketFd);
                mVecReleaseRequest.push_back(it->second);
                mVecReleaseRequest.push_back(request);  // 必须释放,否则析构时会close这个socketfd
                mMapRequestClient.erase(clientSocketFd);
            } else {
                mMapRequestClient.insert(std::pair<int, TcpIoRequest *>(clientSocketFd, request));
                // 添加进epoll
                struct epoll_event ev;
                memset(&ev, 0, sizeof(struct epoll_event));
                ev.events = EPOLLIN | EPOLLET;// 使用epoll边缘触发
                ev.data.fd = clientSocketFd;
                ev.data.ptr = request;
                request->mServerFd = socketServer->descriptor();
                while (0 > epoll_ctl(mEpollFd, EPOLL_CTL_ADD, clientSocketFd, &ev));
                request->Args.Bind.mLocalPort = requestServer->Args.Bind.mLocalPort;
                request->Args.Bind.mLocalIp = requestServer->Args.Bind.mLocalIp;
                request->Args.Bind.mLocalMac = requestServer->Args.Bind.mLocalMac;
                printf("accepted: %s:%d, local address: %s:%d.\n",
                        request->Args.Accept.mRemoteIP.c_str(),
                        request->Args.Accept.mRemotePort,
                        request->Args.Bind.mLocalIp.c_str(),
                        request->Args.Bind.mLocalPort);
            }
            mMutexClient.unlock();
        }
        catch (std::exception & ex) {
            std::cout << ex.what() << std::endl;
            if (NULL != request) {
                delete request;
                request = NULL;
            }
        }
        catch (...) {
            if(NULL != request) {
                delete request;
                request = NULL;
            }
        }
    }
}

void TcpIoScheduleImpl::recvThread()
{
    signal(SIGALRM, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    while(mStatus == TcpIoSchedule::eStarted)
    {std::cout << "thread id: " << std::this_thread::get_id() << " " << __FUNCTION__ << std::endl;
        std::deque<TcpIoRequest *> aDequeRequest;
        {
            pthread_mutex_lock(&mRecvMutex);
            if(mDequeRecvRequest.empty())
            {
                struct timespec aTime;
                aTime.tv_sec = time(NULL) + 10;// 等待5秒钟, 继续下一次循环
                aTime.tv_nsec = 0;
                int iRet = pthread_cond_timedwait(&mRecvCond, &mRecvMutex, &aTime);
                if(iRet == ETIMEDOUT)
                {
                    if(mDequeRecvRequest.empty())
                    {
                        pthread_mutex_unlock(&mRecvMutex);
                        continue;
                    }
                }
            }
            pthread_mutex_lock(&mDequeMutex);
            aDequeRequest = std::move(mDequeRecvRequest);
            pthread_mutex_unlock(&mDequeMutex);
            if(aDequeRequest.empty())
            {
                pthread_mutex_unlock(&mRecvMutex);
                continue;
            }
            pthread_mutex_unlock(&mRecvMutex);
        }
        std::deque<TcpIoRequest *>::iterator it = aDequeRequest.begin();
        for(; it != aDequeRequest.end() && !aDequeRequest.empty(); ++it)
        {
            TcpIoRequest * request = *it;
            if(NULL == request)
            {
                continue;
            }
            try
            {
                printf("request address is %p\n", request);
                updateHeartbeatPacket(request);
                if (NULL == request->Args.Accept.mSocket) {
                    continue;
                }
                int aClientFd = request->Args.Accept.mSocket->descriptor();
                char aBuff[1] = {0};
                if(request->Args.Accept.mSocket->mRecving) {
                    continue;
                }
                if(!request->Args.Accept.mSocket->requestRecv()) {// 已经被释放了, 不用release
                    continue;
                }
                ssize_t msgSize = ::recv(aClientFd, aBuff, 1, MSG_PEEK | MSG_TRUNC);
                if(0 == msgSize) {
                    msgSize = ::recv(aClientFd, aBuff, 1, 0);
                    if(0 == msgSize) {
                        request->Args.Accept.mSocket->releaseRecv();
                        continue;
                    }
                    else if(EPIPE == msgSize) {
                        request->Args.Accept.mSocket->close();
                        request->Args.Accept.mSocket->releaseRecv();
                        releaseCallback(request);
                    }
                }
                // 因为TcpIoSocket::recv(request)有回调，以防万一有异常抛出
                TcpIoSchedule::ReturnType returnType = request->Args.Accept.mSocket->recv(request);
            }
            catch(std::exception & ex) {
                std::cout << ex.what() << std::endl;
            }
            catch(...) {
            }
            request->Args.Accept.mSocket->releaseRecv();
        }
        aDequeRequest.clear();
    }
}

void TcpIoScheduleImpl::epollThread()
{
    int counter = 0;
    while(mStatus == TcpIoSchedule::eStarted) {
        struct epoll_event events[TcpConfig::mEpollSize];
        memset(events, 0, sizeof(events));
        int anFdsCount = epoll_wait(mEpollFd, events, sizeof(events) / sizeof(struct epoll_event), 1000 * 10); // 10s.
        int i = 0;
        for(; i < anFdsCount; ++i) {
            TcpIoRequest * request = (TcpIoRequest *)events[i].data.ptr;
            if(0 > request->mServerFd) {  // the request is accept().
                // mMutexServer.lock();
                std::unique_lock<std::mutex> lck(mMutexServer);
                // mMutexAccept.lock();
                mDequeAcceptRequest.push_back(request);
                // mMutexAccept.unlock();
                mCondServer.notify_one();
                printf("epoll notified: %s:%d.\n", request->Args.Bind.mLocalIp.c_str(), request->Args.Bind.mLocalPort);
                continue;
            }
            int aClientFd = events[i].data.fd;
            if(request->Args.Accept.mSocket->mRecving)
            {// 已正在recv
                printf("%s:%d  已正在recv: %s:%d.\n", __FUNCTION__, __LINE__,
                request->Args.Accept.mRemoteIP.c_str(), request->Args.Accept.mRemotePort);
                continue;
            }
            printf("%s:%d  %s:%d.\n", __FUNCTION__, __LINE__,
            request->Args.Accept.mRemoteIP.c_str(), request->Args.Accept.mRemotePort);
            pthread_mutex_lock(&mDequeMutex);
            mDequeRecvRequest.push_back(request);
            pthread_mutex_unlock(&mDequeMutex);
            pthread_cond_signal(&mRecvCond);
        }
        handleHeartbeatPacket();
        if(++counter >= 10) {
            counter = 0;
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

bool TcpIoScheduleImpl::getRequestServer(TcpIoRequest **request, int serverFd)
{
    mMutexRequestServer.lock();
    std::map<int, TcpIoRequest *>::iterator it = mMapRequestServer.find(serverFd);
    mMutexRequestServer.lock();
    if(it != mMapRequestServer.end())
    {
        *request = it->second;
        return true;
    }
    return false;
}

void TcpIoScheduleImpl::handleHeartbeatPacket() {
    mMutexClient.lock();
    std::map<int, TcpIoRequest *>::iterator it = mMapRequestClient.begin();
    for(; it != mMapRequestClient.end(); ) {
        int clientFd = it->first;
        TcpIoRequest *request = it->second;
        if(sendingOrRecving(request)) {
            updateHeartbeatPacket(request);
            ++it;
            continue;
        }
        struct timespec aTime;
        memset(&aTime, 0, sizeof(struct timespec));
        if(0 != clock_gettime(CLOCK_BOOTTIME, &aTime)) {
            std::cout << "clock_gettime() error." << std::endl;
            ++it;
            continue;
        }
        long long millisecond = (aTime.tv_sec * 1000) + (aTime.tv_nsec / 1000000);
        if(millisecond > request->HeartbeatPacket.mLastTime + mHeartbeatPacketIntervalTime) // 相差mHeartbeatPacketIntervalTime ms.
        {
            signal(SIGALRM, SIG_IGN);
            signal(SIGPIPE, SIG_IGN);
            /*
            std::cout << "HeartbeatPacket.mBuffer: " << request->HeartbeatPacket.mBuffer
                << std::endl
                << " HeartbeatPacket.mBufferSize: " << request->HeartbeatPacket.mBufferSize
                << std::endl;
            */
            ::send(clientFd, request->HeartbeatPacket.mBuffer, request->HeartbeatPacket.mBufferSize, MSG_NOSIGNAL);
            if(EPIPE == errno) {
                struct epoll_event anEv;
                memset(&anEv, 0, sizeof(struct epoll_event));
                anEv.events = EPOLLIN | EPOLLET;// 使用epoll边缘触发
                anEv.data.fd = clientFd;
                epoll_ctl(mEpollFd, EPOLL_CTL_DEL, clientFd, &anEv);
                it = mMapRequestClient.erase(it);
                releaseCallback(request);
                printf("%s:%d release fd=%d.\n", __FUNCTION__, __LINE__, clientFd);
                mVecReleaseRequest.push_back(request);
                continue;
            }
        }
        else {
            ++it;
            continue;
        }
        request->HeartbeatPacket.mThreeTimes++;
        if(3 < request->HeartbeatPacket.mThreeTimes) {
            struct epoll_event anEv;
            memset(&anEv, 0, sizeof(struct epoll_event));
            anEv.events = EPOLLIN | EPOLLET;  // 使用epoll边缘触发
            anEv.data.fd = clientFd;
            epoll_ctl(mEpollFd, EPOLL_CTL_DEL, clientFd, &anEv);
            it = mMapRequestClient.erase(it);
            releaseCallback(request);
            printf("%s:%d release fd=%d.\n", __FUNCTION__, __LINE__, clientFd);
            mVecReleaseRequest.push_back(request);
            continue;
        }
        ++it;
    }
    mMutexClient.unlock();
}

void TcpIoScheduleImpl::removeRequest()
{
    std::vector<TcpIoRequest *>::iterator itVec = mVecReleaseRequest.begin();
    for(; itVec != mVecReleaseRequest.end(); ) {
        TcpIoRequest *request = *itVec;
        if(NULL != request) {
            if(sendingOrRecving(request)) {
                updateHeartbeatPacket(request);
                ++itVec;
                continue;
            }
            releaseCallback(request);
            printf("%s:%d release.\n", __FUNCTION__, __LINE__);
            delete request;
            *itVec = NULL;
            request = NULL;
        }
        itVec = mVecReleaseRequest.erase(itVec);
    }
}

void TcpIoScheduleImpl::updateHeartbeatPacket(TcpIoRequest *request)
{
    if(NULL == request)
        return;
    try {
        request->init();
    }
    catch(std::exception & ex) {
        std::cout << ex.what() << std::endl;
    }
    catch(...) {
    }
}

void TcpIoScheduleImpl::setHeartbeatPacketIntervalTime(int heartbeatPacketIntervalTime)
{
    mHeartbeatPacketIntervalTime = heartbeatPacketIntervalTime;
}

bool TcpIoScheduleImpl::sendingOrRecving(TcpIoRequest *request)
{
    return request->mSending || request->Args.Accept.mSocket->mRecving;
}

bool TcpIoScheduleImpl::releaseCallback(TcpIoRequest * request)
{
    if (NULL != mReleaseCallback && NULL != mCallbackOwner) {
        return mReleaseCallback(request, mCallbackOwner);
    }
    return false;
}

};

