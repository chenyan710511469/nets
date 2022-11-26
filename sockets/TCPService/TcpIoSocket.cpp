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

#include "sockets/tcp/server/TcpIoSocket.h"

namespace tcpserver
{
TcpIoSocket::TcpIoSocket(TcpIoSchedule * sched, int fd)
 : mSched(sched)
 , mFd(fd)
 , mRecving(false)
{
    setIsBlocking(true);
    setSocketOpt();
    pthread_mutex_init(&mAcceptMutex, NULL);

    pthread_mutex_init(&mRecvMutex, NULL);
    pthread_cond_init(&mRecvCond, NULL);
}

TcpIoSocket::TcpIoSocket(TcpIoSchedule * sched, int family, int type, int protocol)
 : mSched(sched)
 , mFd(::socket(family, type, protocol))
 , mRecving(false)
{
    pthread_mutex_init(&mAcceptMutex, NULL);

    pthread_mutex_init(&mRecvMutex, NULL);
    pthread_cond_init(&mRecvCond, NULL);
    initIfconf();
}

TcpIoSocket::~TcpIoSocket()
{
    pthread_mutex_unlock(&mAcceptMutex);
    close();
    pthread_cond_signal(&mRecvCond);
    mRecving = false;
    pthread_mutex_unlock(&mRecvMutex);

    pthread_mutex_destroy(&mAcceptMutex);

    pthread_cond_destroy(&mRecvCond);
    pthread_mutex_destroy(&mRecvMutex);
}

void TcpIoSocket::close()
{
    if(0 < mFd)
    {
        ::close(mFd);
        printf("%s:%d. socket fd = %d, is closed.\n",
                __FILE__, __LINE__, mFd);
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
    if(0 > (flags = fcntl(mFd, F_GETFL, 0)))
    {
        return false;
    }
    if(isBlocking)
    {
        flags = 0;// Set socket to blocking.
    }
    else
    {
        flags |= O_NONBLOCK; // Set socket to non-blocking.
    }
    if(0 > (flags = fcntl(mFd, F_SETFL, flags)))
    {
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

    linger aLinger = {1, 1};
    if(-1 == setsockopt(mFd, SOL_SOCKET, SO_LINGER, (const char *)&aLinger, sizeof(linger)))
    {
        printf("%s:%d\n", __FILE__, __LINE__);
        std::cout << " error:" << ::strerror(errno) << std::endl;
        exit(-1);
    }
}

bool TcpIoSocket::send(TcpIoRequest * request)
{
    // #ifdef SIGALRM
    signal(SIGALRM, SIG_IGN);
    // #endif
    // #ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    // #endif
    int sentBytes = 0;
    while(0 < mFd && sentBytes < request->Args.Send.mBufferSize)
    {
        ssize_t aSentSize = ::send(mFd, request->Args.Send.mBuffer + sentBytes,
                request->Args.Send.mBufferSize - sentBytes,
                request->Args.Send.mFlags);
        if(0 > aSentSize) {
            request->mErrno = errno;
            if(handleError(errno)) {
            }
            else if(EPIPE == request->mErrno) {
                this->close();
                mSched->releaseCallback(request);
                return false;
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
    // #ifdef SIGALRM
    signal(SIGALRM, SIG_IGN);
    // #endif
    // #ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    // #endif
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
        if(0 >= aRet)
        {
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
                if(0 < str.length()) {
                    goto repeat;
                }
                return TcpIoSchedule::eSuccess;
            }
            request->Args.Recv.mReceivedBytes += aRet;
            memcpy(request->Args.Recv.mBuffer, aBufferRecv, aRet);
            request->Args.Recv.mBufferSize = request->Args.Recv.mReceivedBytes;
            if(NULL != request->mRecvCallback)
            {
                printf("%s:%d  %s:%d.\n", __FUNCTION__, __LINE__,
                request->Args.Accept.mRemoteIP.c_str(), request->Args.Accept.mRemotePort);
                request->mRecvCallback(request, request->mOwner);
            }
            memset(request->Args.Recv.mBuffer, 0, sizeof(aPckgSize));
            request->Args.Recv.mBufferSize = 0;
            request->Args.Recv.mReceivedBytes = 0;
        }
    }
    if(0 >= request->Args.Recv.mReceivedBytes)
    {
        std::cout << "没有数据." << std::endl;
        return TcpIoSchedule::eWithoutData;
    }
    //request->mCounter += request->Args.Recv.mReceivedBytes;
    //printf("mCounter = %d.\n", request->mCounter);
    request->Args.Recv.mReceivedBytes = 0;
    return TcpIoSchedule::eSuccess;
}

bool TcpIoSocket::accept(TcpIoRequest *request)
{
    if(0 >= mFd) {
        return false;
    }
    // #ifdef SIGALRM
    signal(SIGALRM, SIG_IGN);
    // #endif
    // #ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    // #endif
    int clientSocketFd = 0;
    socklen_t addrlen = (socklen_t)request->Args.Accept.mAddrLen;
    pthread_mutex_lock(&mAcceptMutex);
    clientSocketFd = ::accept(mFd, request->Args.Accept.mAddr, &addrlen);
    pthread_mutex_unlock(&mAcceptMutex);
    if(0 >= clientSocketFd) {
        return false;
    }
    request->Args.Accept.mSocket = mSched->associate(clientSocketFd);
    // request->Args.Accept.mAddrLen = (unsigned int)addrlen;
    return true;
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

    if(isBlocking)
    {
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
    // std::cout << "error:" << strerror(errno) << " errno:" << theErrno << std::endl;
    switch(theErrno)
    {
        case EINTR:
            return false;
        case EAGAIN:
            return false;
        // case EWOULDBLOCK:
        //    return false;
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
        abstime.tv_sec += 6;
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

bool TcpIoSocket::getPeerAddressAndPort(TcpIoRequest *request)
{
    if (mFd <= 0) {
        return false;
    }
    int family = request->getFamily();
    struct sockaddr * addr = request->Args.Accept.mAddr;
    socklen_t addrLength = request->Args.Accept.mAddrLen;
    std::string ip;
    unsigned short port = 0;
    if (family == AF_INET6) {
        struct sockaddr_in6 * addr6 = (struct sockaddr_in6 *)addr;
        port = ntohs(addr6->sin6_port);
        char IPv6[INET6_ADDRSTRLEN + 1] = {0};
        if (0 != inet_ntop(family, (void *)&(addr6->sin6_addr), IPv6, INET6_ADDRSTRLEN)) {
            fprintf(stderr, "call inet_ntop failed, error: %s, errno: %d.\n", strerror(errno), errno);
        } else {
            ip = std::string(IPv6);
        }
    } else {
        struct sockaddr_in * addr4 = (struct sockaddr_in *)addr;
        ip = std::string(::inet_ntoa(addr4->sin_addr));
        port = ntohs(addr4->sin_port);
        // char IPv4[INET_ADDRSTRLEN + 1] = {0};
        // if (0 != inet_ntop(family, (void *)&(addr4->sin_addr), IPv4, INET_ADDRSTRLEN)) {
        //     printf("call inet_ntop failed, error: %s, errno: %d.\n", strerror(errno), errno);
        // }
    }
    printf("%s:%d remoteIP:%s, remotePort:%d.\n", __FUNCTION__, __LINE__, ip.c_str(), port);
    request->Args.Accept.mRemoteIP = ip;
    request->Args.Accept.mRemotePort = port;
    int clientFd = request->Args.Accept.mSocket->descriptor();
    if (0 != ::getpeername(clientFd, addr, &addrLength)) {
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
            arpReq.arp_pa.sa_family = family;
            arpReq.arp_ha.sa_family = AF_UNSPEC;
            if (0 > ::ioctl(clientFd, SIOCGARP, &arpReq)) {
                fprintf(stderr, "error: %s, errno: %d\n", strerror(errno), errno);
                continue;
            }
            unsigned char* ptr = (unsigned char *)arpReq.arp_ha.sa_data;
            char buf[64] = {0};
            sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
            request->Args.Accept.mRemoteMac = std::string(buf);
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
    for (int i = 0; i < ifreq_length; ++i) {
        mIfreq[i].ifr_hwaddr.sa_family = ARPHRD_ETHER;
    }
    if (0 > ::ioctl(mFd, SIOCGIFCONF, &mIfconf)) {
        // fprintf(stderr, "error: %s, errno: %d\n", strerror(errno), errno);
        return;
    }
}

};  // tcpserver

