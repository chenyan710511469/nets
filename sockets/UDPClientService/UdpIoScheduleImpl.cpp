/*
 * =====================================================================================
 *
 *       Filename:  UdpIoScheduleImpl.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2019年01月20日 20时16分21秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "sockets/udp/client/UdpIoScheduleImpl.h"


namespace udpclient
{
UdpIoScheduleImpl::UdpIoScheduleImpl(
        bool (* callbackRecvfrom)(UdpIoRequest *request, void *owner),
        bool (* callbackRelease)(UdpIoRequest *request, void *owner),
        void *owner)
 : mEpollFd(::epoll_create(UdpConfig::mEpollSize))
 , mStatus(UdpIoSchedule::eStopped)
 , mRecvfromCallback(callbackRecvfrom)
 , mReleaseCallback(callbackRelease)
 , mOwner(owner)
 , mRequest(NULL)
{
    if (0 >= mEpollFd) {
        printf("error:%s. errno:%d\n", strerror(errno), errno);
        exit(-1);
    }
    pthread_mutex_init(&mRecvMutex, NULL);
    pthread_cond_init(&mRecvCond, NULL);
    pthread_mutex_init(&mRequestMutex, NULL);
    pthread_mutex_init(&mReleaseMutex, NULL);
    pthread_mutex_init(&mDequeMutex, NULL);
}

UdpIoScheduleImpl::~UdpIoScheduleImpl()
{
    stopAll();
    pthread_mutex_destroy(&mRecvMutex);
    pthread_cond_destroy(&mRecvCond);
    pthread_mutex_destroy(&mRequestMutex);
    pthread_mutex_destroy(&mReleaseMutex);
    pthread_mutex_destroy(&mDequeMutex);
    if (0 < mEpollFd) {
        ::close(mEpollFd);
        mEpollFd = 0;
    }
}

bool UdpIoScheduleImpl::start(UdpIoRequest *request, int family, int type, int protocol)
{
    try
    {
        if (mStatus == UdpIoSchedule::eStopping || mStatus == UdpIoSchedule::eStarting) {
            delete request;
            return false;
        }
        if (mStatus != UdpIoSchedule::eStarted) {
            mStatus = UdpIoSchedule::eStarting;
            for(int i = 0; i < sizeof(mRecvThreads) / sizeof(std::thread); ++i)
            {
                mRecvThreads[i] = std::thread(&UdpIoScheduleImpl::recvThread, this);
            }
            mEpollThread = std::thread(&UdpIoScheduleImpl::epollThread, this);
            mStatus = UdpIoSchedule::eStarted;
        }
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));
        addr.sin_family = family; // AF_INET;
        std::string ip(request->Args.Bind.mIP);
        if (ip.empty()) {
            addr.sin_addr.s_addr = INADDR_ANY;
        }
        else {
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
        }
        addr.sin_port = htons(request->Args.Bind.mPort);
        printf("ip=%s, port=%d\n", ip.c_str(), request->Args.Bind.mPort);
        int fd = ::socket(family, type, protocol);
        int opt = SO_REUSEADDR;
        ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        request->Args.Bind.mSocket = this->associate(fd);
        request->Args.Bind.mSocket->getSelfAddress(request);

        if (0 > ::bind(fd, (struct sockaddr*)&addr, sizeof(addr))) {
            printf("error:%s. errno:%d.\n", strerror(errno), errno);
            ::close(fd);
            delete request;
            return false;
        }
        addr.sin_addr.s_addr = inet_addr(request->Args.Sendto.mIP.c_str());
        addr.sin_port = htons(request->Args.Sendto.mPort);
        if (0 > ::connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr))) {
            printf("error:%s. errno:%d", strerror(errno), errno);
            ::close(fd);
            delete request;
            return false;
        }
        bool ret = false;
        pthread_mutex_lock(&mRequestMutex);
        std::map<int, UdpIoRequest *>::iterator it = mMapUdpIoRequest.find(fd);
        if (it == mMapUdpIoRequest.end()) {
            std::pair<std::map<int, UdpIoRequest *>::iterator, bool>
            res = mMapUdpIoRequest.insert(std::pair<int, UdpIoRequest *>(fd, request));
            ret = res.second;
            if (!ret) {
                mMapUdpIoRequest.erase(fd);
                mVecReleaseRequest.push_back(it->second);
                mVecReleaseRequest.push_back(request);  // 必须释放,否则析构时会close这个fd
            }
            else {
                // 添加进epoll
                struct epoll_event ev;
                memset(&ev, 0, sizeof(struct epoll_event));
                ev.events = EPOLLIN | EPOLLET;  // 使用epoll边缘触发
                ev.data.fd = fd;
                ev.data.ptr = request;
                if (-1 == ::epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &ev)) {
                    ret = false;
                    mMapUdpIoRequest.erase(fd);
                    mVecReleaseRequest.push_back(request);  // 必须释放,否则析构时会close这个fd
                }
            }
        }
        else {
            mMapUdpIoRequest.erase(fd);
            mVecReleaseRequest.push_back(it->second);
            mVecReleaseRequest.push_back(request);  // 必须释放,否则析构时会close这个fd
        }
        pthread_mutex_unlock(&mRequestMutex);
        return ret;
    }
    catch(std::exception & ex) {
        std::cout << ex.what() << std::endl;
        stop(request);
    }
    catch(...) {
        stop(request);
    }
    if (mStatus == UdpIoSchedule::eStarting) {
        stopAll();
    }
    return false;
}

bool UdpIoScheduleImpl::stop(UdpIoRequest *request)
{
    bool ret = false;
    if (NULL == request) {
        return ret;
    }
    int fd = request->Args.Bind.mSocket->descriptor();
    pthread_mutex_lock(&mReleaseMutex);
    pthread_mutex_lock(&mRequestMutex);
    struct epoll_event ev;
    memset(&ev, 0, sizeof(struct epoll_event));
    ev.events = EPOLLIN | EPOLLET;  // 使用epoll边缘触发
    ev.data.fd = fd;
    ev.data.ptr = request;
    ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &ev);
    mMapUdpIoRequest.erase(fd);
    mVecReleaseRequest.push_back(request);
    pthread_mutex_unlock(&mRequestMutex);
    pthread_mutex_unlock(&mReleaseMutex);
    removeRequest(fd);
    return ret;
}

bool UdpIoScheduleImpl::stopAll()
{
    pthread_mutex_lock(&mRequestMutex);
    if (mStatus != UdpIoSchedule::eStarted) {
        pthread_mutex_unlock(&mRequestMutex);
        return false;
    }
    mStatus = UdpIoSchedule::eStopping;
    pthread_mutex_unlock(&mDequeMutex);
    pthread_mutex_unlock(&mRequestMutex);
    mEpollThread.join();
    for(int i = 0; i < sizeof(mRecvThreads) / sizeof(std::thread); ++i)
    {
        pthread_cond_signal(&mRecvCond);
        mRecvThreads[i].join();
    }
    pthread_mutex_lock(&mReleaseMutex);
    pthread_mutex_lock(&mRequestMutex);
    std::map<int, UdpIoRequest *>::iterator itMap = mMapUdpIoRequest.begin();
    for(; itMap != mMapUdpIoRequest.end(); )
    {
        int fd = itMap->first;
        UdpIoRequest *request = itMap->second;
        // 从epoll中删除.
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));
        ev.events = EPOLLIN | EPOLLET;  // 使用epoll边缘触发
        ev.data.fd = fd;
        ev.data.ptr = request;
        ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &ev);
        request->Args.Bind.mSocket->notifyRelease();
        mVecReleaseRequest.push_back(request);
        itMap = mMapUdpIoRequest.erase(itMap);
    }
    std::vector<UdpIoRequest *>::iterator itVec = mVecReleaseRequest.begin();
    for(; itVec != mVecReleaseRequest.end(); )
    {
        UdpIoRequest *request = *itVec;
        if (request == NULL) {
            if (0 >= mVecReleaseRequest.size()) {
                break;  // mVecReleaseRequest
            }
            ++itVec;
            continue;
        }
        int fd = request->Args.Bind.mSocket->descriptor();
        removeRequest(fd);
        itVec = mVecReleaseRequest.begin();
    }
    pthread_mutex_unlock(&mRequestMutex);
    pthread_mutex_unlock(&mReleaseMutex);
    mStatus = UdpIoSchedule::eStopped;
    return false;
}

bool UdpIoScheduleImpl::close(UdpIoRequest *request)
{
    return stop(request);
}

UdpIoRequest * UdpIoScheduleImpl::allocRequest()
{
    UdpIoRequest *request = NULL;
    do
    {
        try {
            request = new UdpIoRequest();
        }
        catch(std::exception & ex) {
            usleep(10000);
        }
    } while(NULL == request);
    return request;
}

void UdpIoScheduleImpl::freeRequest(UdpIoRequest * request)
{
    if (NULL == request) {
        return;
    }
    delete request;
    request = NULL;
}

UdpIoSocket * UdpIoScheduleImpl::associate(int socketFd)
{
    UdpIoSocket * aSocket = NULL;
    do
    {
        try {
            aSocket = new UdpIoSocket(this, socketFd);
        }
        catch(std::exception & ex) {
            printf("error: %s.\n", ex.what());
            usleep(10000);
        }
    } while(NULL == aSocket);
    return aSocket;
}

void UdpIoScheduleImpl::recvThread()
{
    signal(SIGALRM, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    while(mStatus == UdpIoSchedule::eStarting || mStatus == UdpIoSchedule::eStarted)
    {
        std::vector<UdpIoRequest *> aDequeRecvUdpIoRequest;
        {
            pthread_mutex_lock(&mRecvMutex);
            if (mDequeRecvUdpIoRequest.empty()) {
                struct timespec aTime;
                aTime.tv_sec = time(NULL) + 5;// 等待5秒钟, 继续下一次循环
                aTime.tv_nsec = 0;
                int iRet = pthread_cond_timedwait(&mRecvCond, &mRecvMutex, &aTime);
                if (iRet == ETIMEDOUT) {
                    if (mDequeRecvUdpIoRequest.empty()) {
                        pthread_mutex_unlock(&mRecvMutex);
                        continue;
                    }
                }
            }
            pthread_mutex_lock(&mDequeMutex);
            aDequeRecvUdpIoRequest = std::move(mDequeRecvUdpIoRequest);
            pthread_mutex_unlock(&mDequeMutex);
            if (aDequeRecvUdpIoRequest.empty()) {
                pthread_mutex_unlock(&mRecvMutex);
                continue;
            }
            pthread_mutex_unlock(&mRecvMutex);
        }
        std::vector<UdpIoRequest *>::iterator it;// = aDequeRecvUdpIoRequest.begin();
        for(; it != aDequeRecvUdpIoRequest.end() && !aDequeRecvUdpIoRequest.empty(); )
        {
            it = aDequeRecvUdpIoRequest.begin();
            if (it == aDequeRecvUdpIoRequest.end()) {
                continue;
            }
            UdpIoRequest * aRequest = *it;
            aDequeRecvUdpIoRequest.erase(it);
            printf("aRequest = %p\n", aRequest);
            if (NULL == aRequest) {
                continue;
            }
            try {
                if (aRequest->Args.Bind.mSocket->mRecving) {
                    continue;
                }
                if (!aRequest->Args.Bind.mSocket->requestRecv()) {
                    continue;
                }
                aRequest->Args.Bind.mSocket->recvfrom(aRequest);
            }
            catch(std::exception & ex) {
                std::cout << ex.what() << std::endl;
            }
            catch(...) {
            }
            aRequest->Args.Bind.mSocket->releaseRecv();
        }
        aDequeRecvUdpIoRequest.clear();
    }
}

void UdpIoScheduleImpl::epollThread()
{
    while(mStatus == UdpIoSchedule::eStarting || mStatus == UdpIoSchedule::eStarted)
    {
        struct epoll_event events[UdpConfig::mEpollSize] = {0};
        int fdsCount = epoll_wait(mEpollFd, events, UdpConfig::mEpollSize, 1000 * 3);
        if (mStatus == UdpIoSchedule::eStopping || mStatus == UdpIoSchedule::eStopped) {
            return;  // 避免出现死锁.
        }
        for(int i=0; i < fdsCount; ++i)
        {
            int fd = events[i].data.fd;
            UdpIoRequest * request = (UdpIoRequest *)events[i].data.ptr;
            if (request->Args.Bind.mSocket->mRecving) {  // 正在recv.
                printf("正在接收 = %p\n", request);
                continue;
            }
            printf("epoll request = %p\n", request);
            pthread_mutex_lock(&mDequeMutex);
            mDequeRecvUdpIoRequest.push_back(request);
            pthread_mutex_unlock(&mDequeMutex);
            pthread_cond_signal(&mRecvCond);
        }
        removeRequest();
    }
}

bool UdpIoScheduleImpl::removeRequest(int clientfd)
{
    pthread_mutex_lock(&mReleaseMutex);
    std::vector<UdpIoRequest *>::iterator itVec = mVecReleaseRequest.begin();
    for(; itVec != mVecReleaseRequest.end(); )
    {
        UdpIoRequest *request = *itVec;
        if (NULL != request) {
            if (0 < clientfd) {
                if (clientfd == request->Args.Bind.mSocket->descriptor()) {
                    itVec = mVecReleaseRequest.erase(itVec);
                    while(sendingOrRecving(request)) {
                        request->Args.Bind.mSocket->notifyRelease();
                        usleep(1000);
                        continue;
                    }
                    mReleaseCallback(request, mOwner);
                    delete request;
                    request = NULL;
                    std::cout << __FUNCTION__ << ":" << __LINE__ << " 被释放 fd = " << clientfd << std::endl;
                    pthread_mutex_unlock(&mReleaseMutex);
                    return true;
                }
                continue;
            }
            if (sendingOrRecving(request)) {
                request->Args.Bind.mSocket->notifyRelease();
                //updateHeartbeatPacket(request);
                ++itVec;
                continue;
            }
            mReleaseCallback(request, mOwner);
            printf("%s:%d 被释放 fd =%d\n", __FILE__, __LINE__,
                    request->Args.Bind.mSocket->descriptor());
            delete request;
            *itVec = NULL;
            request = NULL;
        }
        itVec = mVecReleaseRequest.erase(itVec);
    }
    pthread_mutex_unlock(&mReleaseMutex);
    return true;
}


void UdpIoScheduleImpl::updateHeartbeatPacket(UdpIoRequest *request)
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

void UdpIoScheduleImpl::setHeartbeatPacketIntervalTime(UdpIoRequest *request, int heartbeatPacketIntervalTime)
{
}

bool UdpIoScheduleImpl::sendingOrRecving(UdpIoRequest *request)
{
    return (request->mSending || request->Args.Bind.mSocket->mRecving);
}

bool UdpIoScheduleImpl::post(UdpIoRequest * request)
{
    return request->Args.Bind.mSocket->sendto(request);
}

bool UdpIoScheduleImpl::recvfromCallback(UdpIoRequest *request)
{
    return this->mRecvfromCallback(request, mOwner);
}

bool UdpIoScheduleImpl::isReleasing(UdpIoRequest * request)
{
    return request->Args.Bind.mSocket->mReleasing;
}

bool UdpIoScheduleImpl::sendto(UdpIoRequest * request)
{
    if (mStatus != UdpIoSchedule::eStarted) {
        return false;
    }
    return request->Args.Bind.mSocket->sendto(request);
}

};  // udpclient

