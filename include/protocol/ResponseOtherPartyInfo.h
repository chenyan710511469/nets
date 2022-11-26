/*
 * =====================================================================================
 *
 *       Filename:  ResponseOtherPartyInfo.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/23/2019 07:34:22 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_RESPONSE_OTHER_PARTY_INFO_H
#define PROTOCOLS_RESPONSE_OTHER_PARTY_INFO_H

#include "protocol/HandleServerData.h"
#include "protocol/HandleClientData.h"

#include "sockets/AddressInfo.h"

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

#include "protocol/protocol_def.h"
#include "protocol/ProtocolData.h"
#include "protocol/NetworkService.h"


namespace protocols
{
class HandleServerData;
class NetworkService;

class ResponseOtherPartyInfo
{
public:
    ResponseOtherPartyInfo();
    ~ResponseOtherPartyInfo();

    void setUdpService(udpserver::UdpIoService * udpService);
    void setUdpClientService(udpclient::UdpIoService * udpClientService);
    void setTcpService(tcpserver::TcpIoService * tcpService);
    void setTcpClientService(tcpclient::TcpIoService * tcpClientService);

private:
    bool sendProtocolData(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    bool sendToOtherParty(char * data, size_t dataSize, const sockets::AddressInfo * addressInfo,
            const SocketType & socketType);

    sockets::AddressInfo * getAddressInfo(const sockets::AddressInfo * addressInfo, const SocketType & socketType);
    bool addLength(char ** buf, size_t * bufSize);
    void printStacktrace();

private:
    friend class HandleServerData;
    friend class HandleClientData;
    friend class NetworkService;

    udpserver::UdpIoService                 * mUdpService;
    udpclient::UdpIoService                 * mUdpClientService;
    tcpserver::TcpIoService                 * mTcpService;
    tcpclient::TcpIoService                 * mTcpClientService;

};  // ResponseOtherPartyInfo
};  // protocols

#endif  // PROTOCOLS_RESPONSE_OTHER_PARTY_INFO_H
