/*
 * =====================================================================================
 *
 *       Filename:  NetworkManager.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/05/2019 07:18:33 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_NETWORK_MANAGER_H
#define PROTOCOLS_NETWORK_MANAGER_H

#include <string>

#include "protocol/SenderBase.h"
#include "protocol/Configuration.h"
#include "protocol/NetworkService.h"
#include "protocol/ReceiverBase.h"
#include "protocol/ProtocolService.h"
#include "protocol/protocol_def.h"

#include "sockets/tcp/server/TcpClientInfo.h"
#include "sockets/tcp/client/TcpServerInfo.h"
#include "sockets/udp/server/UdpClientInfo.h"
#include "sockets/udp/client/UdpServerInfo.h"

namespace protocols
{
class SenderBase;
class NetworkService;
class ProtocolService;
class ReceiverBase;

class NetworkManager
{
public:
    NetworkManager(std::string xmlConfigPath);
    ~NetworkManager();

    bool startUdpServer();
    bool stopUdpServer();

    bool startUdpClient();
    bool stopUdpClient();

    bool startTcpServer();
    bool stopTcpServer();

    bool startTcpClient();
    bool stopTcpClient();

    bool preStart(ReceiverBase * receiver);
    bool stopAll();

    SenderBase * createSenderForTcpClient(int configGroupId, std::string remoteIp, unsigned short remotePort);
    SenderBase * createSenderForUdpClient(int configGroupId, std::string remoteIp, unsigned short remotePort);
    Configuration * getConfiguration() { return mConfiguration; }
    NetworkService * getNetworkService() { return mNetService; }

private:
    NetworkManager(const NetworkManager & other) = delete;
    NetworkManager & operator=(const NetworkManager & other) = delete;

    int getConfigGroupId(unsigned short localPort, SocketType & socketType);
    sockets::AddressInfo * tcpClientConnect(sockets::AddressInfo * tcpClientInfo, bool isReconnect = false);
    // std::string & remoteIp, unsigned short remotePort, unsigned short localPort, std::string & localIp, bool isReconnect = false);
    bool udpClientBind(std::string & remoteIp, unsigned short remotePort, unsigned short localPort, std::string & localIp);

private:
    friend class                NetworkService;
    Configuration               * mConfiguration;
    NetworkService              * mNetService;
    ProtocolService             * mProtocolService;
    bool                        mUdpServerState;
    bool                        mUdpClientState;
    bool                        mTcpServerState;
    bool                        mTcpClientState;
};  // class NetworkManager
};  // namespace protocols

#endif  // PROTOCOLS_NETWORK_MANAGER_H
