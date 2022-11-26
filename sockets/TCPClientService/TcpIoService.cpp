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
#include "sockets/tcp/client/TcpIoService.h"
#include "sockets/tcp/client/TcpIoScheduleImpl.h"

namespace tcpclient
{
TcpIoService::TcpIoService(void *owner,
        bool (*callbackRecv)(TcpMessage & message, sockets::AddressInfo & serverInfo, void *owner),
        bool (*callbackConnect)(sockets::AddressInfo & serverInfo, bool isConnected, void *owner),
        bool (*callbackRelease)(sockets::AddressInfo & tcpRelease, void *owner))
 : mSched(NULL)
 , mCallbackRecv(callbackRecv)
 , mCallbackConnect(callbackConnect)
 , mCallbackRelease(callbackRelease)
 , mOwner(owner)
{
}

TcpIoService::~TcpIoService()
{
    stopAll();
}

bool TcpIoService::recvCallback(TcpIoRequest *request, void *owner)
{
    return ((TcpIoService *)owner)->recvCallback(request);
}

bool TcpIoService::recvCallback(TcpIoRequest *request)
{
    int aClientFd = request->Args.Connect.mSocket->descriptor();
    TcpMessage aMessage(aClientFd);
    aMessage.setData(request->Args.Recv.mBuffer, request->Args.Recv.mBufferSize);
    TcpServerInfo serverInfo(request->Args.Connect.mIP,
            request->Args.Connect.mPort,
            request->Args.Connect.mMac,
            aClientFd);
    serverInfo.setLocalIPAddress(request->Args.Connect.mLocalIp);
    serverInfo.setLocalPort(request->Args.Connect.mLocalPort);
    serverInfo.setRequest(request);
printf("remoteIp:%s, remotePort:%d.\n", request->Args.Connect.mIP.c_str(), request->Args.Connect.mPort);
    bool (*callbackRecv)(TcpMessage & message, sockets::AddressInfo & serverInfo, void * owner)
        = (bool (*)(TcpMessage&, sockets::AddressInfo&, void*))request->Args.Recv.mCallbackRecv;
    void *owner = request->Args.mOwner;
    return callbackRecv(aMessage, serverInfo, owner);
}

bool TcpIoService::preStart(bool socketIsWait, int timeout)
{
    mSched = new TcpIoScheduleImpl(&recvCallback, &connectCallback, &releaseCallback, this);
    mSched->preStart();
    mSched->setBlocking(socketIsWait, timeout);
    return (NULL != mSched);
}

bool TcpIoService::start()
{
    if(NULL == mSched)
    {
        printf("error. Please call preStart() function first.\n");
        return false;
    }
    return mSched->start();
}

bool TcpIoService::stopAll()
{
    if(NULL != mSched) {
        bool aRet = mSched->stopAll();
        delete mSched;
        mSched = NULL;
        return aRet;
    }
    return false;
}

bool TcpIoService::releaseCallback(TcpIoRequest *request, void *owner)
{
    return ((TcpIoService *)owner)->releaseCallback(request);
}

bool TcpIoService::releaseCallback(TcpIoRequest *request)
{
    TcpServerInfo tcpServer(request->Args.Connect.mIP, request->Args.Connect.mPort,
            request->Args.Connect.mMac,
            request->Args.Connect.mSocket->descriptor());
    tcpServer.setLocalIPAddress(request->Args.Connect.mLocalIp);
    tcpServer.setLocalPort(request->Args.Connect.mLocalPort);
    tcpServer.setRequest(request);
    bool (*callbackRelease)(sockets::AddressInfo & tcpRelease, void *owner)
        = (bool (*)(sockets::AddressInfo & tcpRelease, void *owner))request->Args.mCallbackRelease;
    void *owner = request->Args.mOwner;
    return callbackRelease(tcpServer, owner);
}

bool TcpIoService::sendMsg(TcpMessage & message, sockets::AddressInfo * addressInfo)
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
    TcpIoRequest *request = reinterpret_cast<TcpIoRequest *>(addressInfo->getRequest());
    if(NULL == request) {
        return false;
    }
    if (request->post(data, dataSize)) {
        // 重新初始化心跳包计数器和时间.
        mSched->updateHeartbeatPacket(request);
        return true;
    }
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

bool TcpIoService::stop(sockets::AddressInfo & serverInfo)
{
    if (NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return false;
    }
    TcpIoRequest *request = reinterpret_cast<TcpIoRequest *>(serverInfo.getRequest());
    if (NULL == request) {
        printf("request is NULL.\n");
        return false;
    }
    return mSched->stop(request);
}

bool TcpIoService::close(sockets::AddressInfo & serverInfo)
{
    if (NULL == mSched) {
        printf("error. Please call preStart() function first.\n");
        return false;
    }
    TcpIoRequest *request = reinterpret_cast<TcpIoRequest *>(serverInfo.getRequest());
    if (NULL == request) {
        printf("TcpIoService::%s:%d request is NULL.\n", __FUNCTION__, __LINE__);
        return false;
    }
    return mSched->close(request);
}

bool TcpIoService::connect(sockets::AddressInfo & serverInfo, void *owner,
        bool (*callbackRecv)(TcpMessage & message, sockets::AddressInfo & serverInfo, void *owner),
        bool (*callbackConnect)(sockets::AddressInfo & serverInfo, bool isConnected, void *owner),
        bool (*callbackRelease)(sockets::AddressInfo & tcpRelease, void *owner))
{
    if (NULL == owner) {
        if (NULL == mOwner) {
            return false;
        }
        owner = mOwner;
    }
    if (NULL == callbackRecv) {
        if (NULL == mCallbackRecv) {
            return false;
        }
        callbackRecv = mCallbackRecv;
    }
    if (NULL == callbackConnect) {
        if (NULL == mCallbackConnect) {
            return false;
        }
        callbackConnect = mCallbackConnect;
    }
    if (NULL == callbackRelease) {
        if (NULL == mCallbackRelease) {
            callbackRelease = mCallbackRelease;
        }
        callbackRelease = mCallbackRelease;
    }
    if (serverInfo.getRemoteIPAddress().empty()) {
        return false;
    }
    if (0 >= serverInfo.getRemotePort()) {
        return false;
    }
    std::string ipVersion(serverInfo.getIpVersion());
    int family = -1;
    if (0 == ipVersion.compare(IPv6)) {
        family = AF_INET6;
    }
    else {
        family = AF_INET;
    }
    TcpIoRequest *request = mSched->allocRequest(family);
    if (NULL == request) {
        return false;
    }
    request->Args.mOwner = owner;
    request->Args.mCallbackRelease = (void*)callbackRelease;
    request->Args.Recv.mCallbackRecv = (void*)callbackRecv;
    request->Args.Connect.mCallbackConnect = (void*)callbackConnect;
    request->Args.Connect.mIP = serverInfo.getRemoteIPAddress();
    request->Args.Connect.mPort = serverInfo.getRemotePort();
    request->Args.Connect.mLocalIp = serverInfo.getLocalIPAddress();
    request->Args.Connect.mLocalPort = serverInfo.getLocalPort();
    bool ret = false;
    try {
        ret = mSched->connect(request);
    }
    catch (std::exception & ex) {
        mSched->stop(request);
        printf("error:%s.\n", ex.what());
        return false;
    }
    catch (...) {
        mSched->stop(request);
        return false;
    }
    if (ret) {
        serverInfo.setRemoteMacAddress(request->Args.Connect.mMac);
        serverInfo.setClientFd(request->Args.Connect.mSocket->descriptor());
        serverInfo.setRequest(request);
    }
    TcpIoService::connectCallback(request, ret, this);
    return ret;
}

bool TcpIoService::connectCallback(TcpIoRequest *request, bool isConnected, void *owner)
{
    return ((TcpIoService *)owner)->connectCallback(request, isConnected);
}

bool TcpIoService::connectCallback(TcpIoRequest *request, bool isConnected)
{
    TcpServerInfo serverInfo(request->Args.Connect.mIP, request->Args.Connect.mPort,
            request->Args.Connect.mMac,
            request->Args.Connect.mSocket->descriptor());
    serverInfo.setLocalIPAddress(request->Args.Connect.mLocalIp);
    serverInfo.setLocalPort(request->Args.Connect.mLocalPort);
    serverInfo.setRequest(request);
    if (AF_INET6 == request->getFamily()) {
        serverInfo.setIpVersion(IPv6);
    }
    else {
        serverInfo.setIpVersion(IPv4);
    }
    bool (*callbackConnect)(sockets::AddressInfo & serverInfo, bool isConnected, void *owner)
    = (bool (*)(sockets::AddressInfo & serverInfo, bool isConnected, void *owner))request->Args.Connect.mCallbackConnect;
    void *owner = request->Args.mOwner;
    return callbackConnect(serverInfo, isConnected, owner);
}

};  // tcpclient

