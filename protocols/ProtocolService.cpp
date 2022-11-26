/*
 * =====================================================================================
 *
 *       Filename:  ProtocolService.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  05/17/2019 07:59:19 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/ProtocolService.h"

#include "protocol/protocol_def.h"

namespace protocols
{
ProtocolService::ProtocolService(NetworkService * netService)
 : mNetService(netService)
 , mUdpService(NULL)
 , mUdpClientService(NULL)
 , mTcpService(NULL)
 , mTcpClientService(NULL)
 , mResponse(new ResponseOtherPartyInfo())
 , mHandleClientData(new HandleClientData(this, mResponse))
 , mHandleServerData(new HandleServerData(this, mResponse, mHandleClientData))
{
    mNetService->setResponse(mResponse);
}

ProtocolService::~ProtocolService()
{
    stopUdpClient();
    stopUdpServer();
    stopTcpClient();
    stopTcpServer();
    if (NULL != mHandleServerData) {
        delete mHandleServerData;
        mHandleServerData = NULL;
    }
    if (NULL != mHandleClientData) {
        delete mHandleClientData;
        mHandleClientData = NULL;
    }
    if (NULL != mResponse) {
        delete mResponse;
        mResponse = NULL;
    }
}


/* udp server */
bool ProtocolService::startUdpServer(unsigned short localPort, std::string localIp)
{
    bool ret = false;
    mMutexUdpServer.lock();
    if (NULL == mUdpService) {
        mUdpService = new udpserver::UdpIoService(&ProtocolService::udpServerMessageCallback,
                &ProtocolService::releaseUdpServer, this);
        if (!mUdpService->preStart()) {
            delete mUdpService;
            mUdpService = NULL;
            mMutexUdpServer.unlock();
            return ret;
        }
        mResponse->setUdpService(mUdpService);
    }
    if (mMapUdpServerInfo.find(localPort) != mMapUdpServerInfo.end()) {
        mMutexUdpServer.unlock();
        return true;
    }
    sockets::AddressInfo * addressInfo = new udpserver::UdpClientInfo(localPort, localIp);
    try {
        ret = mUdpService->start(*addressInfo);
        if (ret) {
            mMapUdpServerInfo.insert(std::pair<unsigned short, sockets::AddressInfo *>(localPort, addressInfo));
            mMutexUdpServer.unlock();
            return ret;
        }
    }
    catch(std::exception & ex) {
        printf("error: %s.\n", ex.what());
    }
    catch(...)
    {}
    delete addressInfo;
    addressInfo = NULL;
    mMutexUdpServer.unlock();
    return ret;
}

bool ProtocolService::closeUdpServer(unsigned short localPort)
{
    bool ret = false;
    if (0 >= localPort) {
        return ret;
    }
    mMutexUdpServer.lock();
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapUdpServerInfo.find(localPort);
    if (it == mMapUdpServerInfo.end()) {
        mMutexUdpServer.unlock();
        return ret;
    }
    sockets::AddressInfo * addressInfo = it->second;
    if (NULL == addressInfo) {
        mMutexUdpServer.unlock();
        return ret;
    }
    if (NULL == mUdpService) {
        mMutexUdpServer.unlock();
        return ret;
    }
    ret = mUdpService->close(*addressInfo);
    if (ret) {
        mMapUdpServerInfo.erase(it);
        delete addressInfo;
        addressInfo = NULL;
    }
    if (0 >= mMapUdpServerInfo.size()) {
        delete mUdpService;
        mUdpService = NULL;
        mResponse->setUdpService(NULL);
    }
    mMutexUdpServer.unlock();
    return ret;
}

bool ProtocolService::stopUdpServer()
{
    bool ret = false;
    if (NULL == mUdpService) {
        return ret;
    }
    ret = mUdpService->stopAll();
    mMutexUdpServer.lock();
    delete mUdpService;
    mUdpService = NULL;
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapUdpServerInfo.begin();
    for (; it != mMapUdpServerInfo.end(); ) {
        sockets::AddressInfo * addressInfo = it->second;
        it = mMapUdpServerInfo.erase(it);
        delete addressInfo;
        addressInfo = NULL;
    }
    mResponse->setUdpService(NULL);
    mMutexUdpServer.unlock();
    return ret;
}

void ProtocolService::udpServerMessageCallback(udpserver::UdpMessage & message, sockets::AddressInfo & clientInfo, void * owner)
{
    (static_cast<ProtocolService*>(owner))->udpServerMessage(message, &clientInfo);
}

void ProtocolService::udpServerMessage(udpserver::UdpMessage & message, sockets::AddressInfo * clientInfo)
{
    protocols::SocketType socketType = protocols::UDP_SERVER;
    mHandleServerData->parseProtocolData(message.getData(), message.getDataSize(), clientInfo, socketType);
}

bool ProtocolService::releaseUdpServer(sockets::AddressInfo & udpClient, void * owner)
{
    return (static_cast<ProtocolService*>(owner))->releaseUdpServer(&udpClient);
}

bool ProtocolService::releaseUdpServer(sockets::AddressInfo * udpClient)
{
    printf("release udp server: %s:%d.\n",
            udpClient->getLocalIPAddress().c_str(),
            udpClient->getLocalPort());
    return mNetService->releaseUdpServer(udpClient);
}


/* udp client */
bool ProtocolService::startUdpClient(std::string & remoteIp, unsigned short remotePort, unsigned short localPort, std::string localIp)
{
    bool ret = false;
    mMutexUdpClient.lock();
    if (NULL == mUdpClientService) {
        mUdpClientService = new udpclient::UdpIoService(&ProtocolService::udpClientMessageCallback,
                &ProtocolService::releaseUdpClient, this);
        if (!mUdpClientService->preStart()) {
            delete mUdpClientService;
            mUdpClientService = NULL;
            mMutexUdpClient.unlock();
            return ret;
        }
        mResponse->setUdpClientService(mUdpClientService);
    }
    if (mMapUdpClientInfo.find(localPort) != mMapUdpClientInfo.end()) {
        mMutexUdpClient.unlock();
        return true;
    }
    sockets::AddressInfo * addressInfo = new udpclient::UdpServerInfo(remoteIp, remotePort, localPort, localIp);
    try {
        ret = mUdpClientService->start(*addressInfo);
        if (ret) {
            mMapUdpClientInfo.insert(std::pair<unsigned short, sockets::AddressInfo *>(localPort, addressInfo));
            mMutexUdpClient.unlock();
            return ret;
        }
    }
    catch(std::exception & ex) {
        printf("error: %s\n", ex.what());
    }
    catch(...)
    {}
    delete addressInfo;
    addressInfo = NULL;
    mMutexUdpClient.unlock();
    return ret;
}

bool ProtocolService::stopUdpClient()
{
    bool ret = false;
    if (NULL == mUdpClientService) {
        return ret;
    }
    ret = mUdpClientService->stopAll();
    mMutexUdpClient.lock();
    delete mUdpClientService;
    mUdpClientService = NULL;
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapUdpClientInfo.begin();
    for (; it != mMapUdpClientInfo.end(); ) {
        sockets::AddressInfo * addressInfo = it->second;
        it = mMapUdpClientInfo.erase(it);
        delete addressInfo;
        addressInfo = NULL;
    }
    mResponse->setUdpClientService(NULL);
    mMutexUdpClient.unlock();
    return ret;
}

void ProtocolService::udpClientMessageCallback(udpclient::UdpMessage & message, sockets::AddressInfo & serverInfo, void * owner)
{
    (static_cast<ProtocolService*>(owner))->udpClientMessage(message, &serverInfo);
}

void ProtocolService::udpClientMessage(udpclient::UdpMessage & message, sockets::AddressInfo * serverInfo)
{
    protocols::SocketType socketType = protocols::UDP_CLIENT;
    mHandleServerData->parseProtocolData(message.getData(), message.getDataSize(), serverInfo, socketType);
}

bool ProtocolService::releaseUdpClient(sockets::AddressInfo & udpServer, void * owner)
{
    return (static_cast<ProtocolService*>(owner))->releaseUdpClient(&udpServer);
}

bool ProtocolService::releaseUdpClient(sockets::AddressInfo * udpServer)
{
    unsigned short localPort = udpServer->getLocalPort();
    printf("release udp client, remoteAddress: %s:%d, localAddress: %s:%d.\n",
            udpServer->getRemoteIPAddress().c_str(),
            udpServer->getRemotePort(),
            udpServer->getLocalIPAddress().c_str(),
            localPort);
    mMutexUdpClient.lock();
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapUdpClientInfo.find(localPort);
    sockets::AddressInfo * address = it->second;
    mMapUdpClientInfo.erase(localPort);
    if (address != NULL) {
        address->setRequest(NULL);
        delete address;
        address = NULL;
    }
    mMutexUdpClient.unlock();
    return mNetService->releaseUdpClient(udpServer);
}


/* tcp server */
bool ProtocolService::startTcpServer(sockets::AddressInfo * tcpServerInfo)  // unsigned short localPort, std::string localIp)
{
    unsigned short localPort = tcpServerInfo->getLocalPort();
    std::string localIp(tcpServerInfo->getLocalIPAddress());
    std::string ipVersion(tcpServerInfo->getIpVersion());
    bool ret = false;
    mMutexTcpServer.lock();
    if (NULL == mTcpService) {
        mTcpService = new tcpserver::TcpIoService(&ProtocolService::tcpServerMessageCallback,
                &ProtocolService::releaseTcpServer, this);
        if (!mTcpService->preStart()) {
            delete mTcpService;
            mTcpService = NULL;
            mMutexTcpServer.unlock();
            return ret;
        }
        mResponse->setTcpService(mTcpService);
    }
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapTcpServerInfo.find(localPort);
    if (it != mMapTcpServerInfo.end()) {
        mMutexTcpServer.unlock();
        return true;
    }
    sockets::AddressInfo * addressInfo = new tcpserver::TcpClientInfo(localPort, localIp);
    addressInfo->setIpVersion(ipVersion);
    try {
        ret = mTcpService->start(*addressInfo);
        if (ret) {
            mMapTcpServerInfo.insert(std::pair<unsigned short, sockets::AddressInfo *>(localPort, addressInfo));
            mMutexTcpServer.unlock();
            return ret;
        }
    }
    catch(std::exception & ex) {
        printf("error: %s\n", ex.what());
    }
    catch(...)
    {}
    delete addressInfo;
    addressInfo = NULL;
    mMutexTcpServer.unlock();
    return ret;
}

bool ProtocolService::stopTcpServer()
{
    bool ret = false;
    if (NULL == mTcpService) {
        return ret;
    }
    ret = mTcpService->stop();
    mMutexTcpServer.lock();
    mResponse->setTcpService(NULL);
    delete mTcpService;
    mTcpService = NULL;
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapTcpServerInfo.begin();
    for (; it != mMapTcpServerInfo.end(); ) {
        sockets::AddressInfo * addressInfo = it->second;
        it = mMapTcpServerInfo.erase(it);
        delete addressInfo;
        addressInfo = NULL;
    }
    mMutexTcpServer.unlock();
    return ret;
}

void ProtocolService::tcpServerMessageCallback(tcpserver::TcpMessage & message, sockets::AddressInfo & clientInfo, void * owner)
{
    (static_cast<ProtocolService*>(owner))->tcpServerMessage(message, &clientInfo);
}

void ProtocolService::tcpServerMessage(tcpserver::TcpMessage & message, sockets::AddressInfo * clientInfo)
{
    protocols::SocketType socketType = protocols::TCP_SERVER;
    mHandleServerData->parseProtocolData(message.getData(), message.getDataSize(), clientInfo, socketType);
}

bool ProtocolService::closeClientSocketFd(sockets::AddressInfo * addressInfo, SocketType & socketType)
{
    bool ret = false;
    if (NULL == mTcpService) {
        return ret;
    }
    switch (socketType) {
        case UDP_SERVER:
            if (NULL == mUdpService) {
                break;
            }
            mMutexUdpServer.lock();
            mMutexUdpServer.unlock();
            break;
        case UDP_CLIENT:
            if (NULL == mUdpClientService) {
                break;
            }
            mMutexUdpClient.lock();
            mMutexUdpClient.unlock();
            break;
        case TCP_SERVER:
            if (NULL == mTcpService) {
                break;
            }
            mMutexTcpServer.lock();
            ret = mTcpService->close(tcpserver::TcpIoService::eClient, addressInfo);
            mMutexTcpServer.unlock();
            break;
        case TCP_CLIENT:
            if (NULL == mTcpClientService) {
                break;
            }
            mMutexTcpClient.lock();
            mMutexTcpClient.unlock();
            break;
    }
    return ret;
}

bool ProtocolService::releaseTcpServer(sockets::AddressInfo & tcpClient, void * owner)
{
    return (static_cast<ProtocolService*>(owner))->releaseTcpServer(&tcpClient);
}

bool ProtocolService::releaseTcpServer(sockets::AddressInfo * tcpClient)
{
    unsigned short localPort = tcpClient->getLocalPort();
    printf("release tcp server. localAddress: %s:%d, local mac:%s, remoteAddress:%s:%d, remote mac:%s.\n",
            tcpClient->getLocalIPAddress().c_str(),
            localPort,
            tcpClient->getLocalMacAddress().c_str(),
            tcpClient->getRemoteIPAddress().c_str(),
            tcpClient->getRemotePort(),
            tcpClient->getRemoteMacAddress().c_str());
    return mNetService->releaseTcpServer(tcpClient);
}


/* tcp client */
sockets::AddressInfo * ProtocolService::startTcpClient(sockets::AddressInfo * tcpClientInfo, bool isReconnect)
// std::string & remoteIp, unsigned short remotePort,
// unsigned short localPort, std::string localIp, bool isReconnect)
{
    std::string remoteIp(tcpClientInfo->getRemoteIPAddress());
    unsigned short remotePort = tcpClientInfo->getRemotePort();
    unsigned short localPort = tcpClientInfo->getLocalPort();
    std::string localIp(tcpClientInfo->getLocalIPAddress());

    mMutexTcpClient.lock();
    if (NULL == mTcpClientService) {
        mTcpClientService = new tcpclient::TcpIoService(this,
                &ProtocolService::tcpClientMessageCallback,
                &ProtocolService::tcpClientConnectCallback,
                &ProtocolService::releaseTcpClientCallback);
        if (!mTcpClientService->preStart()) {
            delete mTcpClientService;
            mTcpClientService = NULL;
            mMutexTcpClient.unlock();
            return NULL;
        }
        if (!mTcpClientService->start()) {
            delete mTcpClientService;
            mTcpClientService = NULL;
            mMutexTcpClient.unlock();
            return NULL;
        }
        mResponse->setTcpClientService(mTcpClientService);
    }
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapTcpClientInfo.find(localPort);
    if (it != mMapTcpClientInfo.end()) {
        sockets::AddressInfo * _addressInfo = it->second;
        if (isReconnect) {
            mMapTcpClientInfo.erase(localPort);
            mTcpClientService->close(*_addressInfo);
            delete _addressInfo;
        }
        else {
            mMutexTcpClient.unlock();
            return _addressInfo;
        }
    }
    sockets::AddressInfo * addressInfo = new tcpclient::TcpServerInfo(remoteIp, remotePort, "", -1);
    addressInfo->setLocalIPAddress(localIp);
    addressInfo->setLocalPort(localPort);
    addressInfo->setIpVersion(tcpClientInfo->getIpVersion());
    try {
        if (mTcpClientService->connect(*addressInfo, this)) {
            mMapTcpClientInfo.insert(std::pair<unsigned short, sockets::AddressInfo *>(localPort, addressInfo));
            mMutexTcpClient.unlock();
            return addressInfo;
        }
    }
    catch(std::exception & ex) {
        printf("error: %s\n", ex.what());
    }
    catch(...)
    {}
    delete addressInfo;
    addressInfo = NULL;
    mMutexTcpClient.unlock();
    return NULL;
}

bool ProtocolService::stopTcpClient()
{
    bool ret = false;
    if (NULL == mTcpClientService) {
        return ret;
    }
    ret = mTcpClientService->stopAll();
    // could give rise to deadlock, if mMutexTcpClient.lock() is before stopAll().
    mMutexTcpClient.lock();
    delete mTcpClientService;
    mTcpClientService = NULL;
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapTcpClientInfo.begin();
    for (; it != mMapTcpClientInfo.end(); ) {
        sockets::AddressInfo *addressInfo = it->second;
        it = mMapTcpClientInfo.erase(it);
        delete addressInfo;
        addressInfo = NULL;
    }
    mResponse->setTcpClientService(NULL);
    mMutexTcpClient.unlock();
    return ret;
}

bool ProtocolService::tcpClientMessageCallback(tcpclient::TcpMessage & message, sockets::AddressInfo & serverInfo, void * owner)
{
    return (static_cast<ProtocolService*>(owner))->tcpClientMessage(message, &serverInfo);
}

bool ProtocolService::tcpClientMessage(tcpclient::TcpMessage & message, sockets::AddressInfo * serverInfo)
{
    protocols::SocketType socketType = protocols::TCP_CLIENT;
    return mHandleServerData->parseProtocolData(message.getData(), message.getDataSize(), serverInfo, socketType);
}

bool ProtocolService::tcpClientConnectCallback(sockets::AddressInfo & tcpServer, bool isConnected, void * owner)
{
    return (static_cast<ProtocolService*>(owner))->tcpClientConnect(tcpServer, isConnected);
}

bool ProtocolService::tcpClientConnect(sockets::AddressInfo & tcpServer, bool isConnected)
{
    printf("connect remoteAddress: %s:%d. localAddress: %s:%d. isConnected: %d.\n",
            tcpServer.getRemoteIPAddress().c_str(), tcpServer.getRemotePort(),
            tcpServer.getLocalIPAddress().c_str(), tcpServer.getLocalPort(),
            isConnected);
    return mNetService->receiveTcpClientConnect(&tcpServer, isConnected);
}

bool ProtocolService::releaseTcpClientCallback(sockets::AddressInfo & tcpServer, void * owner)
{
    return (static_cast<ProtocolService*>(owner))->releaseTcpClient(&tcpServer);
}

bool ProtocolService::releaseTcpClient(sockets::AddressInfo * tcpServer)
{
    unsigned short localPort = tcpServer->getLocalPort();
    printf("release tcp client, remoteAddress: %s:%d, localAddress: %s:%d.\n",
            tcpServer->getRemoteIPAddress().c_str(),
            tcpServer->getRemotePort(),
            tcpServer->getLocalIPAddress().c_str(),
            localPort);
    // mMutexTcpClient.lock();
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapTcpClientInfo.find(localPort);
    // sockets::AddressInfo * address = it->second;
    mMapTcpClientInfo.erase(localPort);
    /*if (address != NULL) {
        address->setRequest(NULL);  // the line crashed.
        delete address;
        address = NULL;
    }*/
    // mMutexTcpClient.unlock();
    return mNetService->releaseTcpClient(tcpServer);
}

bool ProtocolService::receive(StorageData * receivedData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    return mNetService->receive(receivedData, addressInfo, socketType);
}

sockets::AddressInfo * ProtocolService::getUdpClientAddressInfo(unsigned short localPort)
{
    sockets::AddressInfo * addressInfo = NULL;
    mMutexUdpClient.lock();
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapUdpClientInfo.find(localPort);
    if (it == mMapUdpClientInfo.end()) {
        mMutexUdpClient.unlock();
        return addressInfo;
    }
    addressInfo = it->second;
    mMutexUdpClient.unlock();
    return addressInfo;
}

sockets::AddressInfo * ProtocolService::getTcpClientAddressInfo(unsigned short localPort)
{
    sockets::AddressInfo * addressInfo = NULL;
    mMutexTcpClient.lock();
    std::map<unsigned short, sockets::AddressInfo *>::iterator it = mMapTcpClientInfo.find(localPort);
    if (it == mMapTcpClientInfo.end()) {
        mMutexTcpClient.unlock();
        return addressInfo;
    }
    addressInfo = it->second;
    mMutexTcpClient.unlock();
    return addressInfo;
}

void ProtocolService::notifyRequestGroupIdFailed(sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    mNetService->notifyRequestGroupIdFailed(addressInfo, socketType);
}

void ProtocolService::notifyRequestGroupIdSuccess(const std::string & groupSId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    mNetService->notifyRequestGroupIdSuccess(groupSId, addressInfo, socketType);
}

bool ProtocolService::requestSequenceData(std::string & groupSId, int sequenceId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    return mNetService->requestSequenceData(groupSId, sequenceId, addressInfo, socketType);
}

bool ProtocolService::ackSequenceData(std::string & groupSId, int sequenceId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    return mNetService->ackSequenceData(groupSId, sequenceId, addressInfo, socketType);
}

bool ProtocolService::assembleComplete(std::string & groupSId, int maxSequenceId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    return mNetService->assembleComplete(groupSId, maxSequenceId, addressInfo, socketType);
}

bool ProtocolService::ackCompleted(std::string & groupSId, int maxSequenceId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    return mNetService->ackCompleted(groupSId, maxSequenceId, addressInfo, socketType);
}

/* client ---> server */
bool ProtocolService::reconnectedPeer(MessageBase * message)
{
    return mNetService->reconnectedPeer(message);
}

/* server ---> client */
bool ProtocolService::reconnectedPeerAck(MessageBase * message)
{
    return mNetService->reconnectedPeerAck(message);
}

/* server <---> client */
bool ProtocolService::closePeer(MessageBase * message)
{
    return mNetService->closePeer(message);
}

/* server <---> client */
bool ProtocolService::closePeerOver(MessageBase * message)
{
    return mNetService->closePeerOver(message);
}

/* server <---> client */
bool ProtocolService::closePeerOverAck(MessageBase * message)
{
    return mNetService->closePeerOverAck(message);
}

int ProtocolService::getClientGroupId(const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    return mNetService->getClientGroupId(addressInfo, socketType);
}

void ProtocolService::releaseMapGroup()
{
    mHandleClientData->releaseMapGroupClient();
    mHandleServerData->releaseMapGroupServer();
}

};  // namespace protocols
