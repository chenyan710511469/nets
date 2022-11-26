/*
 * =====================================================================================
 *
 *       Filename:  TcpIoSocket.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月05日 15时18分30秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */

#include "sockets/tcp/client/TcpIoSocket.h"

namespace tcpclient
{
TcpIoSocket::TcpIoSocket(TcpIoSchedule * sched, int fd)
 : mSched(sched)
 , mFd(fd)
{
    setIsBlocking(true);
    setSocketOpt();

    pthread_mutex_init(&mRecvMutex, NULL);
    pthread_cond_init(&mRecvCond, NULL);
}

TcpIoSocket::TcpIoSocket(TcpIoSchedule * sched, TcpIoRequest * request, int type, int protocol)
 : mSched(sched)
 , mFd(::socket(request->getFamily(), type, protocol))
 , mRecving(false)
{
    setRecvBlocking(false, 0);
    struct sockaddr_in clientAddr4, * serverAddr4 = NULL;
    struct sockaddr_in6 clientAddr6, * serverAddr6 = NULL;
    struct sockaddr * clientAddr = NULL, * serverAddr = NULL;
    socklen_t addrlen = 0;
    if (request->getFamily() == AF_INET) {
        addrlen = sizeof(struct sockaddr_in);
        memset(&clientAddr4, 0, addrlen);
        while (NULL == (serverAddr4 = (struct sockaddr_in *)malloc(addrlen + 1)));
        memset(serverAddr4, 0, addrlen + 1);
        clientAddr4.sin_family = request->getFamily();
        if (request->Args.Connect.mLocalIp.empty()) {
            clientAddr4.sin_addr.s_addr = INADDR_ANY;
        } else {
            clientAddr4.sin_addr.s_addr = inet_addr(request->Args.Connect.mLocalIp.c_str());
        }
        serverAddr4->sin_addr.s_addr = ::inet_addr(request->Args.Connect.mIP.c_str());
        clientAddr4.sin_port = htons(request->Args.Connect.mLocalPort);
        clientAddr = (struct sockaddr *)&clientAddr4;

        serverAddr4->sin_family = request->getFamily();
        serverAddr4->sin_addr.s_addr = inet_addr(request->Args.Connect.mIP.c_str());
        serverAddr4->sin_port = htons(request->Args.Connect.mPort);
        serverAddr = (struct sockaddr *)serverAddr4;
    }
    else if (request->getFamily() == AF_INET6) {
        addrlen = sizeof(struct sockaddr_in6);
        memset(&clientAddr6, 0, addrlen);
        while (NULL == (serverAddr6 = (struct sockaddr_in6 *)malloc(addrlen + 1)));
        memset(serverAddr6, 0, addrlen + 1);
        clientAddr6.sin6_family = request->getFamily();
        if (request->Args.Connect.mLocalIp.empty()) {
            clientAddr6.sin6_addr = in6addr_any;
        } else {
            inet_pton(request->getFamily(), request->Args.Connect.mLocalIp.c_str(), &(clientAddr6.sin6_addr));
        }
        clientAddr6.sin6_port = htons(request->Args.Connect.mLocalPort);
        clientAddr = (struct sockaddr *)&clientAddr6;

        serverAddr6->sin6_family = request->getFamily();
        inet_pton(request->getFamily(), request->Args.Connect.mIP.c_str(), &(serverAddr6->sin6_addr));
        serverAddr6->sin6_port = htons(request->Args.Connect.mPort);
        serverAddr = (struct sockaddr *)serverAddr6;
    }
    else {
        this->close();
        throw std::runtime_error("family is error.");
    }
    request->Args.Connect.mAddr = serverAddr;
    request->Args.Connect.mAddrLen = addrlen;
    if (0 > ::bind(mFd, clientAddr, addrlen)) {
        this->close();
        throw std::runtime_error(strerror(errno));
    }
    const int times = 5;
    for(int i = 0; i <= times; ++i) {
        if(0 != connect(mFd, serverAddr, addrlen)) {
            printf("error: %s, errno: %d.\n", strerror(errno), errno);
            if(i >= times) {
                this->close();
                char * error = strerror(errno);
                throw std::runtime_error(error);
            }
            usleep(10000);
            continue;
        }
        break;
    }
    request->Args.Connect.mSocket = this;

    pthread_mutex_init(&mRecvMutex, NULL);
    pthread_cond_init(&mRecvCond, NULL);
    initIfconf();
}

TcpIoSocket::~TcpIoSocket()
{
    close();
    pthread_cond_signal(&mRecvCond);
    mRecving = false;
    pthread_mutex_unlock(&mRecvMutex);

    pthread_cond_destroy(&mRecvCond);
    pthread_mutex_destroy(&mRecvMutex);
}

void TcpIoSocket::close()
{
    if(0 < mFd) {
        ::close(mFd);
        printf("%s:%d. socket fd = %d, is closed.\n", __FILE__, __LINE__, mFd);
        mFd = 0;
    }
}

int TcpIoSocket::descriptor()
{
    return mFd;
}

bool TcpIoSocket::setIsBlocking(bool isBlocking)
{
    int flags = -1;
    if(0 > (flags = fcntl(mFd, F_GETFL, 0))) {
        return false;
    }
    if(isBlocking) {
        flags = 0;// Set socket to blocking.
    }
    else {
        flags |= O_NONBLOCK; // Set socket to non-blocking.
    }
    if(0 > (flags = fcntl(mFd, F_SETFL, flags))) {
        std::cout << "flags = " << flags << std::endl;
        return false;
    }
    return true;
}

void TcpIoSocket::setSocketOpt()
{
    unsigned int aValue = 1;
    if(-1 == setsockopt(mFd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&aValue, sizeof(aValue)))
    {
        printf("%s:%d\n", __FILE__, __LINE__);
        std::cout << " error:" << ::strerror(errno) << std::endl;
        exit(-1);
    }
#ifdef MSG_NOSIGNAL
    // 非Linux操作系统时设置成 SO_NOSIGPIPE
    if(-1 == setsockopt(mFd, SOL_SOCKET, MSG_NOSIGNAL, (const char *)&aValue, sizeof(aValue)))
    {
        printf("%s:%d\n", __FILE__, __LINE__);
        std::cout << " error:" << ::strerror(errno) << std::endl;
    }
#endif
#ifdef SO_NOSIGPIPE
    if(-1 == setsockopt(mFd, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&aValue, sizeof(aValue)))
    {
        printf("%s:%d\n", __FILE__, __LINE__);
        std::cout << " error:" << ::strerror(errno) << std::endl;
    }
#endif
    // 不使用底层nagle cache, 立即发送数据
    if(-1 == setsockopt(mFd, IPPROTO_TCP, TCP_NODELAY, (const char *)&aValue, sizeof(aValue)))
    {
        printf("%s:%d\n", __FILE__, __LINE__);
        std::cout << " error:" << ::strerror(errno) << std::endl;
        exit(-1);
    }

    linger aLinger = {0, 0};
    if(-1 == setsockopt(mFd, SOL_SOCKET, SO_LINGER, (const char *)&aLinger, sizeof(linger)))
    {
        printf("%s:%d\n", __FILE__, __LINE__);
        std::cout << " error:" << ::strerror(errno) << std::endl;
        exit(-1);
    }
}

bool TcpIoSocket::send(TcpIoRequest * request)
{
#ifdef SIGALRM
    signal(SIGALRM, SIG_IGN);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif
    int sentBytes = 0;
    while(0 < mFd && sentBytes < request->Args.Send.mBufferSize)  // has crashed.
    {
        ssize_t aSentSize = ::send(mFd, request->Args.Send.mBuffer + sentBytes,
                request->Args.Send.mBufferSize - sentBytes,
                request->Args.Send.mFlags);
        if(0 > aSentSize) {
            if(!handleError(errno)) {
                request->mErrno = errno;
                if(EPIPE == request->mErrno) {
                    this->close();
                    mSched->releaseCallback(request);
                    return TcpIoSchedule::eErrorDontClose;
                }
            }
            break;
        }
        else {
            request->mErrno = 0;
            sentBytes += aSentSize;
        }
    }
    request->Args.Send.mSentBytes += sentBytes;
    if(0 >= mFd) {
        return false;
    }
    return (sentBytes == request->Args.Send.mBufferSize);
}

TcpIoSchedule::ReturnType TcpIoSocket::recv(TcpIoRequest * request)
{
#ifdef SIGALRM
    signal(SIGALRM, SIG_IGN);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif
    TcpIoSchedule::ReturnType returnType;
    const int aPckgSize = sizeof(request->Args.Recv.mBuffer);
    char aBufferRecv[aPckgSize];
    request->mErrno = 0;
    ssize_t aRet = 0;
    char * aSaveBuffer = NULL;
    request->Args.Recv.mFlags = MSG_DONTWAIT;
    memset(request->Args.Recv.mBuffer, 0, sizeof(aPckgSize));
    request->Args.Recv.mBufferSize = 0;
    request->Args.Recv.mReceivedBytes = 0;
    while(0 < mFd)
    {
        memset(aBufferRecv, 0, sizeof(aBufferRecv));
        aRet = ::recv(mFd, aBufferRecv, sizeof(aBufferRecv) - 1, request->Args.Recv.mFlags);
        if(0 >= aRet) {
            request->mErrno = errno;
            if(0 == aRet) {
                break;
            }
            if(!handleError(errno)) {
                if(EPIPE == request->mErrno) {
                    this->close();
                    mSched->releaseCallback(request);
                    return TcpIoSchedule::eErrorDontClose;
                }
            }
        }
        else {
repeat:
            if(0 == strncmp(aBufferRecv, request->HeartbeatPacket.mBuffer, request->HeartbeatPacket.mBufferSize))
            {// 是心跳包.
                request->init();
                std::string str(aBufferRecv);
                str.replace(0, request->HeartbeatPacket.mBufferSize, "");
                memset(aBufferRecv, 0, sizeof(aBufferRecv));
                strncpy(aBufferRecv, str.c_str(), str.length());
                // 回复服务端心跳包.
                ::send(mFd, request->HeartbeatPacket.mBuffer, request->HeartbeatPacket.mBufferSize, MSG_NOSIGNAL);
                if(0 < str.length())
                {
                    goto repeat;
                }
                return TcpIoSchedule::eSuccess;
            }
            request->Args.Recv.mReceivedBytes += aRet;
            memcpy(request->Args.Recv.mBuffer, aBufferRecv, aRet);
            request->Args.Recv.mBufferSize = request->Args.Recv.mReceivedBytes;
            printf("%s:%d  %s:%d.\n", __FUNCTION__, __LINE__,
            request->Args.Connect.mIP.c_str(), request->Args.Connect.mPort);
            mSched->recvCallback(request);
            memset(request->Args.Recv.mBuffer, 0, sizeof(aPckgSize));
            request->Args.Recv.mBufferSize = 0;
            request->Args.Recv.mReceivedBytes = 0;
        }
    }
    if(0 >= request->Args.Recv.mReceivedBytes) {
        std::cout << "没有数据." << std::endl;
        return TcpIoSchedule::eWithoutData;
    }
    request->Args.Recv.mReceivedBytes = 0;
    return TcpIoSchedule::eSuccess;
}

bool TcpIoSocket::setRecvBlocking(bool isBlocking, int timeOut)
{
    unsigned int aValue = 1;
    if(-1 == setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&(aValue), sizeof(aValue)))
    {
        printf("%s:%d\n", __FILE__, __LINE__);
        std::cout << " error:" << ::strerror(errno) << std::endl;
        exit(-1);
    }

    if(isBlocking) {
        int seconds = timeOut / 1000;
        int nanoseconds = (timeOut % 1000) * 1000000;
        struct timeval tv = {seconds, nanoseconds};
        return (0 == ::setsockopt(mFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)));
    }
    return true;
}

bool TcpIoSocket::setSendBlocking(bool isBlocking, int timeOut)
{
    if(isBlocking)
    {
        int seconds = timeOut / 1000;
        int nanoseconds = (timeOut % 1000) * 1000000;
        struct timeval tv = {seconds, nanoseconds};
        return (0 == ::setsockopt(mFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)));
    }
    return true;
}

bool TcpIoSocket::handleError(int theErrno)
{
    switch(theErrno)
    {
        case EINTR:
            return false;
        case EAGAIN:
        //case EWOULDBLOCK:
            return false;
        case ENOTSOCK:
            return false;
        case EPIPE:
            return false;
    }
    return true;
}

bool TcpIoSocket::requestRecv()
{
    pthread_mutex_lock(&mRecvMutex);
    if(mRecving) {
        struct timespec abstime;
        if(0 != clock_gettime(CLOCK_BOOTTIME, &abstime)) {
            std::cout << "clock_gettime() error." << std::endl;
        }
        abstime.tv_sec += 10;
        if (ETIMEDOUT == pthread_cond_timedwait(&mRecvCond, &mRecvMutex, &abstime)) {
            printf("%s:%d pthread_cond_timedwait timeout.\n", __FUNCTION__, __LINE__);
        }
    }
    mRecving = true;
    pthread_mutex_unlock(&mRecvMutex);
    if(mFd <= 0)
        return false;
    return true;
}

void TcpIoSocket::releaseRecv()
{
    mRecving = false;
    pthread_cond_signal(&mRecvCond);
}

bool TcpIoSocket::getAddressAndPort(TcpIoRequest *request)
{
    if(mFd <= 0) {
        return false;
    }
    int family = request->getFamily();
    struct sockaddr * addr = request->Args.Connect.mAddr;
    socklen_t addrLength = request->Args.Connect.mAddrLen;
    int aClientFd = request->Args.Connect.mSocket->descriptor();
    if(0 != ::getpeername(aClientFd, addr, (socklen_t *)&addrLength)) {
        fprintf(stderr, "%s:%d error: %s, errno: %d.\n", __FUNCTION__, __LINE__, strerror(errno), errno);
        return false;
    }
    {
        struct ifreq *ifFirst = mIfreq, *ifEnd = (struct ifreq *)((char *)mIfreq + mIfconf.ifc_len);
        for(; ifFirst < ifEnd; ++ifFirst)
        {
            int tmpFamily = ifFirst->ifr_addr.sa_family;
            if (tmpFamily != AF_INET && tmpFamily != AF_INET6)
            {continue;}

            struct arpreq arpReq;
            memset(&arpReq, 0, sizeof(struct arpreq));
            memcpy(&arpReq.arp_pa, addr, addrLength);
            strcpy(arpReq.arp_dev, ifFirst->ifr_name);
            arpReq.arp_pa.sa_family = AF_INET;
            arpReq.arp_ha.sa_family = AF_UNSPEC;
            if(0 > ::ioctl(aClientFd, SIOCGARP, &arpReq)) {
                fprintf(stderr, "error: %s, errno: %d\n", strerror(errno), errno);
                continue;
            }
            unsigned char* ptr = (unsigned char *)arpReq.arp_ha.sa_data;
            char buf[64] = {0};
            sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
            request->Args.Connect.mMac = std::string(buf);
            break;
        }
    }
    return true;
}

void TcpIoSocket::initIfconf()
{
    memset(&mIfreq, 0, sizeof(mIfreq));
    mIfconf.ifc_buf = (char *)mIfreq;
    mIfconf.ifc_len = sizeof(mIfreq);

    if(0 > ::ioctl(mFd, SIOCGIFCONF, &mIfconf)) {
        // fprintf(stderr, "error: %s, errno: %d\n", strerror(errno), errno);
        return;
    }
}

bool TcpIoSocket::replace(char *src, char *dest, int start, int count)
{
    return false;
}

};  // tcpclient

