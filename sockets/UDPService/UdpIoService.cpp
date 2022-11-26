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
#include "sockets/udp/server/UdpIoService.h"
#include "sockets/udp/server/UdpIoScheduleImpl.h"

namespace udpserver
{
UdpIoService::UdpIoService(
        void (* callbackRecvfrom)(UdpMessage & message, sockets::AddressInfo & clientInfo, void * owner),
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
    if (NULL != mSched) {
        delete mSched;
        mSched = NULL;
    }
}

bool UdpIoService::recvfromCallback(UdpIoRequest * request, void * owner)
{
    ((UdpIoService *)owner)->recvfromCallback(request);
}

bool UdpIoService::recvfromCallback(UdpIoRequest * request)
{
    int aClientSocketFd = request->Args.Bind.mSocket->descriptor();
    UdpMessage aMessage(aClientSocketFd);
    aMessage.setData(request->Args.Recvfrom.mBuffer, request->Args.Recvfrom.mBufferSize);
    UdpClientInfo aClientInfo(request->Args.Recvfrom.mIP,
            request->Args.Recvfrom.mPort,
            request->Args.Recvfrom.mMac,
            aClientSocketFd,
            request->Args.Bind.mPort,
            request->Args.Bind.mMac,
            request->Args.Bind.mIP);
    aClientInfo.setRequest(request);
    void (*callbackRecvfrom)(UdpMessage & message, sockets::AddressInfo & clientInfo, void * owner)
    = (void (*)(UdpMessage & message, sockets::AddressInfo & clientInfo, void * owner))request->Args.Recvfrom.mCallbackRecv;
    if (NULL == callbackRecvfrom) {
        callbackRecvfrom = mRecvfromCallback;
    }
    void *owner = request->Args.mOwner;
    if (NULL == owner) {
        owner = mOwner;
    }
    callbackRecvfrom(aMessage, aClientInfo, owner);
    return true;
}

bool UdpIoService::preStart()
{
    mSched = new UdpIoScheduleImpl(&UdpIoService::recvfromCallback, &UdpIoService::releaseCallback, this);
    return (NULL != mSched);
}

bool UdpIoService::start(sockets::AddressInfo & clientInfo,
        void (* callbackRecvfrom)(UdpMessage & message, sockets::AddressInfo & clientInfo, void * owner),
        bool (* callbackRelease)(sockets::AddressInfo & tcpRelease, void * owner),
        void * owner)
{
    bool ret = false;
    if (NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return ret;
    }
    UdpIoRequest *request = mSched->allocRequest();
    request->Args.Bind.mMac = clientInfo.getLocalMacAddress();
    request->Args.Bind.mIP = clientInfo.getLocalIPAddress();
    request->Args.Bind.mPort = clientInfo.getLocalPort();
    if (NULL == owner) {
        if (NULL == mOwner) {
            mSched->freeRequest(request);
            return ret;
        }
        request->Args.mOwner = mOwner;
    }
    else {
        request->Args.mOwner = owner;
    }
    if (NULL == callbackRecvfrom) {
        if (NULL == mRecvfromCallback) {
            mSched->freeRequest(request);
            return ret;
        }
        request->Args.Recvfrom.mCallbackRecv = (void *)mRecvfromCallback;
    }
    else {
        request->Args.Recvfrom.mCallbackRecv = (void *)callbackRecvfrom;
    }
    if (NULL == callbackRelease) {
        if (NULL == mReleaseCallback) {
            mSched->freeRequest(request);
            return ret;
        }
        request->Args.mCallbackRelease = (void *)mReleaseCallback;
    }
    else {
        request->Args.mCallbackRelease = (void *)callbackRelease;
    }
    ret = mSched->start(request);
    if (ret) {
        clientInfo.setServerFd(request->Args.Bind.mSocket->descriptor());
        clientInfo.setLocalMacAddress(request->Args.Bind.mMac);
        clientInfo.setRequest(request);
    }
    return ret;
}

bool UdpIoService::stop(sockets::AddressInfo & clientInfo)
{
    int fd = clientInfo.getServerFd();
    if (fd <= 0) {
        return false;
    }
    UdpIoRequest *request = reinterpret_cast<UdpIoRequest *>(clientInfo.getRequest());
    if (NULL == request) {
        return false;
    }
    return mSched->stop(request);
}

bool UdpIoService::stopAll()
{
    if (NULL == mSched) {
        return false;
    }
    bool ret = mSched->stopAll();
    delete mSched;
    mSched = NULL;
    return ret;
}

bool UdpIoService::releaseCallback(UdpIoRequest * request, void * owner)
{
    return ((UdpIoService *)owner)->releaseCallback(request);
}

bool UdpIoService::releaseCallback(UdpIoRequest * request)
{
    if (NULL != mReleaseCallback) {
        UdpClientInfo tcpClient("", 0, "",
                request->Args.Bind.mSocket->descriptor(),
                request->Args.Bind.mPort, request->Args.Bind.mMac, request->Args.Bind.mIP);
        tcpClient.setRequest(request);
        return mReleaseCallback(tcpClient, mOwner);
    }
    return true;
}

bool UdpIoService::sendMsg(UdpMessage & message, sockets::AddressInfo & clientInfo)
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
    UdpIoRequest * request = reinterpret_cast<UdpIoRequest *>(clientInfo.getRequest());
    if (NULL == request) {
        printf("UdpIoService %s:%d request is NULL.\n", __FUNCTION__, __LINE__);
        return false;
    }
    request->Args.Sendto.mIP = clientInfo.getRemoteIPAddress();
    request->Args.Sendto.mPort = clientInfo.getRemotePort();
    if (request->post(data, dataSize, clientInfo.getRemoteIPAddress(), clientInfo.getRemotePort())) {
        // 重新初始化心跳包计数器和时间.
        mSched->updateHeartbeatPacket(request);
        printf("%s:%d send data successful.\n", __FUNCTION__, __LINE__);
        return true;
    }printf("%s:%d send data failed.\n", __FUNCTION__, __LINE__);
    return false;
}

void UdpIoService::setHeartbeatPacketIntervalTime(sockets::AddressInfo & clientInfo, int heartbeatPacketIntervalTime)
{
    if (NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return;
    }
    UdpIoRequest * request = reinterpret_cast<UdpIoRequest *>(clientInfo.getRequest());
    if (NULL == request) {
        return;
    }
    mSched->setHeartbeatPacketIntervalTime(request, heartbeatPacketIntervalTime);
}

bool UdpIoService::close(sockets::AddressInfo & clientInfo)
{
    if (NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return false;
    }
    UdpIoRequest * request = reinterpret_cast<UdpIoRequest *>(clientInfo.getRequest());
    if (NULL == request) {
        printf("request is NULL.\n");
        return false;
    }
    if (clientInfo.getLocalPort() == request->Args.Bind.mPort) {
        return mSched->stop(request);
    }
    return false;
}

};  // tcpserver

