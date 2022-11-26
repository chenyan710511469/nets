/*
 * =====================================================================================
 *
 *       Filename:  NetworkManager.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/05/2019 07:16:58 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/NetworkManager.h"


namespace protocols
{
NetworkManager::NetworkManager(std::string xmlConfigPath)
 : mConfiguration(new Configuration())
 , mNetService(NULL)
 , mProtocolService(NULL)
 , mUdpServerState(false)
 , mUdpClientState(false)
 , mTcpServerState(false)
 , mTcpClientState(false)
{
    try
    {
        if (!mConfiguration->parseXmlConfig(xmlConfigPath)) {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX,
                    "parse config file failed: %s.\n", xmlConfigPath.c_str());
            delete mConfiguration;
            mConfiguration = NULL;
            throw std::runtime_error(buf);
        }
        printf("parse xml configuration file success.\n");
        mNetService = new NetworkService(this);
        mProtocolService = new ProtocolService(mNetService);
    }
    catch(std::exception & ex) {
        throw std::runtime_error(ex.what());
    }
    catch(...) {
        throw;
    }
}

NetworkManager::~NetworkManager()
{
    stopAll();
    if(NULL != mConfiguration) {
        delete mConfiguration;
        mConfiguration = NULL;
    }
    if (NULL != mProtocolService) {
        delete mProtocolService;
        mProtocolService = NULL;
    }
    if (NULL != mNetService) {
        delete mNetService;
        mNetService = NULL;
    }
}

bool NetworkManager::stopAll()
{
    stopUdpClient();
    stopUdpServer();
    stopTcpClient();
    stopTcpServer();
    if (NULL != mNetService) {
        return mNetService->stop();
    }
    return false;
}

bool NetworkManager::startUdpServer()
{
    printf("%s:%d.\n", __FUNCTION__, __LINE__);
    try {
        if (NULL == mNetService->getReceiver()) {
            char buf[SIXTY_FOUR] = {ZERO};
            snprintf(buf, SIXTY_FOUR, "receiver pointer can't be NULL.\n");
            throw std::runtime_error(buf);
        }
        if (NULL == mProtocolService) {
            char buf[SIXTY_FOUR] = {ZERO};
            snprintf(buf, SIXTY_FOUR, "mProtocolService pointer can't be NULL.\n");
            throw std::runtime_error(buf);
        }
        _udpserver * udpserver = mConfiguration->getUdpserver();
        std::map<unsigned short, _ip *> * ipMap = udpserver->getIpMap();
        if (ZERO >= ipMap->size()) {
            char buf[SIXTY_FOUR] = {ZERO};
            snprintf(buf, SIXTY_FOUR, "please configurate udp server ip address and port.\n");
            throw std::runtime_error(buf);
        }
        std::map<unsigned short, _ip *>::iterator itMap = ipMap->begin();
        bool ret1 = true;
        for (; itMap != ipMap->end(); ++itMap) {
            unsigned short localport = itMap->first;
            _ip * ip = itMap->second;
            int i = ZERO;
            bool ret2 = false;
            for (; i < ip->ports.size(); ++i) {
                unsigned short localPort = ip->ports[i].localPort;
                if (localport != localPort) {
                    continue;
                }
                if (mProtocolService->startUdpServer(localPort, ip->localIpAddress)) {
                    ip->ports[i].started = true;
                    ret2 = true;
                    printf("start success: localAddress: %s:%d.\n", ip->localIpAddress.c_str(), localPort);
                } else {
                    printf("error: localAddress: %s:%d.\n", ip->localIpAddress.c_str(), localPort);
                }
                break;
            }
            if (!ret2) {
                ret1 = ret2;
            }
        }
        mUdpServerState = ret1;
    }
    catch(std::exception & ex) {
        throw std::runtime_error(ex.what());
    }
    catch(...) {
        throw;
    }
    if (!mUdpServerState) {
        mProtocolService->stopUdpServer();
    }
    return mUdpServerState;
}

bool NetworkManager::stopUdpServer()
{
    if (mUdpServerState) {
        mUdpServerState = false;
        return mProtocolService->stopUdpServer();
    }
    return false;
}

bool NetworkManager::startUdpClient()
{
    printf("%s:%d.\n", __FUNCTION__, __LINE__);
    try {
        if (NULL == mNetService->getReceiver()) {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX,
                    "receiver pointer can't be NULL.\n");
            throw std::runtime_error(buf);
        }
        if (NULL == mProtocolService) {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX,
                    "mProtocolService pointer can't be NULL.\n");
            throw std::runtime_error(buf);
        }
        _udpclient * udpclient = mConfiguration->getUdpclient();
        std::map<unsigned short, _ip *> * ipMap = udpclient->getIpMap();
        if (ZERO >= ipMap->size()) {
            char buf[SIXTY_FOUR] = {ZERO};
            snprintf(buf, SIXTY_FOUR, "please configurate udp client ip address and port.\n");
            throw std::runtime_error(buf);
        }
        std::map<unsigned short, _ip *>::iterator itMap = ipMap->begin();
        bool ret1 = true;
        for (; itMap != ipMap->end(); ++itMap) {
            unsigned short localport = itMap->first;
            _ip * ip = itMap->second;
            int i = ZERO;
            bool ret2 = false;
            for (; i < ip->ports.size(); ++i) {
                unsigned short localPort = ip->ports[i].localPort;
                if (localport != localPort) {
                    continue;
                }
                std::string remoteIpAddress(ip->remoteIpAddress);
                unsigned short remotePort = ip->ports[i].remotePort;
                std::string localIpAddress(ip->localIpAddress);
                if (udpClientBind(remoteIpAddress, remotePort, localPort, localIpAddress)) {
                    ip->ports[i].started = true;
                    ret2 = true;
                    printf("start success: remoteAddress: %s:%d, localAddress:%s:%d.\n",
                        remoteIpAddress.c_str(), remotePort, localIpAddress.c_str(), localPort);
                } else {
                    printf("error: remoteAddress: %s:%d, localAddress:%s:%d.\n",
                        remoteIpAddress.c_str(), remotePort, localIpAddress.c_str(), localPort);
                }
                break;
            }
            if (!ret2) {
                ret1 = ret2;
            }
        }
        mUdpClientState = ret1;
    }
    catch(std::exception & ex) {
        throw std::runtime_error(ex.what());
    }
    catch(...) {
        throw;
    }
    if (!mUdpClientState) {
        mProtocolService->stopUdpClient();
    }
    return mUdpClientState;
}

bool NetworkManager::stopUdpClient()
{
    if (mUdpClientState) {
        mUdpClientState = false;
        return mProtocolService->stopUdpClient();
    }
    return false;
}

bool NetworkManager::startTcpServer()
{
    printf("%s:%d.\n", __FUNCTION__, __LINE__);
    try {
        if (NULL == mNetService->getReceiver()) {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX, "receiver pointer can't be NULL.\n");
            throw std::runtime_error(buf);
        }
        if (NULL == mProtocolService) {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX, "mProtocolService pointer can't be NULL.\n");
            throw std::runtime_error(buf);
        }
        _tcpserver * tcpserver = mConfiguration->getTcpserver();
        std::map<unsigned short, _ip *> * ipMap = tcpserver->getIpMap();
        if (ZERO >= ipMap->size()) {
            char buf[SIXTY_FOUR] = {ZERO};
            snprintf(buf, SIXTY_FOUR, "please configurate tcp server ip address and port.\n");
            throw std::runtime_error(buf);
        }
        std::map<unsigned short, _ip *>::iterator itMap = ipMap->begin();
        bool ret1 = true;
        for (; itMap != ipMap->end(); ++itMap) {
            unsigned short localport = itMap->first;
            _ip * ip = itMap->second;
            int i = ZERO;
            bool ret2 = false;
            for (; i < ip->ports.size(); ++i) {
                unsigned short localPort2 = ip->ports[i].localPort;
                if (localport != localPort2) {
                    continue;
                }
                tcpserver::TcpClientInfo address(localPort2, ip->localIpAddress);
                sockets::AddressInfo * addressBase = &address;
                addressBase->setIpVersion(ip->ipVersion);
                if (mProtocolService->startTcpServer(&address)) {
                    ip->ports[i].started = true;
                    ret2 = true;
                    printf("start tcp server success: %s:%d.\n", ip->localIpAddress.c_str(), localPort2);
                } else {
                    printf("error: localAddress: %s:%d.\n", ip->localIpAddress.c_str(), localPort2);
                }
                break;
            }
            if (!ret2) {
                ret1 = ret2;
            }
        }
        mTcpServerState = ret1;
    }
    catch(std::exception & ex) {
        throw std::runtime_error(ex.what());
    }
    catch(...) {
        throw;
    }
    if (!mTcpServerState) {
        mProtocolService->stopTcpServer();
    }
    return mTcpServerState;
}

bool NetworkManager::stopTcpServer()
{
    if (mTcpServerState) {
        mTcpServerState = false;
        return mProtocolService->stopTcpServer();
    }
    return false;
}

bool NetworkManager::startTcpClient()
{
    printf("%s:%d.\n", __FUNCTION__, __LINE__);
    try {
        if (NULL == mNetService->getReceiver()) {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX, "receiver pointer can't be NULL.\n");
            throw std::runtime_error(buf);
        }
        if (NULL == mProtocolService) {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX, "receiver pointer can't be NULL.\n");
            throw std::runtime_error(buf);
        }
        _tcpclient * tcpclient = mConfiguration->getTcpclient();
        std::map<unsigned short, _ip *> * ipMap = tcpclient->getIpMap();
        if (ZERO >= ipMap->size()) {
            char buf[SIXTY_FOUR] = {ZERO};
            snprintf(buf, SIXTY_FOUR, "please configurate tcp client ip address and port.\n");
            throw std::runtime_error(buf);
        }
        std::map<unsigned short, _ip *>::iterator itMap = ipMap->begin();
        bool ret1 = true;
        for (; itMap != ipMap->end(); ++itMap) {
            unsigned short localport = itMap->first;
            _ip * ip = itMap->second;
            int i = ZERO;
            bool ret2 = false;
            for (; i < ip->ports.size(); ++i) {
                unsigned short localPort2 = ip->ports[i].localPort;
                if (localport != localPort2) {
                    continue;
                }
                tcpclient::TcpServerInfo address(ip->remoteIpAddress, ip->ports[i].remotePort);
                sockets::AddressInfo * addressBase = &address;
                addressBase->setLocalIPAddress(ip->localIpAddress);
                addressBase->setLocalPort(localPort2);
                addressBase->setIpVersion(ip->ipVersion);
                if (NULL != tcpClientConnect(&address)) {
                    ip->ports[i].started = true;
                    ret2 = true;
                    printf("start success: remoteAddress: %s:%d, localAddress:%s:%d.\n",
                        ip->remoteIpAddress.c_str(),
                        ip->ports[i].remotePort,
                        ip->localIpAddress.c_str(),
                        localPort2);
                } else {
                    printf("error: remoteAddress: %s:%d, localAddress:%s:%d.\n",
                        ip->remoteIpAddress.c_str(),
                        ip->ports[i].remotePort,
                        ip->localIpAddress.c_str(),
                        localPort2);
                }
                break;
            }
            if (!ret2) {
                ret1 = ret2;
            }
        }
        mTcpClientState = ret1;
    }
    catch(std::exception & ex) {
        throw std::runtime_error(ex.what());
    }
    catch(...) {
        throw;
    }
    if (!mTcpClientState) {
        mProtocolService->stopTcpClient();
    }
    return mTcpClientState;
}

bool NetworkManager::stopTcpClient()
{
    if (mTcpClientState) {
        mTcpClientState = false;
        return mProtocolService->stopTcpClient();
    }
    return false;
}

bool NetworkManager::preStart(ReceiverBase * receiver)
{
    try {
        if (NULL != mNetService->getReceiver()) {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX,
                    "receiver pointer already exists. preStart function can be called only once.\n");
            throw std::runtime_error(buf);
        }
        if (NULL == receiver) {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX, "receiver pointer can't be NULL.\n");
            throw std::runtime_error(buf);
        }
        return mNetService->preStart(receiver);
    }
    catch(std::exception & ex) {
        throw std::runtime_error(ex.what());
    }
    catch(...) {
        throw;
    }
    return false;
}

int NetworkManager::getConfigGroupId(unsigned short localPort, SocketType & socketType)
{
    int configGroupId = MINUS_ONE;
    std::map<unsigned short, _ip *>::iterator it;
    switch (socketType) {
        case UDP_SERVER: {
            std::map<unsigned short, _ip *> * ipMap = mConfiguration->getUdpserver()->getIpMap();
            it = ipMap->find(localPort);
            if (it != ipMap->end()) {
                configGroupId = it->second->configGroupId;
            }
            break;
        }
        case UDP_CLIENT: {
            std::map<unsigned short, _ip *> * ipMap = mConfiguration->getUdpclient()->getIpMap();
            it = ipMap->find(localPort);
            if (it != ipMap->end()) {
                configGroupId = it->second->configGroupId;
            }
            break;
        }
        case TCP_SERVER: {
            std::map<unsigned short, _ip *> * ipMap = mConfiguration->getTcpserver()->getIpMap();
            it = ipMap->find(localPort);
            if (it != ipMap->end()) {
                configGroupId = it->second->configGroupId;
            }
            break;
        }
        case TCP_CLIENT: {
            std::map<unsigned short, _ip *> * ipMap = mConfiguration->getTcpclient()->getIpMap();
            it = ipMap->find(localPort);
            if (it != ipMap->end()) {
                configGroupId = it->second->configGroupId;
            }
            break;
        }
    }
    return configGroupId;
}

sockets::AddressInfo * NetworkManager::tcpClientConnect(sockets::AddressInfo * tcpClientInfo, bool isReconnect)
// std::string & remoteIp, unsigned short remotePort,
//         unsigned short localPort, std::string & localIp, bool isReconnect)
{
    return mProtocolService->startTcpClient(tcpClientInfo, isReconnect);
}

bool NetworkManager::udpClientBind(std::string & remoteIp, unsigned short remotePort, unsigned short localPort, std::string & localIp)
{
    return mProtocolService->startUdpClient(remoteIp, remotePort, localPort, localIp);
}

SenderBase * NetworkManager::createSenderForTcpClient(int configGroupId, std::string remoteIp, unsigned short remotePort)
{
    return mNetService->createSenderForTcpClient(configGroupId, remoteIp, remotePort);
}

SenderBase * NetworkManager::createSenderForUdpClient(int configGroupId, std::string remoteIp, unsigned short remotePort)
{
    return mNetService->createSenderForUdpClient(configGroupId, remoteIp, remotePort);
}

};  // namespace protocols

