/*
 * =====================================================================================
 *
 *       Filename:  HandleClientData.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  09/06/2019 08:34:19 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_HANDLE_CLIENT_DATA_H
#define PROTOCOLS_HANDLE_CLIENT_DATA_H

#include "protocol/ProtocolService.h"
#include "protocol/ResponseOtherPartyInfo.h"
#include "protocol/ProtocolData.h"
#include "protocol/HandleServerData.h"
#include "protocol/code_def.h"
#include "protocol/protocol_def.h"
#include "protocol/ProtocolGroup.h"
#include "protocol/StorageData.h"
#include "protocol/MessageBase.h"

#include "sockets/AddressInfo.h"

namespace protocols
{
class ProtocolService;
class ResponseOtherPartyInfo;
class HandleServerData;
class HandleClientData
{
public:
    HandleClientData(ProtocolService * pService, ResponseOtherPartyInfo * response);
    ~HandleClientData();

    void releaseMapGroupClient();

private:
    bool handleProtocolData(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */  // client发给server
    bool sequenceData(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool requestGroup(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool replyGroup(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool ackGroup(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool groupNotExist(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool redundantGroup(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool requestSequenceData(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool endSequence(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool ackSequenceData(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool assembleComplete(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool ackAssembleComplete(ProtocolData *pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* client ---> server */
    bool ackCompleted(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool reconnectedPeerAck(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool closePeer(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool closePeerOver(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    /* server ---> client */
    bool closePeerOverAck(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType);

private:
    friend class HandleServerData;

    // key = remoteIp_remotePort_groupId
    std::map<std::string, ProtocolGroup *>  mMapGroupClient;
    std::vector<ProtocolGroup *>            mVecReleaseGroup;
    std::mutex                              mMutexGroupClient;

    ProtocolService                         * mProtocolService;
    ResponseOtherPartyInfo                  * mResponse;

};  // HandleClientData
};  // protocols
#endif  // PROTOCOLS_HANDLE_CLIENT_DATA_H
