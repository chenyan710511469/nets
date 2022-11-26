/*
 * =====================================================================================
 *
 *       Filename:  Sender.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/05/2019 07:48:38 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_SENDER_H
#define PROTOCOLS_SENDER_H

#include <time.h>

#include <deque>
#include <vector>
#include <list>
#include <mutex>

#include "protocol/SenderBase.h"
#include "protocol/NetworkService.h"
#include "protocol/StorageData.h"

namespace protocols
{
class SenderBase;
class NetworkService;
class StorageData;

class Sender : public SenderBase
{
public:
    Sender(const Sender & other) = delete;
    Sender & operator=(const Sender & other) = delete;
    SocketType getSocketType();

private:
    ~Sender();
    explicit Sender(SocketType socketType, sockets::AddressInfo * addressInfo, NetworkService * netService);

    bool send(tinyxml2::XMLDocument * xmlDocument, void * upperPointer);
    bool send(const char * buffer, const size_t bufferSize, void * upperPointer);
    bool send(Json::Value * rootValue, void * upperPointer);
    int getConfigGroupId();
    sockets::AddressInfo * getAddressInfo();

    bool requestGroupId();
    bool resendEndSequenceData();
    bool packet(const std::string & groupSId);
    bool requestSend(const std::string & groupSId);
    bool requestSend(const std::string & groupSId, const int sequenceId);
    bool requestSend(sockets::AddressInfo * addressInfo);
    bool responseReconnectedAck();
    bool assembleComplete(const std::string & groupSId, const int maxSequenceId, bool isSuccess);
    bool ackCompleted(const std::string & groupSId, const int maxSequenceId);

    void updateTime();
    bool isTimeout(unsigned long long timeInterval);  // unit is millisecond.
    bool closeFromPeer();
    bool closeOverFromPeer();
    void updateRequestGroupIdTime();
    bool isTimeoutRequestGroupId(unsigned long long timeInterval);  // unit is millisecond.
    long long getCurrentBootTime();  // CLOCK_BOOTTIME

private:
    friend class                    NetworkService;
    SocketType                                          mSocketType;
    int                                                 mConfigGroupId;
    NetworkService*                                     mNetService;
    short                                               mProtocolMTU;
    sockets::AddressInfo*                               mAddressInfo;
    int                                                 mSendGroupId;
    std::deque<StorageData *>                           mDequeStorageData;
    std::mutex                                          mMutexDeque;  // 2

    /*********key=groupId**********/
    std::map<std::string, std::vector<StorageData *> *> mMapSending;
    std::mutex                                          mMutexSending;  // 3
    std::map<std::string, std::vector<StorageData *> *> mMapInterimSend;
    std::vector<StorageData *>                          mVecRelease;
    std::list<std::vector<StorageData *> *>             mListRelease;
    std::mutex                                          mMutexRelease;  // 1
    std::map<std::string, StorageData *>                mEndSequenceData;
    std::mutex                                          mMutexEndSequenceData;  // 4
    struct timespec                                     mTime;
    int                                                 mSendFailedTimes;
    bool                                                mIsConnected;
    bool                                                mIsRelease;
    std::string                                         mLocalNetIp;
    unsigned short                                      mLocalNetPort;
    struct timespec                                     mRequestGroupIdTime;
    long long                                           mLastSendIntervalTime;
    long long                                           mLastRequestGroupIntervalTime;
};  // class Sender
};  // namespace protocols
#endif  // end PROTOCOLS_SENDER_H

