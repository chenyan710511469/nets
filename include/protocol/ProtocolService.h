/*
 * =====================================================================================
 *
 *       Filename:  ProtocolService.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  05/17/2019 07:29:34 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_PROTOCOL_SERVICE_H
#define PROTOCOLS_PROTOCOL_SERVICE_H

#include <map>
#include <mutex>

#include "sockets/udp/server/UdpMessage.h"
#include "sockets/udp/server/UdpClientInfo.h"
#include "sockets/udp/server/UdpIoService.h"

#include "sockets/udp/client/UdpMessage.h"
#include "sockets/udp/client/UdpServerInfo.h"
#include "sockets/udp/client/UdpIoService.h"

#include "sockets/tcp/server/TcpMessage.h"
#include "sockets/tcp/server/TcpClientInfo.h"
#include "sockets/tcp/server/TcpIoService.h"

#include "sockets/tcp/client/TcpMessage.h"
#include "sockets/tcp/client/TcpServerInfo.h"
#include "sockets/tcp/client/TcpIoService.h"

#include "sockets/AddressInfo.h"

#include "protocol/HandleClientData.h"
#include "protocol/HandleServerData.h"
#include "protocol/ResponseOtherPartyInfo.h"
#include "protocol/NetworkService.h"
#include "protocol/StorageData.h"
#include "protocol/MessageBase.h"

namespace protocols
{
class HandleServerData;
class ResponseOtherPartyInfo;
class NetworkService;
class HandleClientData;
class ProtocolService
{
public:
    ProtocolService(NetworkService * netService);
    ~ProtocolService();

    bool startUdpServer(unsigned short localPort, std::string localIp = "");
    bool closeUdpServer(unsigned short localPort);
    bool stopUdpServer();

    bool startUdpClient(std::string & remoteIp, unsigned short remotePort, unsigned short localPort, std::string localIp = "");
    bool stopUdpClient();

    bool startTcpServer(sockets::AddressInfo * addressInfo);  // unsigned short localPort, std::string localIp = "");
    bool stopTcpServer();

    sockets::AddressInfo * startTcpClient(sockets::AddressInfo * tcpClientInfo, bool isReconnect = false);
    // std::string & remoteIp, unsigned short remotePort,
    // unsigned short localPort, std::string localIp = "", bool isReconnect = false);
    bool stopTcpClient();

    ProtocolService(const ProtocolService & other) = delete;
    ProtocolService & operator=(const ProtocolService & other) = delete;

    bool receive(StorageData * receivedData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

private:
    /* udp server */
    static void udpServerMessageCallback(udpserver::UdpMessage & message, sockets::AddressInfo & clientInfo, void * owner);
    void udpServerMessage(udpserver::UdpMessage & message, sockets::AddressInfo * clientInfo);
    static bool releaseUdpServer(sockets::AddressInfo & udpClient, void * owner);
    bool releaseUdpServer(sockets::AddressInfo * udpClient);

    /* udp client */
    static void udpClientMessageCallback(udpclient::UdpMessage & message, sockets::AddressInfo & serverInfo, void * owner);
    void udpClientMessage(udpclient::UdpMessage & message, sockets::AddressInfo * serverInfo);
    static bool releaseUdpClient(sockets::AddressInfo & udpServer, void * owner);
    bool releaseUdpClient(sockets::AddressInfo * udpServer);

    /* tcp server */
    static void tcpServerMessageCallback(tcpserver::TcpMessage & message, sockets::AddressInfo & clientInfo, void * owner);
    void tcpServerMessage(tcpserver::TcpMessage & message, sockets::AddressInfo * clientInfo);
    bool closeClientSocketFd(sockets::AddressInfo * addressInfo, SocketType & socketType);  // in tcp server.
    static bool releaseTcpServer(sockets::AddressInfo & tcpClient, void * owner);
    bool releaseTcpServer(sockets::AddressInfo * tcpClient);

    /* tcp client */
    static bool tcpClientMessageCallback(tcpclient::TcpMessage & message, sockets::AddressInfo & serverInfo, void * owner);
    bool tcpClientMessage(tcpclient::TcpMessage & message, sockets::AddressInfo * serverInfo);
    static bool tcpClientConnectCallback(sockets::AddressInfo & tcpServer, bool isConnected, void * owner);
    bool tcpClientConnect(sockets::AddressInfo & tcpServer, bool isConnected);
    static bool releaseTcpClientCallback(sockets::AddressInfo & tcpServer, void * owner);
    bool releaseTcpClient(sockets::AddressInfo * tcpServer);

    void notifyRequestGroupIdFailed(sockets::AddressInfo * addressInfo, const SocketType & socketType);
    void notifyRequestGroupIdSuccess(const std::string & groupSId, sockets::AddressInfo * addressInfo, const SocketType & socketType);
    bool requestSequenceData(std::string & groupSId, int sequenceId, sockets::AddressInfo * addressInfo, const SocketType & socketType);
    bool ackSequenceData(std::string & groupSId, int sequenceId, sockets::AddressInfo * addressInfo, const SocketType & socketType);
    bool assembleComplete(std::string & groupSId, int maxSequenceId, sockets::AddressInfo * addressInfo, const SocketType & socketType);
    bool ackCompleted(std::string & groupSId, int maxSequenceId, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool reconnectedPeer(MessageBase * message);

    /* server ---> client */
    bool reconnectedPeerAck(MessageBase * message);

    /* server <---> client */
    bool closePeer(MessageBase * message);

    /* server <---> client */
    bool closePeerOver(MessageBase * message);

    /* server <---> client */
    bool closePeerOverAck(MessageBase * message);

    int getClientGroupId(const sockets::AddressInfo * addressInfo, const SocketType & socketType);
    sockets::AddressInfo * getUdpClientAddressInfo(unsigned short localPort);
    sockets::AddressInfo * getTcpClientAddressInfo(unsigned short localPort);
    void releaseMapGroup();

private:
    friend class                                        NetworkService;
    friend class                                        HandleServerData;
    friend class                                        HandleClientData;
    NetworkService                                      * mNetService;
    udpserver::UdpIoService                             * mUdpService;
    udpclient::UdpIoService                             * mUdpClientService;
    tcpserver::TcpIoService                             * mTcpService;
    tcpclient::TcpIoService                             * mTcpClientService;

    /*** key = localPort ***/
    std::map<unsigned short, sockets::AddressInfo *>    mMapUdpServerInfo;
    std::mutex                                          mMutexUdpServer;
    std::map<unsigned short, sockets::AddressInfo *>    mMapUdpClientInfo;
    std::mutex                                          mMutexUdpClient;
    std::map<unsigned short, sockets::AddressInfo *>    mMapTcpServerInfo;
    std::mutex                                          mMutexTcpServer;
    std::map<unsigned short, sockets::AddressInfo *>    mMapTcpClientInfo;
    std::mutex                                          mMutexTcpClient;

    ResponseOtherPartyInfo                              * mResponse;
    HandleClientData                                    * mHandleClientData;
    HandleServerData                                    * mHandleServerData;
};
};  // protocols
#endif  // PROTOCOLS_PROTOCOL_SERVICE_H

