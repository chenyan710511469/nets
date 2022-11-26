/*
 * =====================================================================================
 *
 *       Filename:  UdpIoService.cpp
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
#include "sockets/udp/client/UdpIoService.h"
#include "sockets/udp/client/UdpIoScheduleImpl.h"

namespace udpclient
{
UdpIoService::UdpIoService(
        void (* callbackRecvfrom)(UdpMessage & message, sockets::AddressInfo & serverInfo, void * owner),
        bool (* callbackRelease)(sockets::AddressInfo & tcpRelease, void * owner),
        void * owner)
 : mSched(NULL)
 , mRecvfromCallback(callbackRecvfrom)
 , mReleaseCallback(callbackRelease)
 , mOwner(owner)
{
}

UdpIoService::~UdpIoService()
{
    stopAll();
}

bool UdpIoService::recvfromCallback(UdpIoRequest * request, void * owner)
{
    ((UdpIoService *)owner)->recvfromCallback(request);
}

bool UdpIoService::recvfromCallback(UdpIoRequest * request)
{
    int aServerSocketFd = request->Args.Bind.mSocket->descriptor();
    UdpMessage aMessage(aServerSocketFd);
    aMessage.setData(request->Args.Recvfrom.mBuffer, request->Args.Recvfrom.mBufferSize);
    UdpServerInfo aServerInfo(request->Args.Recvfrom.mIP,
            request->Args.Recvfrom.mPort,
            request->Args.Recvfrom.mMac,
            aServerSocketFd,
            request->Args.Bind.mPort,
            request->Args.Bind.mMac,
            request->Args.Bind.mIP
        );
    aServerInfo.setRequest(request);
    void (*callbackRecvfrom)(UdpMessage & message, sockets::AddressInfo & serverInfo, void * owner)
    = (void (*)(UdpMessage & message, sockets::AddressInfo & serverInfo, void * owner))request->Args.Recvfrom.mCallbackRecv;
    if(NULL == callbackRecvfrom) {
        callbackRecvfrom = mRecvfromCallback;
    }
    void *owner = request->Args.mOwner;
    if(NULL == owner) {
        owner = mOwner;
    }
    callbackRecvfrom(aMessage, aServerInfo, owner);
    return true;
}

bool UdpIoService::preStart()
{
    mSched = new UdpIoScheduleImpl(&UdpIoService::recvfromCallback, &UdpIoService::releaseCallback, this);
    return (NULL != mSched);
}

bool UdpIoService::start(sockets::AddressInfo & serverInfo,
        void (* callbackRecvfrom)(UdpMessage & message, sockets::AddressInfo & serverInfo, void * owner),
        bool (* callbackRelease)(sockets::AddressInfo & tcpRelease, void * owner),
        void * owner)
{
    bool ret = false;
    if(NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return ret;
    }
    UdpIoRequest *request = mSched->allocRequest();
    request->Args.Bind.mIP = serverInfo.getLocalIPAddress();
    request->Args.Bind.mPort = serverInfo.getLocalPort();
    request->Args.Sendto.mIP = serverInfo.getRemoteIPAddress();
    request->Args.Sendto.mPort = serverInfo.getRemotePort();
    if(NULL == owner) {
        if(NULL == mOwner) {
            mSched->freeRequest(request);
            return ret;
        }
        request->Args.mOwner = mOwner;
    }
    else {
        request->Args.mOwner = owner;
    }
    if(NULL == callbackRecvfrom) {
        if(NULL == mRecvfromCallback) {
            mSched->freeRequest(request);
            return ret;
        }
        request->Args.Recvfrom.mCallbackRecv = (void *)mRecvfromCallback;
    }
    else {
        request->Args.Recvfrom.mCallbackRecv = (void *)callbackRecvfrom;
    }
    if(NULL == callbackRelease) {
        if(NULL == mReleaseCallback) {
            mSched->freeRequest(request);
            return ret;
        }
        request->Args.mCallbackRelease = (void *)mReleaseCallback;
    }
    else {
        request->Args.mCallbackRelease = (void *)callbackRelease;
    }
    ret = mSched->start(request);
    if(ret) {
        serverInfo.setClientFd(request->Args.Bind.mSocket->descriptor());
        serverInfo.setRemoteMacAddress(request->Args.Bind.mMac);
        serverInfo.setRequest(request);
    }
    return ret;
}

bool UdpIoService::stop(sockets::AddressInfo & serverInfo)
{
    int fd = serverInfo.getClientFd();
    if(fd <= 0) {
        return false;
    }
    UdpIoRequest *request = reinterpret_cast<UdpIoRequest *>(serverInfo.getRequest());
    if(NULL == request) {
        return false;
    }
    return mSched->stop(request);
}

bool UdpIoService::stopAll()
{
    if(NULL == mSched) {
        return false;
    }
    bool aRet = mSched->stopAll();
    delete mSched;
    mSched = NULL;
    return aRet;
}

bool UdpIoService::releaseCallback(UdpIoRequest * request, void * owner)
{
    return ((UdpIoService *)owner)->releaseCallback(request);
}

bool UdpIoService::releaseCallback(UdpIoRequest * request)
{
    if(NULL != mReleaseCallback) {
        UdpServerInfo udpServer(request->Args.Sendto.mIP,
                request->Args.Sendto.mPort,
                request->Args.Sendto.mMac,
                request->Args.Bind.mSocket->descriptor(),
                request->Args.Bind.mPort,
                request->Args.Bind.mMac,
                request->Args.Bind.mIP
            );
        udpServer.setRequest(request);
        return mReleaseCallback(udpServer, mOwner);
    }
    return true;
}

bool UdpIoService::sendMsg(UdpMessage & message, sockets::AddressInfo & serverInfo)
{
    if(NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return false;
    }
    const char * data = message.getData();
    const size_t dataSize = message.getDataSize();
    if (data == NULL || 0 >= dataSize) {
        printf("%s:%d. without data.", __FILE__, __LINE__);
        return false;
    }
    UdpIoRequest * request = reinterpret_cast<UdpIoRequest *>(serverInfo.getRequest());
    if (NULL == request) {
        printf("UdpIoService %s:%d request is NULL.\n", __FUNCTION__, __LINE__);
        return false;
    }
    if (request->post(data, dataSize, serverInfo.getRemoteIPAddress(), serverInfo.getRemotePort())) {
        // 重新初始化心跳包计数器和时间.
        mSched->updateHeartbeatPacket(request);
        printf("%s:%d send data successful.\n", __FUNCTION__, __LINE__);
        return true;
    }printf("%s:%d send data failed.\n", __FUNCTION__, __LINE__);
    return false;
}

void UdpIoService::setHeartbeatPacketIntervalTime(sockets::AddressInfo & serverInfo, int heartbeatPacketIntervalTime)
{
    if(NULL == mSched)
    {
        printf("error. Please call preStart() function first.\n");
        return;
    }
    UdpIoRequest *request = reinterpret_cast<UdpIoRequest *>(serverInfo.getRequest());
    if(NULL == request) {
        return;
    }
    mSched->setHeartbeatPacketIntervalTime(request, heartbeatPacketIntervalTime);
}


bool UdpIoService::close(sockets::AddressInfo & serverInfo)
{
    if(NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return false;
    }
    if(serverInfo.getLocalIPAddress().empty() && serverInfo.getLocalMacAddress().empty()) {
        printf("IP or MAC addresss can't be empty.");
        return false;
    }
    UdpIoRequest * request = reinterpret_cast<UdpIoRequest *>(serverInfo.getRequest());
    if(NULL == request) {
        printf("%s:%d request is NULL.\n", __FUNCTION__, __LINE__);
        return false;
    }
    return mSched->stop(request);
}

};  // tcpclient

