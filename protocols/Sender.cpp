/*
 * =====================================================================================
 *
 *       Filename:  Sender.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/05/2019 08:03:15 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */

#include "protocol/Sender.h"
#include <stdlib.h>
#include <string.h>
#include "protocol/code_def.h"
#include "protocol/protocol_def.h"

namespace protocols
{
Sender::Sender(SocketType socketType, sockets::AddressInfo * addressInfo, NetworkService * netService)
 : mSocketType(socketType)
 , mConfigGroupId(netService->getConfigGroupId(addressInfo->getLocalPort(), socketType))
 , mNetService(netService)
 , mProtocolMTU(ONE_THOUSAND_AND_TWO_HUNDRED)
 , mAddressInfo(addressInfo)
 , mSendFailedTimes(ZERO)
 , mIsConnected(true)
 , mIsRelease(false)
 , mLocalNetPort(ZERO)
 , mLastSendIntervalTime(FIVE_HUNDRED)
 , mLastRequestGroupIntervalTime(SIX)
{
    memset(&mTime, ZERO, sizeof(struct timespec));
    updateTime();
    updateRequestGroupIdTime();
}

Sender::~Sender()
{
    mIsRelease = true;
    mMutexDeque.lock();
    std::deque<StorageData *>::iterator itDeque = mDequeStorageData.begin();
    while (itDeque != mDequeStorageData.end()) {
        StorageData * data = *itDeque;
        itDeque = mDequeStorageData.erase(itDeque);
        delete data;
    }
    mMutexDeque.unlock();

    mMutexRelease.lock();
    mMutexSending.lock();
    mMutexEndSequenceData.lock();
    std::map<std::string, std::vector<StorageData *> *>::iterator itSending = mMapSending.begin();
    while (itSending != mMapSending.end()) {
        std::vector<StorageData *> * vec = itSending->second;
        itSending = mMapSending.erase(itSending);
        std::vector<StorageData *>::iterator itVec = vec->begin();
        while (itVec != vec->end()) {
            StorageData * data = *itVec;
            itVec = vec->erase(itVec);
            delete data;
        }
    }
    mEndSequenceData.clear();
    mMutexEndSequenceData.unlock();
    mMutexSending.unlock();
    std::list<std::vector<StorageData *> *>::iterator itListRelease = mListRelease.begin();
    for (; itListRelease != mListRelease.end(); ) {
        std::vector<StorageData *> * vecR = *itListRelease;
        itListRelease = mListRelease.erase(itListRelease);
        delete vecR;
    }
    std::vector<StorageData *>::iterator itVecRelease = mVecRelease.begin();
    for (; itVecRelease != mVecRelease.end(); ) {
        StorageData * data = *itVecRelease;
        itVecRelease = mVecRelease.erase(itVecRelease);
        delete data;
    }
    mMutexRelease.unlock();
    mNetService = NULL;
    if (NULL != mAddressInfo) {
        delete mAddressInfo;
        mAddressInfo = NULL;
    }
}

SocketType Sender::getSocketType()
{
    return mSocketType;
}

int Sender::getConfigGroupId()
{
    return mConfigGroupId;
}

sockets::AddressInfo * Sender::getAddressInfo()
{
    return mAddressInfo;
}

bool Sender::send(tinyxml2::XMLDocument * xmlDocument, void * upperPointer)
{
    if (mIsRelease) {
        printf("%s:%d release sender.\n", __FUNCTION__, __LINE__);
        return false;
    }
    if (!mIsConnected) {
        printf("mIsConnected is false.\n");
        return false;
    }
    if (SIX < mDequeStorageData.size()) {
        ++mSendFailedTimes;
        mNetService->requestGroupId(this);
        usleep(250000);
        printf("%s:%d mSendFailedTimes=%d.\n", __FUNCTION__, __LINE__, mSendFailedTimes);
        return false;
    }
    if (NULL == xmlDocument) {
        printf("xmlDocument is NULL.\n");
        return false;
    }
    std::string xmlData(xmlDocument->Value());
    if (xmlData.empty()) {
        return false;
    }
    size_t bufLength = xmlData.size();
    if (bufLength < THREE) {
        printf("xml content is error.\n");
        return false;
    }
    updateTime();
    char *buf = NULL;
    while (NULL == (buf = (char *)malloc(bufLength + ONE)));
    memset(buf, ZERO, bufLength + ONE);
    memcpy(buf, xmlData.c_str(), bufLength);
    StorageData * data = new StorageData(buf, bufLength, TYPE_XML);
    if (NULL == data) {
        free(buf);
        buf = NULL;
        return false;
    }
    data->setUpperPointer(upperPointer);
    mMutexDeque.lock();
    mDequeStorageData.push_back(data);
    mMutexDeque.unlock();
    mNetService->requestGroupId(this);
    return true;
}

bool Sender::send(const char * buffer, const size_t bufferSize, void * upperPointer)
{
    if (mIsRelease) {
        printf("%s:%d release sender.\n", __FUNCTION__, __LINE__);
        return false;
    }
    if (!mIsConnected) {
        printf("mIsConnected is false.\n");
        return false;
    }
    if (SIX < mDequeStorageData.size()) {
        ++mSendFailedTimes;
        mNetService->requestGroupId(this);
        usleep(250000);
        printf("%s:%d mSendFailedTimes=%d.\n", __FUNCTION__, __LINE__, mSendFailedTimes);
        return false;
    }
    if (NULL == buffer) {
        printf("buffer is NULL.\n");
        return false;
    }
    if (ZERO >= bufferSize) {
        printf("bufferSize must be more than 0.\n");
        return false;
    }
    updateTime();
    char * buf = NULL;
    while (NULL == (buf = (char *)malloc(bufferSize + ONE)));
    memset(buf, ZERO, bufferSize + ONE);
    memcpy(buf, buffer, bufferSize);
    StorageData * data = new StorageData(buf, bufferSize, TYPE_TEXT);
    if (NULL == data) {
        free(buf);
        buf = NULL;
        return false;
    }
    data->setUpperPointer(upperPointer);
    mMutexDeque.lock();
    printf("%s %s:%d:  Sender:%p\n", __FILE__, __FUNCTION__, __LINE__, this);
    mDequeStorageData.push_back(data);
    mMutexDeque.unlock();
    mNetService->requestGroupId(this);
    return true;
}

bool Sender::send(Json::Value * rootValue, void * upperPointer)
{
    if (mIsRelease) {
        printf("%s:%d release sender.\n", __FUNCTION__, __LINE__);
        return false;
    }
    if (!mIsConnected) {
        printf("mIsConnected is false.\n");
        return false;
    }
    if (SIX < mDequeStorageData.size()) {
        ++mSendFailedTimes;
        mNetService->requestGroupId(this);
        usleep(250000);
        printf("%s:%d mSendFailedTimes=%d.\n", __FUNCTION__, __LINE__, mSendFailedTimes);
        return false;
    }
    if (NULL == rootValue) {
        printf("rootValue is NULL.\n");
        return false;
    }
    std::string jsonData(rootValue->asCString());
    size_t bufLength = jsonData.size();
    if (bufLength <= FIVE) {
        printf("json content is error.\n");
        return false;
    }
    updateTime();
    char * buf = NULL;
    while (NULL == (buf = (char *)malloc(bufLength + ZERO)));
    memset(buf, ZERO, bufLength + ZERO);
    memcpy(buf, jsonData.c_str(), bufLength);
    StorageData * data = new StorageData(buf, bufLength, TYPE_JSON);
    if (NULL == data) {
        free(buf);
        buf = NULL;
        return false;
    }
    data->setUpperPointer(upperPointer);
    mMutexDeque.lock();
    mDequeStorageData.push_back(data);
    mMutexDeque.unlock();
    mNetService->requestGroupId(this);
    return true;
}

bool Sender::packet(const std::string & groupSId)
{
    if (mIsRelease) {
        return false;
    }
    mMutexSending.lock();
    std::map<std::string, std::vector<StorageData *> *>::iterator it = mMapSending.find(groupSId);
    if (it != mMapSending.end()) {
        mMutexSending.unlock();
        // TODO: check StorageData of mMapSending to be timeout ?
        return false;
    }
    it = mMapInterimSend.find(groupSId);
    if (it != mMapInterimSend.end()) {
        mMutexSending.unlock();
        // TODO: check StorageData of mMapSending to be timeout ?
        return false;
    }
    std::vector<StorageData *> * vec = new std::vector<StorageData *>();
    std::pair<std::map<std::string, std::vector<StorageData *> *>::iterator, bool> retMap
    = mMapInterimSend.insert(std::pair<std::string, std::vector<StorageData *> *>(groupSId, vec));
    mMutexSending.unlock();
    if (!retMap.second) {
        delete vec;
        return false;
    }
    char socketProtocol[FOUR] = {ZERO};
    int code = MINUS_ONE;
    int endSequenceCode = MINUS_ONE;
    switch (mSocketType) {
        case UDP_SERVER: {
            code = CLIENT_REPLACE_WITH_SEQUENCE_DATA;
            endSequenceCode = CLIENT_END_SEQUENCE;
            strncpy(socketProtocol, "udp", THREE);
            break;
        }
        case UDP_CLIENT: {
            code = SERVER_REPLACE_WITH_SEQUENCE_DATA;
            endSequenceCode = SERVER_END_SEQUENCE;
            strncpy(socketProtocol, "udp", THREE);
            break;
        }
        case TCP_SERVER: {
            code = CLIENT_REPLACE_WITH_SEQUENCE_DATA;
            endSequenceCode = CLIENT_END_SEQUENCE;
            strncpy(socketProtocol, "tcp", THREE);
            break;
        }
        case TCP_CLIENT: {
            code = SERVER_REPLACE_WITH_SEQUENCE_DATA;
            endSequenceCode = SERVER_END_SEQUENCE;
            strncpy(socketProtocol, "tcp", THREE);
            break;
        }
        default: {
            mMutexSending.lock();
            mMapInterimSend.erase(groupSId);
            mMutexSending.unlock();
            delete vec;printf("%s:%d\n", __FUNCTION__, __LINE__);
            return false;
        }
    }
    mMutexDeque.lock();
    if (mDequeStorageData.empty()) {
        mMutexDeque.unlock();  // 先释放锁.
        mMutexSending.lock();  // 注意和 requestGroupId() 不要造成死锁.
        mMapInterimSend.erase(groupSId);
        mMutexSending.unlock();
        delete vec;
        return false;
    }
    StorageData * storageData = mDequeStorageData.front();
    mDequeStorageData.pop_front();
    mMutexDeque.unlock();
    const char * buffer = storageData->getData();
    const size_t bufferSize = storageData->getDataLength();
    const int dataType = storageData->getDataType();
    const int suffixLength = ONE_HUNDRED_AND_TWENTY_EIGHT * TWO;
    char protocolSuffix[ONE_HUNDRED_AND_TWENTY_EIGHT] = {ZERO};
    snprintf(protocolSuffix, sizeof(protocolSuffix), "%s,%d,%ld", socketProtocol, dataType, bufferSize);
    void * upperPointer = storageData->getUpperPointer();
    /*
    if (bufferSize <= mProtocolMTU) {
        char suffix[suffixLength] = {ZERO};
        sprintf(suffix, "%s,1,%d,%s,%ld{", groupSId.c_str(), code, protocolSuffix, bufferSize);
        size_t suffixSize = strlen(suffix);
        size_t totalSize = suffixSize + bufferSize + ONE;  // length of include "}".
        char * buf = NULL;
        while (NULL == (buf = (char *)malloc(totalSize + ONE)));
        memset(buf, ZERO, totalSize + ONE);
        memcpy(buf, suffix, suffixSize);
        memcpy(buf + suffixSize, buffer, bufferSize);
        memcpy(buf + suffixSize + bufferSize, "}", ONE);
        vec->push_back(buf);
        memset(suffix, ZERO, suffixLength);
        sprintf(suffix, "%s,2,%d,%s,0{}", groupSId.c_str(), endSequenceCode, protocolSuffix);  // maxSequenceId = 2;
        suffixSize = strlen(suffix);
        totalSize = suffixSize;
        buf = NULL;
        while (NULL == (buf = (char *)malloc(totalSize + ONE)));
        memset(buf, ZERO, totalSize + ONE);
        memcpy(buf, suffix, suffixSize);
        vec->push_back(buf);
    }
    else {
    */
    bool exactDivision = true;
    int maxSequenceId = bufferSize / mProtocolMTU;
    ++maxSequenceId;
    int remainder = bufferSize % mProtocolMTU;
    if (ZERO != remainder) {
        ++maxSequenceId;
        exactDivision = false;
    }
    for (int i = 0; i < maxSequenceId - ONE; ++i) {
        int leng = 0;
        char suffix[suffixLength] = {ZERO};
        if ((i == (maxSequenceId - ONE) - ONE) && !exactDivision) {
            leng = remainder;
        }
        else {
            leng = mProtocolMTU;
        }
        sprintf(suffix, "%s,%d,%d,%s,%d{", groupSId.c_str(), (i + ONE), code, protocolSuffix, leng);
        size_t suffixSize = strlen(suffix);
        size_t totalSize = suffixSize + leng + ONE;  // length of include "}".
        char * buf = NULL;
        while (NULL == (buf = (char *)malloc(totalSize + ONE)));
        memset(buf, ZERO, totalSize + ONE);
        memcpy(buf, suffix, suffixSize);
        memcpy(buf + suffixSize, buffer + (i * mProtocolMTU), leng);
        memcpy(buf + suffixSize + leng, "}", ONE);
        StorageData * data = new StorageData(buf, totalSize, dataType);
        vec->push_back(data);
    }
    char maxSuffix[suffixLength] = {ZERO};
    memset(maxSuffix, ZERO, suffixLength);
    snprintf(maxSuffix, sizeof(maxSuffix), "%s,%d,%d,%s,0{}",
            groupSId.c_str(),
            maxSequenceId,
            endSequenceCode,
            protocolSuffix);
    size_t maxSuffixSize = strlen(maxSuffix);
    char * maxBuf = NULL;
    while (NULL == (maxBuf = (char *)malloc(maxSuffixSize + ONE)));
    memset(maxBuf, ZERO, maxSuffixSize + ONE);
    memcpy(maxBuf, maxSuffix, maxSuffixSize);
    StorageData * maxData = new StorageData(maxBuf, maxSuffixSize, dataType);
    maxData->setGroupSId(groupSId);
    maxData->setMaxSequenceId(maxSequenceId);
    maxData->setUpperPointer(upperPointer);
    vec->push_back(maxData);
    // }

    mMutexSending.lock();
    retMap = mMapSending.insert(std::pair<std::string, std::vector<StorageData *> *>(groupSId, vec));
    mMapInterimSend.erase(groupSId);  // the line code has crashed.
    mMutexSending.unlock();
    bool ret = true;
    if (!retMap.second) {
        std::vector<StorageData *>::iterator it = vec->begin();
        for (; it != vec->end(); ) {
            StorageData * data = *it;
            it = vec->erase(it);
            delete data;
        }
        delete vec;
        ret = false;
    } else {
        std::size_t posIp = groupSId.find('_');
        if (posIp != std::string::npos) {
            mLocalNetIp = groupSId.substr(ZERO, posIp);
            std::size_t posPort = groupSId.find(posIp + ONE, '_');
            if (posPort != std::string::npos) {
                std::string localNetPort = groupSId.substr(posIp + ONE, posPort - (posIp + ONE));
                mLocalNetPort = atoi(localNetPort.c_str());
            }
        }
    }
    delete storageData;
    storageData = NULL;
    return ret;
}

bool Sender::requestSend(const std::string & groupSId)
{
    if (mIsRelease) {
        printf("%s:%d release sender.\n", __FUNCTION__, __LINE__);
        return false;
    }
    mMutexSending.lock();
    std::map<std::string, std::vector<StorageData *> *>::iterator it = mMapSending.find(groupSId);
    if (it == mMapSending.end()) {
        mMutexSending.unlock();
        return false;
    }
    std::vector<StorageData *> * vec = it->second;
    if (vec->empty()) {
        mMutexSending.unlock();
        return false;
    }
    mMapInterimSend.insert(std::pair<std::string, std::vector<StorageData *> *>(groupSId, vec));
    mMutexSending.unlock();
    StorageData * maxData = NULL;
    int i = 0;
    size_t leng = vec->size();
    for (; i < leng; ++i) {
        StorageData * data = NULL;
        try {
            data = vec->at(i);
        } catch (const std::out_of_range & ex) {
            std::cerr << "Line: " << __LINE__ << ", " << ex.what() << std::endl;
            continue;
        }
        if (data == NULL) {
            continue;
        }
        char * buf = data->getData();
        size_t bufSize = data->getDataLength();
        if (!mNetService->sendToOtherParty(buf, bufSize, mAddressInfo, mSocketType)) {
            ++mSendFailedTimes;
        }
        else {
            mSendFailedTimes = ZERO;
        }
        printf("%s:%d %p mSendFailedTimes=%d\n", __FUNCTION__, __LINE__, this, mSendFailedTimes);
        if (i == leng - ONE) {  // maxSequenceId
            maxData = data;
        }
    }
    if (NULL == maxData) {
        mMutexSending.lock();
        mMapInterimSend.erase(groupSId);
        mMutexSending.unlock();
        return false;
    }
    mMutexEndSequenceData.lock();
    std::map<std::string, StorageData *>::iterator itMax = mEndSequenceData.find(groupSId);
    if (itMax == mEndSequenceData.end()) {
        mEndSequenceData.insert(std::pair<std::string, StorageData *>(groupSId, maxData));
    }
    mMutexEndSequenceData.unlock();
    mMutexSending.lock();
    mMapInterimSend.erase(groupSId);
    mMutexSending.unlock();
    return true;
}

bool Sender::requestSend(const std::string & groupSId, const int sequenceId)
{
    if (mIsRelease) {
        printf("%s:%d release sender.\n", __FUNCTION__, __LINE__);
        return false;
    }
    mMutexSending.lock();
    std::map<std::string, std::vector<StorageData *> *>::iterator it = mMapSending.find(groupSId);
    if (it == mMapSending.end()) {
        mMutexSending.unlock();printf("%s:%d.\n", __FUNCTION__, __LINE__);
        return false;
    }
    std::vector<StorageData *> * vec = it->second;
    if (vec->empty()) {
        mMutexSending.unlock();printf("%s:%d.\n", __FUNCTION__, __LINE__);
        return false;
    }
    if (sequenceId >= (const int)vec->size()) {
        mMutexSending.unlock();printf("%s:%d.\n", __FUNCTION__, __LINE__);
        return false;
    }
    StorageData * data = vec->at(sequenceId - ONE);
    if (data == NULL) {
        mMutexSending.unlock();printf("%s:%d.\n", __FUNCTION__, __LINE__);
        return false;
    }
    mMapInterimSend.insert(std::pair<std::string, std::vector<StorageData *> *>(groupSId, vec));
    mMutexSending.unlock();
    char * buf = data->getData();
    size_t bufSize = data->getDataLength();
    bool ret = false;
    if (!(ret = (mNetService->sendToOtherParty(buf, bufSize, mAddressInfo, mSocketType)))) {
        ++mSendFailedTimes;
    }
    else {
        mSendFailedTimes = ZERO;
    }
    printf("%s:%d %p mSendFailedTimes=%d\n", __FUNCTION__, __LINE__, this, mSendFailedTimes);
    mMutexSending.lock();
    mMapInterimSend.erase(groupSId);
    mMutexSending.unlock();
    return ret;
}

bool Sender::requestSend(sockets::AddressInfo * addressInfo)
{
    bool ret = false;
    mMutexRelease.lock();
    mMutexDeque.lock();
    mMutexSending.lock();
    mMutexEndSequenceData.lock();
    std::map<std::string, std::vector<StorageData *> *>::iterator it = mMapSending.begin();
    for (; it != mMapSending.end(); ) {
        std::vector<StorageData *> * vec = it->second;
        if (vec->empty()) {
            mListRelease.push_back(vec);
            it = mMapSending.erase(it);
            continue;
        }
        std::vector<StorageData *>::iterator itVec = vec->begin();
        if (itVec == vec->end()) {
            mListRelease.push_back(vec);
            it = mMapSending.erase(it);
            continue;
        }
        int counter = 0;
        for (; itVec != vec->end(); ++itVec) {
            StorageData * data = *itVec;
            char * buf = data->getData();
            size_t bufSize = data->getDataLength();
            ret = mNetService->sendToOtherParty(buf, bufSize, addressInfo, mSocketType);
            if (!ret) {
                printf("resend data failed, when reconnected:%d.\n", ++counter);
                ret = mNetService->sendToOtherParty(buf, bufSize, addressInfo, mSocketType);
                if (ret) {
                    printf("resend data success, when reconnected.\n");
                }
            }
            mVecRelease.push_back(data);
            itVec = vec->erase(itVec);
        }
        vec->clear();
        mListRelease.push_back(vec);
    }
    delete mAddressInfo;
    mAddressInfo = mNetService->getAddressInfo(addressInfo, mSocketType);
    responseReconnectedAck();
    mMutexEndSequenceData.unlock();
    mMutexSending.unlock();
    mMutexDeque.unlock();
    mMutexRelease.unlock();
    return ret;
}

/* server ---> client */
bool Sender::responseReconnectedAck() {
    if (NULL == mAddressInfo) {
        return false;
    }
    if (mSocketType != TCP_SERVER) {
        return false;
    }
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_0,0,%d,tcp,%d,0,0{}",
            mAddressInfo->getRemoteIPAddress().c_str(),
            mAddressInfo->getRemotePort(),
            CLIENT_RECONNECTED_PEER_ACK,
            TYPE_TEXT);
    return mNetService->sendToOtherParty(buf, strlen(buf), mAddressInfo, mSocketType);
}

void Sender::updateTime()
{
    clock_gettime(CLOCK_BOOTTIME, &mTime);
}
void Sender::updateRequestGroupIdTime()
{
    clock_gettime(CLOCK_BOOTTIME, &mRequestGroupIdTime);
}

bool Sender::isTimeout(unsigned long long timeInterval)
{
    struct timespec newTime;
    memset(&newTime, ZERO, sizeof(struct timespec));
    clock_gettime(CLOCK_BOOTTIME, &newTime);
    unsigned long long newTimeMS = (newTime.tv_sec * ONE_THOUSAND) + (newTime.tv_nsec / ONE_MILLION);
    unsigned long long oldTimeMS = (mTime.tv_sec * ONE_THOUSAND) + (mTime.tv_nsec / ONE_MILLION);
    return (timeInterval < newTimeMS - oldTimeMS);
}
bool Sender::isTimeoutRequestGroupId(unsigned long long timeInterval)
{
    struct timespec newTime;
    memset(&newTime, ZERO, sizeof(struct timespec));
    clock_gettime(CLOCK_BOOTTIME, &newTime);
    unsigned long long newTimeMS = (newTime.tv_sec * ONE_THOUSAND) + (newTime.tv_nsec / ONE_MILLION);
    unsigned long long oldTimeMS = (mRequestGroupIdTime.tv_sec * ONE_THOUSAND) + (mRequestGroupIdTime.tv_nsec / ONE_MILLION);
    return (timeInterval < newTimeMS - oldTimeMS);
}

bool Sender::resendEndSequenceData()
{
    if (mIsRelease) {
        printf("%s:%d release sender.\n", __FUNCTION__, __LINE__);
        return false;
    }
    if (!mMutexEndSequenceData.try_lock()) {
        return true;
    }
    bool ret = true;
    std::map<std::string, StorageData *>::iterator itMax = mEndSequenceData.begin();
    if (itMax == mEndSequenceData.end()) {
        ret = false;
    }
    if (mIsConnected) {
        for ( ; itMax != mEndSequenceData.end(); ) {
            StorageData * data = itMax->second;
            if (!data->isTimeout(mLastSendIntervalTime)) {  // 500ms.
                if (TEN < data->getEndSequenceDataCounter()) {
                    itMax = mEndSequenceData.erase(itMax);
                    std::string groupSId;
                    if (itMax != mEndSequenceData.end()) {
                        groupSId = itMax->first;  // record groupSId, it may be erase in assembleComplete().
                    }
                    mMutexEndSequenceData.unlock();
                    assembleComplete(data->getGroupSId(), data->getMaxSequenceId(), false);
                    printf("%s:%d.\n", __FUNCTION__, __LINE__);
                    mMutexEndSequenceData.lock();
                    if (groupSId.empty()) {
                        break;
                    }
                    // The existence of the itMax must be determined, otherwise it is possible that StorageData has
                    // been destroyed in the mEndSequenceData of assembleComplete(),
                    // causing invalid pointers to crash the process.
                    // 必须判断itMax是否存在,否则可能会在函数 assembleComplete()里mEndSequenceData中销毁StorageData,
                    // 导致无效指针引起进程crash.
                    if (mEndSequenceData.end() == mEndSequenceData.find(groupSId)) {
                        printf("%s:%d %s has erased in mEndSequenceData.", __FUNCTION__, __LINE__, groupSId.c_str());
                        itMax = mEndSequenceData.begin();  // starting from head of mEndSequenceData(从头开始遍历).
                    }
                }
                continue;
            }
            char * buf = data->getData();
            size_t bufSize = data->getDataLength();
            if (!mNetService->sendToOtherParty(buf, bufSize, mAddressInfo, mSocketType)) {
                ++mSendFailedTimes;
            }
            else {
                mSendFailedTimes = ZERO;
            }
            ++itMax;
        }
    }
    mMutexEndSequenceData.unlock();
    if (TEN < mSendFailedTimes) {
        printf("%s:%d %p mSendFailedTimes=%d\n", __FUNCTION__, __LINE__, this, mSendFailedTimes);
        switch (mSocketType) {
            case TCP_SERVER: {
                std::string localNetIp(mLocalNetIp);
                unsigned short localNetPort(mLocalNetPort);
                printf("%s:%d.\n", __FUNCTION__, __LINE__);
                if (!localNetIp.empty()) {
                    printf("%s:%d.\n", __FUNCTION__, __LINE__);
                    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
                    // send CLIENT_CLOSE_PEER to tcp client from tcp server, then close client socket fd in tcp server.
                    snprintf(buf, sizeof(buf), "%s_%d_0,0,%d,tcp,%d,0,0{}",
                            localNetIp.c_str(),
                            localNetPort,
                            CLIENT_CLOSE_PEER,
                            TYPE_TEXT);
                    mNetService->sendToOtherParty(buf, strlen(buf), mAddressInfo, mSocketType);
                }printf("%s:%d.\n", __FUNCTION__, __LINE__);
                mNetService->closeClientSocketFd(mAddressInfo, mSocketType);
                break;
            }
            case TCP_CLIENT: {
                // 1. record current remote ip and remote port.
                std::string localNetIp(mLocalNetIp);
                unsigned short localNetPort(mLocalNetPort);
                char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
                printf("%s:%d.\n", __FUNCTION__, __LINE__);
                if (!localNetIp.empty()) {
                    printf("%s:%d.\n", __FUNCTION__, __LINE__);
                    // 2. send SERVER_CLOSE_PEER to tcp server from tcp client.
                    snprintf(buf, sizeof(buf), "%s_%d_0,0,%d,tcp,%d,0,0{}",
                            localNetIp.c_str(),
                            localNetPort,
                            SERVER_CLOSE_PEER,
                            TYPE_TEXT);
                    mNetService->sendToOtherParty(buf, strlen(buf), mAddressInfo, mSocketType);
                }
                // 3. reconnect tcp server.
                sockets::AddressInfo * _addressInfo = mNetService->tcpClientReconnect(mAddressInfo);
                if (NULL != _addressInfo) {
                    sockets::AddressInfo * addressInfo = mNetService->getAddressInfo(_addressInfo, mSocketType);
                    mAddressInfo->setRequest(addressInfo->getRequest());
                    mAddressInfo->setClientFd(addressInfo->getClientFd());
                    mAddressInfo->setServerFd(addressInfo->getServerFd());
                    mIsConnected = true;
                    printf("reconnect success: %p\n", this);
                } else {
                    mIsConnected = false;
                    // notify upper level.
                    printf("reconnect failed: %p\n", this);
                    break;
                }
                if (!localNetIp.empty()) {
                    printf("%s:%d.\n", __FUNCTION__, __LINE__);
                    // 4. send SERVER_RECONNECTED_PEER to tcp server from tcp client.
                    // resend data that was not sent last time in tcp client.
                    memset(buf, ZERO, sizeof(buf));
                    snprintf(buf, sizeof(buf), "%s_%d_0,0,%d,tcp,%d,0,0{}",
                            localNetIp.c_str(),
                            localNetPort,
                            SERVER_RECONNECTED_PEER,
                            TYPE_TEXT);
                    mNetService->sendToOtherParty(buf, strlen(buf), mAddressInfo, mSocketType);
                    // 5. let tcp server resend data that was not sent last time in server.
                }printf("%s:%d.\n", __FUNCTION__, __LINE__);
                break;
            }
            case UDP_SERVER: {
                break;
            }
            case UDP_CLIENT: {
                break;
            }
        }
    }
    return ret;
}

bool Sender::requestGroupId()
{
    if (mIsRelease) {
        printf("%s:%d release sender.\n", __FUNCTION__, __LINE__);
        return false;
    }
    if (!mIsConnected) {
        printf("%s:%d mIsConnected is false.\n", __FUNCTION__, __LINE__);
        return false;
    }
    if (isTimeoutRequestGroupId(mLastRequestGroupIntervalTime)) {
        updateRequestGroupIdTime();
    } else {printf("%s:%d.\n", __FUNCTION__, __LINE__);
        return true;
    }
    mMutexSending.lock();
    printf("%s:%d  map empty:%d\n", __FUNCTION__, __LINE__, mMapSending.empty());
    if (NULL == mAddressInfo) {
        mMutexSending.unlock();
        printf("%s:%d mAddressInfo is NULL.\n", __FUNCTION__, __LINE__);
        return false;
    }
    printf("%s:%d  deque empty:%d  size:%ld  Sender:%p  remotePort:%d\n",
            __FUNCTION__,
            __LINE__,
            mDequeStorageData.empty(),
            mDequeStorageData.size(),
            this,
            mAddressInfo->getRemotePort());
    if (mMapSending.empty()) {
        mMutexSending.unlock();
        mMutexDeque.lock();
        bool ret = mDequeStorageData.empty();
        mMutexDeque.unlock();
        if (ret) {printf("%s:%d.\n", __FUNCTION__, __LINE__);
            return !ret;
        }
    }
    mMutexDeque.lock();  // 注意和 packet() 不要造成死锁.
    if (mDequeStorageData.empty()) {
        mMutexDeque.unlock();  // 先释放这把锁.
        mMutexSending.unlock();printf("%s:%d.\n", __FUNCTION__, __LINE__);
        return true;
    }
    StorageData * storageData = mDequeStorageData.front();
    const size_t bufferSize = storageData->getDataLength();
    const int dataType = storageData->getDataType();
    mMutexDeque.unlock();
    mMutexSending.unlock();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    switch (mSocketType) {
        case UDP_SERVER: // from client get group id. send to udp client.
            snprintf(buf, sizeof(buf), "0,0,%d,udp,%d,%ld,0{}",
                    CLIENT_REQUEST_GROUP, dataType, bufferSize);
            break;
        case TCP_SERVER: // from client get group id. send to tcp client.
            snprintf(buf, sizeof(buf), "0,0,%d,tcp,%d,%ld,0{}",
                    CLIENT_REQUEST_GROUP, dataType, bufferSize);
            break;
        case UDP_CLIENT: // from server get group id. send to udp server.
            snprintf(buf, sizeof(buf), "0,0,%d,udp,%d,%ld,0{}",
                    SERVER_REQUEST_GROUP, dataType, bufferSize);
            break;
        case TCP_CLIENT: // from server get group id. send to tcp server.
            snprintf(buf, sizeof(buf), "0,0,%d,tcp,%d,%ld,0{}",
                    SERVER_REQUEST_GROUP, dataType, bufferSize);
            break;
        default: {printf("%s:%d.\n", __FUNCTION__, __LINE__);
            return true;
        }
    }
    bool ret = false;
    if (!(ret = mNetService->sendToOtherParty(buf, strlen(buf), mAddressInfo, mSocketType))) {
        ++mSendFailedTimes;
        if (isTimeout(SIXTY * ONE_THOUSAND)) {
            mIsConnected = false;
        }
    }
    else {
        mSendFailedTimes = ZERO;
        updateTime();
    }
    printf("%s:%d %p mSendFailedTimes=%d\n", __FUNCTION__, __LINE__, this, mSendFailedTimes);
    return ret;
}

bool Sender::assembleComplete(const std::string & groupSId, const int maxSequenceId, bool isSuccess)
{
    if (mIsRelease) {
        printf("%s:%d release sender.\n", __FUNCTION__, __LINE__);
        return false;
    }
    void * upperPointer = NULL;
    mMutexEndSequenceData.lock();
    std::map<std::string, StorageData *>::iterator itMax = mEndSequenceData.find(groupSId);
    if (itMax != mEndSequenceData.end()) {
        StorageData * data = itMax->second;
        if (data != NULL) {
            long long intervalTime = getCurrentBootTime() - data->getStartTimer();
            if (intervalTime <= ZERO) {
                intervalTime = TEN;
            }
            else if (intervalTime >= ONE_THOUSAND) {
                intervalTime = ONE_THOUSAND;
            }
            mLastRequestGroupIntervalTime = intervalTime;
            mLastSendIntervalTime = intervalTime;
            upperPointer = data->getUpperPointer();
        }
    }
    mEndSequenceData.erase(groupSId);
    mMutexEndSequenceData.unlock();
    mMutexRelease.lock();
    mMutexSending.lock();
printf("%s:%d, mMapSending:%ld\n", __FUNCTION__, __LINE__, mMapSending.size());
    std::map<std::string, std::vector<StorageData *> *>::iterator it = mMapSending.find(groupSId);
    if (it == mMapSending.end()) {
        mMutexSending.unlock();printf("%s:%d, mMapSending:%ld\n", __FUNCTION__, __LINE__, mMapSending.size());
        mMutexRelease.unlock();
        return false;
    }
    std::vector<StorageData *> * vec = it->second;
    mMapSending.erase(groupSId);
    std::vector<StorageData *>::iterator itRV = vec->begin();
    for (; itRV != vec->end(); ) {
        mVecRelease.push_back(*itRV);
        itRV = vec->erase(itRV);
    }
    vec->clear();
    mListRelease.push_back(vec);
    mMutexSending.unlock();
    mMutexRelease.unlock();
    mNetService->replyUpper(this, isSuccess, upperPointer);
    return true;
}

bool Sender::ackCompleted(const std::string & groupSId, const int maxSequenceId)
{
    if (mIsRelease) {
        printf("%s:%d release sender.\n", __FUNCTION__, __LINE__);
        return false;
    }
    mMutexRelease.lock();
    std::list<std::vector<StorageData *> *>::iterator itListRelease = mListRelease.begin();
    for (; itListRelease != mListRelease.end(); ) {
        std::vector<StorageData *> * vecR = *itListRelease;
        {
            mMutexSending.lock();
            std::map<std::string, std::vector<StorageData *> *>::iterator it = mMapInterimSend.find(groupSId);
            if (it != mMapInterimSend.end()) {
                mMutexSending.unlock();
                continue;
            }
            itListRelease = mListRelease.erase(itListRelease);
            mMutexEndSequenceData.lock();
            mEndSequenceData.erase(groupSId);
            mMutexEndSequenceData.unlock();
            mMutexSending.unlock();
        }
        delete vecR;
    }
    std::vector<StorageData *>::iterator itVecRelease = mVecRelease.begin();
    for (; itVecRelease != mVecRelease.end(); ) {
        StorageData * data = *itVecRelease;
        itVecRelease = mVecRelease.erase(itVecRelease);
        delete data;
    }
    mMutexRelease.unlock();

    return mNetService->requestGroupId(this);
}

bool Sender::closeFromPeer()
{
    mMutexRelease.lock();
    mMutexDeque.lock();
    mMutexSending.lock();
    mMutexEndSequenceData.lock();
    // response peer.
    mNetService->closeClientSocketFd(mAddressInfo, mSocketType);
    mIsConnected = false;
    mMutexEndSequenceData.unlock();
    mMutexSending.unlock();
    mMutexDeque.unlock();
    mMutexRelease.unlock();
    return true;
}
bool Sender::closeOverFromPeer()
{
    mMutexRelease.lock();
    mMutexDeque.lock();
    mMutexSending.lock();
    mMutexEndSequenceData.lock();
    // response peer.
    mNetService->closeClientSocketFd(mAddressInfo, mSocketType);
    mIsConnected = false;
    mMutexEndSequenceData.unlock();
    mMutexSending.unlock();
    mMutexDeque.unlock();
    mMutexRelease.unlock();
    return true;
}
long long Sender::getCurrentBootTime()
{
    struct timespec currentBootTime;
    clock_gettime(CLOCK_BOOTTIME, &currentBootTime);
    return ((currentBootTime.tv_sec * ONE_THOUSAND) + (currentBootTime.tv_nsec / ONE_MILLION));
}

};  // namespace protocols

