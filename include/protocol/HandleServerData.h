/*
 * =====================================================================================
 *
 *       Filename:  HandleServerData.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/16/2019 06:58:42 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_HANDLE_SERVER_DATA_H
#define PROTOCOLS_HANDLE_SERVER_DATA_H

#include <mutex>

#include "sockets/AddressInfo.h"

#include "protocol/ProtocolService.h"
#include "protocol/ProtocolGroup.h"
#include "protocol/protocol_def.h"
#include "protocol/code_def.h"
#include "protocol/ResponseOtherPartyInfo.h"
#include "protocol/HandleClientData.h"

#include "sockets/udp/server/UdpClientInfo.h"
#include "sockets/udp/client/UdpServerInfo.h"
#include "sockets/tcp/server/TcpClientInfo.h"
#include "sockets/tcp/client/TcpServerInfo.h"
#include "protocol/MessageBase.h"
#include "protocol/Message.h"


namespace protocols
{
class ProtocolService;
class ResponseOtherPartyInfo;
class HandleClientData;
class HandleServerData
{
public:
    HandleServerData(ProtocolService *pService, ResponseOtherPartyInfo *response, HandleClientData * handleClientData);
    ~HandleServerData();

    HandleServerData(const HandleServerData & other) = delete;
    HandleServerData & operator=(const HandleServerData & other) = delete;

    bool parseProtocolData(char *data, const size_t dataSize, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    bool packagedIntoProtocolData(char *data, const size_t dataSize);
    void releaseMapGroupServer();

private:
    bool checkCode(const int code);
    char* checkNextPackageData(char *p);

    bool handleProtocolData(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */  // client发给server
    bool sequenceData(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool requestGroup(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool replyGroup(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool ackGroup(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool groupNotExist(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool redundantGroup(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool requestSequenceData(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool endSequence(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool ackSequenceData(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool assembleComplete(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool ackAssembleComplete(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool ackCompleted(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool reconnectedPeer(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool closePeer(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool closePeerOver(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool closePeerOverAck(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

private:

    // key = remoteIp_remotePort_groupId
    std::map<std::string, ProtocolGroup *>  mMapGroupServer;
    std::vector<ProtocolGroup *>            mVecReleaseGroup;
    std::mutex                              mMutexGroupServer;

    ProtocolService                         * mProtocolService;
    ResponseOtherPartyInfo                  * mResponse;
    HandleClientData                        * mHandleClientData;
};  // HandleServerData
};  // protocols

#endif  // PROTOCOLS_HANDLE_SERVER_DATA_H
