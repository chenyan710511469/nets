/*
 * =====================================================================================
 *
 *       Filename:  UdpIoSocket.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2019年01月26日 11时40分31秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "sockets/udp/client/UdpIoSocket.h"

namespace udpclient
{
UdpIoSocket::UdpIoSocket(UdpIoSchedule * sched, int fd)
 : mSched(sched)
 , mFd(fd)
 , mRecving(false)
 , mReleasing(false)
{
    pthread_mutex_init(&mRecvMutex, NULL);
    pthread_cond_init(&mRecvCond, NULL);
    setIsBlocking(false);
    initIfconf();
}

UdpIoSocket::~UdpIoSocket()
{
    close();
    pthread_cond_destroy(&mRecvCond);
    pthread_mutex_destroy(&mRecvMutex);
}

void UdpIoSocket::close()
{
    ::close(mFd);
    mFd = -1;
}

int UdpIoSocket::descriptor()
{
    return mFd;
}

bool UdpIoSocket::setIsBlocking(bool isBlocking)
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

bool UdpIoSocket::recvfrom(UdpIoRequest * request)
{
    struct sockaddr_in srcAddr;
    socklen_t addrLen = sizeof(struct sockaddr_in);
    ::memset(&srcAddr, 0, addrLen);
    char buffer[sizeof(request->Args.Recvfrom.mBuffer)] = {0};
    int flags = request->Args.Recvfrom.mFlags;
    request->Args.Recvfrom.mBufferSize = 0;
    request->Args.Recvfrom.mReceivedBytes = 0;
    ssize_t receivedBytes = 0;
    while(!mReleasing && mRecving)
    {
        receivedBytes = ::recvfrom(mFd, buffer, sizeof(buffer) - 1, flags, (sockaddr*)&srcAddr, &addrLen);
        if(0 >= receivedBytes)
        {
            int anErrno = errno;
            handleError(anErrno);
            if(EPIPE == anErrno)
            {
                return false;
            }
            if(-1 == receivedBytes)
            {
                return false;
            }
            return true;
        }
        //printf("recv: %s\n", buffer);
        char *ip = ::inet_ntoa(srcAddr.sin_addr);
        uint16_t port = htons(srcAddr.sin_port);
        request->Args.Recvfrom.mIP = ip;
        request->Args.Recvfrom.mPort = port;
        ::memset(request->Args.Recvfrom.mBuffer, 0, sizeof(request->Args.Recvfrom.mBuffer));
        ::memcpy(request->Args.Recvfrom.mBuffer, buffer, receivedBytes);
        request->Args.Recvfrom.mReceivedBytes += receivedBytes;
        request->Args.Recvfrom.mBufferSize = request->Args.Recvfrom.mReceivedBytes;
        mSched->recvfromCallback(request);
        memset(request->Args.Recvfrom.mBuffer, 0, sizeof(request->Args.Recvfrom.mBuffer));
        request->Args.Recvfrom.mReceivedBytes = 0;
        request->Args.Recvfrom.mBufferSize = 0;
    }
    return true;
}

bool UdpIoSocket::sendto(UdpIoRequest * request)
{
    struct sockaddr_in remoteAddr;
    sockaddr *addr = NULL;
    socklen_t addrLen = 0;
    if(!request->Args.Sendto.mIP.empty())
    {
        addrLen = sizeof(struct sockaddr_in);
        ::memset(&remoteAddr, 0, addrLen);
        remoteAddr.sin_addr.s_addr = ::inet_addr(request->Args.Sendto.mIP.c_str());
        remoteAddr.sin_port = htons(request->Args.Sendto.mPort);
        addr = (sockaddr *)&remoteAddr;
    }
    unsigned int sentBytes = 0;
    while(0 < mFd && sentBytes < request->Args.Sendto.mBufferSize)
    {
        ssize_t aSentSize = ::sendto(mFd, request->Args.Sendto.mBuffer,
                request->Args.Sendto.mBufferSize - sentBytes,
                request->Args.Sendto.mFlags,
                addr, addrLen);
        if(0 >= aSentSize)
        {
            handleError(errno);
            if(-1 == aSentSize)
            {
                return false;
            }
            break;
        }
        else
        {
            sentBytes += aSentSize;
        }
        if(!mReleasing)
        {
            break;
        }
    }
    request->Args.Sendto.mSentBytes += sentBytes;
    return (sentBytes == request->Args.Sendto.mBufferSize);
}

bool UdpIoSocket::handleError(int theErrno)
{
    if (11 != theErrno) {
        printf("error:%s.\n", strerror(errno));
        printf("errno:%d.\n", theErrno);
    }
    switch(theErrno)
    {
        case EINTR:
            return false;
        case EAGAIN:
        //case EWOULDBLOCK:
            return false;
        case ENOTSOCK:
            return false;
    }
    return true;
}

bool UdpIoSocket::requestRecv()
{
    pthread_mutex_lock(&mRecvMutex);
    if(mRecving)
    {
        pthread_cond_wait(&mRecvCond, &mRecvMutex);
    }
    if(mReleasing)
    {
        pthread_mutex_unlock(&mRecvMutex);
        return false;
    }
    mRecving = true;
    pthread_mutex_unlock(&mRecvMutex);
    if(mFd <= 0)
        return false;
    return true;
}

void UdpIoSocket::releaseRecv()
{
    mRecving = false;
    pthread_cond_signal(&mRecvCond);
}

void UdpIoSocket::notifyRelease()
{
    mReleasing = true;
}

void UdpIoSocket::initIfconf()
{
    memset(&mIfreq, 0, sizeof(mIfreq));
    mIfconf.ifc_buf = (char *)mIfreq;
    mIfconf.ifc_len = sizeof(mIfreq);
    for(int i = 0; i < ifreq_length; ++i)
    {
        mIfreq[i].ifr_hwaddr.sa_family = ARPHRD_ETHER;
    }
    if(0 > ::ioctl(mFd, SIOCGIFCONF, &mIfconf))
    {
        // fprintf(stderr, "error: %s, errno: %d\n", strerror(errno), errno);
        return;
    }
}

void UdpIoSocket::getSelfAddress(UdpIoRequest * request)
{
    struct ifreq *ifFirst = mIfreq, *ifEnd = (struct ifreq *)((char *)mIfreq + mIfconf.ifc_len);
    struct sockaddr_in addr;
    int addrLength = sizeof(struct sockaddr_in);
    memset(&addr, 0, addrLength);
    for(; ifFirst < ifEnd; ++ifFirst)
    {
        if(ifFirst->ifr_addr.sa_family != AF_INET)
        {continue;}
        struct arpreq arpReq;
        memset(&arpReq, 0, sizeof(struct arpreq));
        memcpy((struct sockaddr_in *)&arpReq.arp_pa, (struct sockaddr_in *)&addr, addrLength);
        strcpy(arpReq.arp_dev, ifFirst->ifr_name);
        arpReq.arp_pa.sa_family = AF_INET;
        arpReq.arp_ha.sa_family = AF_UNSPEC;
        if(0 > ::ioctl(mFd, SIOCGARP, &arpReq))
        {
            // fprintf(stderr, "error: %s, errno: %d\n", strerror(errno), errno);
            continue;
        }
        unsigned char* ptr = (unsigned char *)arpReq.arp_ha.sa_data;
        char buf[64] = {0};
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
        request->Args.Bind.mMac = std::string(buf);
        break;
    }
}

bool UdpIoSocket::getPeerAddressAndPort(UdpIoRequest * request)
{
    if(mFd <= 0)
    {
        return false;
    }
    struct sockaddr_in addr;
    int addrLength = sizeof(struct sockaddr_in);
    memset(&addr, 0, addrLength);
    int aClientFd = request->Args.Bind.mSocket->descriptor();
    if(0 != ::getpeername(aClientFd, (struct sockaddr *)&addr, (socklen_t *)&addrLength))
    {
        // fprintf(stderr, "error: %s, errno: %d\n", strerror(errno), errno);
        return false;
    }
    std::string ip = std::string(::inet_ntoa(addr.sin_addr));
    unsigned short port = ntohs(addr.sin_port);
    request->Args.Sendto.mIP = ip;
    request->Args.Sendto.mPort = port;
    {
        struct ifreq *ifFirst = mIfreq, *ifEnd = (struct ifreq *)((char *)mIfreq + mIfconf.ifc_len);
        for(; ifFirst < ifEnd; ++ifFirst)
        {
            if(ifFirst->ifr_addr.sa_family != AF_INET)
            {continue;}
            struct arpreq arpReq;
            memset(&arpReq, 0, sizeof(struct arpreq));
            memcpy((struct sockaddr_in *)&arpReq.arp_pa, (struct sockaddr_in *)&addr, addrLength);
            strcpy(arpReq.arp_dev, ifFirst->ifr_name);
            arpReq.arp_pa.sa_family = AF_INET;
            arpReq.arp_ha.sa_family = AF_UNSPEC;
            if(0 > ::ioctl(mFd, SIOCGARP, &arpReq))
            {
                // fprintf(stderr, "error: %s, errno: %d\n", strerror(errno), errno);
                continue;
            }
            unsigned char* ptr = (unsigned char *)arpReq.arp_ha.sa_data;
            char buf[64] = {0};
            sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
            request->Args.Bind.mMac = std::string(buf);
            break;
        }
    }
    return true;
}

};  // udpclient
