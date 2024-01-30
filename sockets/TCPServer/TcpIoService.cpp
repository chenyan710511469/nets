/*
 * =====================================================================================
 *
 *       Filename:  TcpIoService.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月09日 10时31分55秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#include "sockets/tcp/server/TcpIoService.h"
#include "sockets/tcp/server/TcpIoScheduleImpl.h"

namespace tcpserver
{
TcpIoService::TcpIoService(
        void (* callbackRecv)(TcpMessage & message, sockets::AddressInfo & clientInfo, void * owner),
        bool (* callbackRelease)(sockets::AddressInfo & tcpRelease, void * owner),
        void * owner)
 : mSched(NULL)
 , mPort(0)
 , mRecvCallback(callbackRecv)
 , mReleaseCallback(callbackRelease)
 , mOwner(owner)
{
}

TcpIoService::~TcpIoService()
{
    stop();
}

void TcpIoService::recvCallback(TcpIoRequest * request, void * owner)
{
    ((TcpIoService *)owner)->recvCallback(request);
}

void TcpIoService::recvCallback(TcpIoRequest * request)
{
    int clientSocketFd = request->Args.Accept.mSocket->descriptor();
    int serverSocketFd = request->getServerFd();
    TcpMessage aMessage(clientSocketFd);
    aMessage.setData(request->Args.Recv.mBuffer, request->Args.Recv.mBufferSize);
    TcpClientInfo clientInfo(request->Args.Accept.mRemoteIP, request->Args.Accept.mRemotePort,
            request->Args.Accept.mRemoteMac, serverSocketFd);
    clientInfo.setLocalIPAddress(request->Args.Bind.mLocalIp);
    clientInfo.setLocalPort(request->Args.Bind.mLocalPort);
    clientInfo.setLocalMacAddress(request->Args.Bind.mLocalMac);
    clientInfo.setClientFd(clientSocketFd);
    clientInfo.setRequest(request);
    int family = request->getFamily();
    if (family == AF_INET6) {
        clientInfo.setIpVersion(IPv6);
    }
    else {
        clientInfo.setIpVersion(IPv4);
    }
    printf("remoteIp:%s, remotePort:%d.\n", request->Args.Accept.mRemoteIP.c_str(), request->Args.Accept.mRemotePort);
    mRecvCallback(aMessage, clientInfo, mOwner);
}

bool TcpIoService::preStart(bool socketAcceptIsWait, int acceptTimeout)
{
    if (NULL != mSched) {
        return true;
    }
    mSched = new TcpIoScheduleImpl(&TcpIoService::recvCallback, &TcpIoService::releaseCallback, this);
    mSched->preStart(2, 10);
    mSched->setBlocking(socketAcceptIsWait, acceptTimeout);
    return (NULL != mSched);
}

bool TcpIoService::start(sockets::AddressInfo & clientInfo)
{
    if(NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return false;
    }
    std::string ipVersion(clientInfo.getIpVersion());
    std::string ip(clientInfo.getLocalIPAddress());
    struct sockaddr_in * addr4 = NULL;
    struct sockaddr_in6 * addr6 = NULL;
    socklen_t addrlen = 0;
    struct sockaddr * addr = NULL;
    int family = -1;
    if (0 == ipVersion.compare(IPv4)) {
        addrlen = sizeof(struct sockaddr_in);
        while (NULL == (addr4 = (struct sockaddr_in *)malloc(addrlen)));
        memset(addr4, 0, addrlen);
        family = AF_INET;
        addr4->sin_family = family;
        addr4->sin_port = htons(clientInfo.getLocalPort());
        if (ip.empty()) {
            addr4->sin_addr.s_addr = INADDR_ANY;
        } else {
            addr4->sin_addr.s_addr = inet_addr(ip.c_str());
        }
        addr = (struct sockaddr *)addr4;
    }
    else if (0 == ipVersion.compare(IPv6)) {
        addrlen = sizeof(struct sockaddr_in6);
        while (NULL == (addr6 = (struct sockaddr_in6 *)malloc(addrlen)));
        memset(addr6, 0, addrlen);
        family = AF_INET6;
        addr6->sin6_family = family;
        addr6->sin6_port = htons(clientInfo.getLocalPort());
        if (ip.empty()) {
            addr6->sin6_addr = in6addr_any;
        } else {
            inet_pton(family, ip.c_str(), &(addr6->sin6_addr));
        }
        addr = (struct sockaddr *)addr6;
    }
    else {
        printf("ip version is error in AddressInfo.\n");
        return false;
    }
    TcpIoRequest *request = mSched->allocRequest(family);
    request->Args.Bind.mAddr = addr;
    request->Args.Bind.mAddrLen = addrlen;
    request->Args.Bind.mLocalIp = ip;
    request->Args.Bind.mLocalPort = clientInfo.getLocalPort();
    request->Args.Bind.mLocalMac = clientInfo.getLocalMacAddress();
    bool ret = mSched->start(request);
    if (!ret) {
        mSched->stop(request);
        return ret;
    }
    clientInfo.setServerFd(request->Args.Bind.mSocket->descriptor());
    return ret;
}

bool TcpIoService::stop()
{
    if (NULL != mSched) {
        bool aRet = mSched->stopAll();
        delete mSched;
        mSched = NULL;
        return aRet;
    }
    return false;
}

bool TcpIoService::releaseCallback(TcpIoRequest * request, void * owner)
{
    return ((TcpIoService *)owner)->releaseCallback(request);
}

bool TcpIoService::releaseCallback(TcpIoRequest * request)
{
    if (NULL == mReleaseCallback) {
        return false;
    }
    int socketClientFd = 0;
    int socketServerFd = 0;
    if (NULL != request->Args.Accept.mSocket) {
        socketClientFd = request->Args.Accept.mSocket->descriptor();
    }
    if (NULL != request->Args.Bind.mSocket) {
        socketServerFd = request->Args.Bind.mSocket->descriptor();
    }
    TcpClientInfo tcpClient(request->Args.Accept.mRemoteIP, request->Args.Accept.mRemotePort,
            request->Args.Accept.mRemoteMac,
            socketServerFd);
    if (0 < socketClientFd) { // client fd
        tcpClient.setClientFd(socketClientFd);
    }
    if (0 < socketServerFd) { // server fd
        tcpClient.setServerFd(socketServerFd);
    }
    tcpClient.setLocalIPAddress(request->Args.Bind.mLocalIp);
    tcpClient.setLocalPort(request->Args.Bind.mLocalPort);
    tcpClient.setLocalMacAddress(request->Args.Bind.mLocalMac);
    tcpClient.setRequest(request);
    int family = request->getFamily();
    if (family == AF_INET6) {
        tcpClient.setIpVersion(IPv6);
    }
    else {
        tcpClient.setIpVersion(IPv4);
    }printf("%s:%s:%d release request.\n", __FILE__, __FUNCTION__, __LINE__);
    return mReleaseCallback(tcpClient, mOwner);
}

bool TcpIoService::sendMsg(TcpMessage & message, sockets::AddressInfo * addressInfo)
{
    if (NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return false;
    }
    const char * data = message.getData();
    const size_t dataSize = message.getDataSize();
    if (data == NULL || 0 >= dataSize) {
        printf("%s:%d. without data.", __FILE__, __LINE__);
        return false;
    }
    TcpIoRequest * request = reinterpret_cast<TcpIoRequest *>(addressInfo->getRequest());
    if (NULL == request) {
        return false;
    }
    if (request->post(data, dataSize)) {
        // 重新初始化心跳包计数器和时间.
        mSched->updateHeartbeatPacket(request);
        printf("%s:%d send data successful.\n", __FUNCTION__, __LINE__);
        return true;
    }printf("%s:%d send data failed.\n", __FUNCTION__, __LINE__);
    return false;
}

void TcpIoService::setHeartbeatPacketIntervalTime(int heartbeatPacketIntervalTime)
{
    if (NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return;
    }
    mSched->setHeartbeatPacketIntervalTime(heartbeatPacketIntervalTime);
}

bool TcpIoService::close(ServerOrClient type, sockets::AddressInfo * clientInfo)
{
    if (NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return false;
    }
    TcpIoRequest * request = reinterpret_cast<TcpIoRequest *>(clientInfo->getRequest());
    if (NULL == request) {
        printf("TcpIoService::%s:%d request is NULL.\n", __FUNCTION__, __LINE__);
        return false;
    }
    clientInfo->setRequest(NULL);
    printf("%s:%d setRequest(NULL).\n", __FUNCTION__, __LINE__);
    return mSched->stop(request);
}
};  // namespace tcpserver

