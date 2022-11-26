/*
 * =====================================================================================
 *
 *       Filename:  NetworkService.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/05/2019 07:28:26 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_NETWORK_SERVICE_H
#define PROTOCOLS_NETWORK_SERVICE_H

#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "sockets/AddressInfo.h"
#include "protocol/NetworkManager.h"
#include "protocol/ReceiverBase.h"
#include "protocol/StorageData.h"
#include "protocol/ResponseOtherPartyInfo.h"
#include "protocol/Sender.h"
#include "protocol/protocol_def.h"
#include "protocol/ProtocolService.h"
#include "protocol/MessageBase.h"
#include "protocol/Message.h"

namespace protocols
{
class NetworkManager;
class ResponseOtherPartyInfo;
class Sender;
class ProtocolService;
class MessageBase;
class Message;
class ReceiverBase;

class NetworkService
{
public:
    NetworkService(NetworkManager * netManager);
    ~NetworkService();
    ReceiverBase * getReceiver();

private:
    bool preStart(ReceiverBase * receiver);
    void run();
    void handleMessage(MessageBase * message);
    void check();
    bool stop();

    sockets::AddressInfo * getAddressInfo(const sockets::AddressInfo * addressInfo, const SocketType & socketType);
    int getConfigGroupId(unsigned short localPort, SocketType & socketType);
    void setResponse(ResponseOtherPartyInfo  * response);
    bool receive(StorageData * receivedData, const sockets::AddressInfo * addressInfo, const SocketType & socketType);
    SenderBase * createSenderForTcpClient(int configGroupId, std::string & remoteIp, unsigned short remotePort);
    SenderBase * createSenderForUdpClient(int configGroupId, std::string & remoteIp, unsigned short remotePort);
    Sender * getSenderByAddressInfo(std::string & key);
    bool sendToOtherParty(char * data, size_t dataSize, const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    void notifyRequestGroupIdFailed(sockets::AddressInfo * addressInfo, const SocketType & socketType);
    void notifyRequestGroupIdSuccess(const std::string & groupSId, sockets::AddressInfo * addressInfo, const SocketType & socketType);
    bool requestSequenceData(std::string & groupSId, int sequenceId, sockets::AddressInfo * addressInfo, const SocketType & socketType);
    bool ackSequenceData(std::string & groupSId, int sequenceId, sockets::AddressInfo * addressInfo, const SocketType & socketType);
    bool assembleComplete(std::string & groupSId, int maxSequenceId, sockets::AddressInfo * addressInfo, const SocketType & socketType);
    bool ackCompleted(std::string & groupSId, int maxSequenceId, sockets::AddressInfo * addressInfo, const SocketType & socketType);

    int getClientGroupId(const sockets::AddressInfo * addressInfo, const SocketType & socketType);

    bool serverReplyGroup(std::vector<void *> * vecObj);
    bool clientReplyGroup(std::vector<void *> * vecObj);
    bool serverSequenceData(std::vector<void *> * vecObj);
    bool clientSequenceData(std::vector<void *> * vecObj);
    bool serverAckSequence(std::vector<void *> * vecObj);
    bool clientAckSequence(std::vector<void *> * vecObj);
    bool serverAssembleComplete(std::vector<void *> * vecObj);
    bool clientAssembleComplete(std::vector<void *> * vecObj);
    bool serverAckCompleted(std::vector<void *> * vecObj);
    bool clientAckCompleted(std::vector<void *> * vecObj);
    bool requestGroupId(Sender * sender);
    bool handleRequestSend(std::vector<void *> * vecObj);

    bool receiveTcpClientConnect(sockets::AddressInfo * addressInfo, bool isConnected);
    bool releaseTcpServer(sockets::AddressInfo * addressInfo);
    bool releaseTcpClient(sockets::AddressInfo * addressInfo);
    bool releaseUdpServer(sockets::AddressInfo * addressInfo);
    bool releaseUdpClient(sockets::AddressInfo * addressInfo);
    sockets::AddressInfo * tcpClientReconnect(sockets::AddressInfo * addressInfo);
    bool closeClientSocketFd(sockets::AddressInfo * addressInfo, SocketType & socketType);  // in tcp server.

    /* client ---> server */
    bool reconnectedPeer(MessageBase * message);
    bool serverReconnectedPeer(std::vector<void *> * vecObj);

    /* server ---> client */
    bool reconnectedPeerAck(MessageBase * message);
    bool clientReconnectedPeerAck(std::vector<void *> * vecObj);

    /* server <---> client */
    bool closePeer(MessageBase * message);
    bool serverClosePeer(std::vector<void *> * vecObj);
    bool clientClosePeer(std::vector<void *> * vecObj);

    /* server <---> client */
    bool closePeerOver(MessageBase * message);
    bool serverClosePeerOver(std::vector<void *> * vecObj);
    bool clientClosePeerOver(std::vector<void *> * vecObj);

    /* server <---> client */
    bool closePeerOverAck(MessageBase * message);
    bool serverClosePeerOverAck(std::vector<void *> * vecObj);
    bool clientClosePeerOverAck(std::vector<void *> * vecObj);

    void replyUpper(Sender * sender, bool result, void * upperPointer);

private:
    friend class            NetworkManager;
    friend class            Sender;
    friend class            ProtocolService;

    NetworkManager                                  * mNetManager;
    ReceiverBase                                    * mReceiver;
    ResponseOtherPartyInfo                          * mResponse;

    std::vector<std::thread *>                      mVecThread;
    bool                                            mIsRunning;
    std::condition_variable                         mCondition;

    /* ***** key = remoteIp:remotePort ***** */
    std::map<std::string, Sender *>                 mMapSender;
    std::mutex                                      mMutexMap;
    std::deque<MessageBase *>                       mDequeMessage;
    std::mutex                                      mMutexDeque;
    std::vector<Sender *>                           mReleaseSender;
    std::mutex                                      mMutexRelease;

    int                                             mCounter;
};  //  class NetworkService
};  //  namespace protocols

#endif  // PROTOCOLS_NETWORK_SERVICE_H
