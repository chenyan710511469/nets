/*
 * =====================================================================================
 *
 *       Filename:  HandleClientData.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  09/06/2019 09:02:27 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/HandleClientData.h"

namespace protocols
{
HandleClientData::HandleClientData(ProtocolService * pService, ResponseOtherPartyInfo * response)
 : mProtocolService(pService)
 , mResponse(response)
{
}

HandleClientData::~HandleClientData()
{
    mMutexGroupClient.lock();
    std::map<std::string, ProtocolGroup *>::iterator itMap = mMapGroupClient.begin();
    while (itMap != mMapGroupClient.end()) {
        ProtocolGroup * pGroup = itMap->second;
        itMap = mMapGroupClient.erase(itMap);
        delete pGroup;
    }
    std::vector<ProtocolGroup *>::iterator itVec = mVecReleaseGroup.begin();
    while (itVec != mVecReleaseGroup.end()) {
        ProtocolGroup * pGroup = *itVec;
        itVec = mVecReleaseGroup.erase(itVec);
        delete pGroup;
    }
    mMutexGroupClient.unlock();
}

bool HandleClientData::handleProtocolData(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    bool ret = false;
    const int code = pData->getCode();
    switch (code)
    {
        case CLIENT_REQUEST_GROUP: {
            return requestGroup(pData, addressInfo, socketType);
        }
        case CLIENT_REPLY_GROUP: {
            return replyGroup(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case CLIENT_ACK_GROUP: {
            return ackGroup(pData, addressInfo, socketType);
        }
        case CLIENT_GROUP_NOT_EXIST: {
            return groupNotExist(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case CLIENT_REDUNDANT_GROUP: {
            return redundantGroup(pData, addressInfo, socketType);
        }
        case CLIENT_SEQUENCE_DATA:
        case CLIENT_REPLACE_WITH_SEQUENCE_DATA: {
            return sequenceData(pData, addressInfo, socketType);
        }
        case CLIENT_REQUEST_SEQUENCE_DATA: {
            return requestSequenceData(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case CLIENT_ACK_SEQUENCE: {
            return ackSequenceData(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case CLIENT_END_SEQUENCE: {
            return endSequence(pData, addressInfo, socketType);
        }
        case CLIENT_ASSEMBLE_COMPLETE: {
            return assembleComplete(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case CLIENT_ACK_ASSEMBLE_COMPLETE: {
            return ackAssembleComplete(pData, addressInfo, socketType);
        }
        case CLIENT_ACK_COMPLETED: {
            return ackCompleted(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case CLIENT_RECONNECTED_PEER_ACK: {
            return reconnectedPeerAck(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case CLIENT_CLOSE_PEER: {
            return closePeer(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case CLIENT_CLOSE_PEER_OVER: {
            return closePeerOver(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case CLIENT_CLOSE_PEER_OVER_ACK: {
            return closePeerOverAck(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        default: {
            delete pData;
            pData = NULL;
            return false;
        }
    }
    return false;
}

/* server ---> client */  // server send to client.
bool HandleClientData::requestGroup(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    printf("%s:%d.\n", __FUNCTION__, __LINE__);
    int groupId = ONE;
    std::string remoteIp(const_cast<sockets::AddressInfo *>(addressInfo)->getRemoteIPAddress());
    unsigned short remotePort(const_cast<sockets::AddressInfo *>(addressInfo)->getRemotePort());
    printf("%s:%d. remote address:%s:%d.\n", __FUNCTION__, __LINE__, remoteIp.c_str(), remotePort);
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    mMutexGroupClient.lock();
    bool flag = false;
    if(!mMapGroupClient.empty()) {
        size_t size = mMapGroupClient.size();
        for (int i=ONE; i <= size; ++i) {
            if (i >= INT_MAX) {
                i = ONE;
            }
            memset(buf, ZERO, sizeof(buf));
            snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, i);
            std::string tmpGroupSId(buf);
            std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupClient.find(tmpGroupSId);
            if (it == mMapGroupClient.end()) {
                groupId = i;
                flag = true;
                break;
            }
        }
        if (!flag) {
            groupId += size;
        }
    }
    flag = false;
    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string keyGroupSId(buf);
    ProtocolGroup * pGroup = new ProtocolGroup();
    std::pair<std::map<std::string, ProtocolGroup *>::iterator, bool> ret;
    do
    {
        pGroup->setGroupId(groupId);
        ret = mMapGroupClient.insert(std::pair<std::string, ProtocolGroup *>(keyGroupSId, pGroup));
        flag = ret.second;
        if (!flag) {
            ++groupId;
            memset(buf, 0, TWO_HUNDRED_AND_FIFTY_SIX);
            snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
            keyGroupSId = std::string(buf);
        }
    } while(!flag);
    mMutexGroupClient.unlock();

    memset(buf, ZERO, sizeof(buf));
    int sequenceId = 0;
    snprintf(buf, sizeof(buf), "%d,%d", pGroup->getGroupId(), sequenceId);
    std::string key(buf);
    ProtocolSequence * pSequence = new ProtocolSequence();
    pSequence->setSequence(sequenceId);
    pSequence->setProtocolData(pData);
    if(!pGroup->addMapSequence(key, pSequence)) {
        delete pSequence;
        pSequence = NULL;
    }
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%d,0{}",
            remoteIp.c_str(),
            remotePort,
            pGroup->getGroupId(),
            sequenceId,
            SERVER_REPLY_GROUP,
            pData->getProtocol().c_str(),
            pData->getType(),
            pData->getDataTotalLength());
    size_t bufSize = strlen(buf);
    bool res = mResponse->sendToOtherParty(buf, bufSize, addressInfo, socketType);
    if (res) {
        printf("%s:%d reply group success: %s.\n", __FUNCTION__, __LINE__, buf);
    } else {
        printf("%s:%d reply group error: %s.\n", __FUNCTION__, __LINE__, buf);
    }
    return res;
}

/* client ---> server */
bool HandleClientData::replyGroup(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    int groupId = pData->getGroup();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    printf("%s:%d buf:%s\n", __FUNCTION__, __LINE__, buf);
    std::string groupSId(buf);
    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%d,0{}",
            remoteIp.c_str(),
            remotePort,
            groupId,
            pData->getSequence(),
            SERVER_ACK_GROUP,
            pData->getProtocol().c_str(),
            pData->getType(),
            pData->getDataTotalLength());
    size_t dataSize = strlen(buf);printf("%s:%d buf:%s\n", __FUNCTION__, __LINE__, buf);
    mResponse->sendToOtherParty(buf, dataSize, addressInfo, socketType);
    delete pData;
    pData = NULL;
    //  这里通知上层发送数据.
    mProtocolService->notifyRequestGroupIdSuccess(groupSId, addressInfo, socketType);
    return true;
}

/* server ---> client */
bool HandleClientData::ackGroup(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    int groupId = pData->getGroup();
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    mMutexGroupClient.lock();
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupClient.find(groupSId);
    if (it != mMapGroupClient.end()) {
        ProtocolGroup * pGroup = it->second;
        pGroup->setAckGroup(true);
        mMutexGroupClient.unlock();
        delete pData;
        return true;
    }
    ProtocolGroup * pGroup = new ProtocolGroup();
    pGroup->setGroupId(groupId);
    memset(buf, ZERO, sizeof(buf));
    int sequenceId = 0;
    snprintf(buf, sizeof(buf), "%d,%d", pGroup->getGroupId(), sequenceId);
    std::string key(buf);
    ProtocolSequence * pSequence = new ProtocolSequence();
    pSequence->setSequence(sequenceId);
    pSequence->setProtocolData(pData);
    if (!pGroup->addMapSequence(key, pSequence)) {
        delete pSequence;
        pSequence = NULL;
    }
    pGroup->setAckGroup(true);
    std::pair<std::map<std::string, ProtocolGroup *>::iterator, bool> ret
    = mMapGroupClient.insert(std::pair<std::string, ProtocolGroup *>(groupSId, pGroup));
    mMutexGroupClient.unlock();
    if (!ret.second) {
        delete pGroup;
        pGroup = NULL;
    }
    return ret.second;
}

/* client ---> server */
bool HandleClientData::groupNotExist(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    // 再次向client端请求groupId ?
    delete pData;
    pData = NULL;
    return false;
}

/* server---> client */
bool HandleClientData::redundantGroup(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    delete pData;
    pData = NULL;
    return true;
}

/* server ---> client*/
bool HandleClientData::sequenceData(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    int groupId = pData->getGroup();
    int sequenceId = pData->getSequence();
    if(0 >= sequenceId || 0 >= groupId) {
        delete pData;
        pData = NULL;
        return false;
    }
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    mMutexGroupClient.lock();
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupClient.find(groupSId);
    ProtocolGroup * pGroup = NULL;
    if(it == mMapGroupClient.end()) {
        pGroup = new ProtocolGroup();
        char bufKey[ONE_HUNDRED_AND_TWENTY_EIGHT] = {ZERO};
        int sequenceIdTmp = 0;
        sprintf(bufKey, "%d,%d", pGroup->getGroupId(), sequenceIdTmp);
        std::string key(bufKey);
        ProtocolSequence * pSequenceTmp = new ProtocolSequence();
        pSequenceTmp->setSequence(sequenceIdTmp);
        // pSequenceTmp->setProtocolData(pData); //  此处不能设置,不然析构释放时两次delete.
        if (!pGroup->addMapSequence(key, pSequenceTmp)) {
            delete pSequenceTmp;
            pSequenceTmp = NULL;
            delete pGroup;
            pGroup = NULL;
            delete pData;
            pData = NULL;
            mMutexGroupClient.unlock();
            return false;
        }
        std::pair<std::map<std::string, ProtocolGroup *>::iterator, bool> ret
        = mMapGroupClient.insert(std::pair<std::string, ProtocolGroup *>(groupSId, pGroup));
        if (!ret.second) {
            delete pGroup;
            pGroup = ret.first->second;
        }
    } else {
        pGroup = it->second;
    }
    mMutexGroupClient.unlock();
    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d,%d", groupId, sequenceId);
    std::string key(buf);
    ProtocolSequence * pSequence = new ProtocolSequence();
    pSequence->setSequence(sequenceId);
    pSequence->setProtocolData(pData);  // 看上一条注释.
    if(!pGroup->addMapSequence(key, pSequence)) {
        delete pSequence;
        pSequence = NULL;
        return false;
    }
    memset(buf, ZERO, sizeof(buf));
    const int type = pData->getType();
    const size_t dataTotalLength = pData->getDataTotalLength();
    snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%lu,0{}",
            remoteIp.c_str(),
            remotePort,
            groupId,
            sequenceId,
            SERVER_ACK_SEQUENCE,
            pData->getProtocol().c_str(),
            type,
            dataTotalLength);
    size_t size = strlen(buf);
    return mResponse->sendToOtherParty(buf, size, addressInfo, socketType);
}

/* client ---> server */
bool HandleClientData::requestSequenceData(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    printf("%s:%d.\n", __FUNCTION__, __LINE__);
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    int groupId = pData->getGroup();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    int sequenceId = pData->getSequence();
    bool ret = mProtocolService->requestSequenceData(groupSId, sequenceId, addressInfo, socketType);
    delete pData;
    pData = NULL;
    return ret;
}

/* client ---> server */
bool HandleClientData::ackSequenceData(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    int groupId = pData->getGroup();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    int sequenceId = pData->getSequence();
    bool ret = mProtocolService->ackSequenceData(groupSId, sequenceId, addressInfo, socketType);
    delete pData;
    pData = NULL;
    return ret;
}

/* server ---> client */
bool HandleClientData::endSequence(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    std::string remoteIp(pData->getRemoteIp());
    const unsigned short remotePort(pData->getRemotePort());
    const int groupId = pData->getGroup();
    const int maxSequenceId = pData->getSequence();  // max sequence id.
    const int protocolLength = pData->getProtocol().size();
    char cProtocol[protocolLength + ONE] = {ZERO};
    strncpy(cProtocol, pData->getProtocol().c_str(), protocolLength);
    const int type = pData->getType();
    const size_t dataTotalLength = pData->getDataTotalLength();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    mMutexGroupClient.lock();
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupClient.find(groupSId);
    if(it == mMapGroupClient.end()) {
        mMutexGroupClient.unlock();
        memset(buf, ZERO, sizeof(buf));
        snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%lu,0{}",
                remoteIp.c_str(),
                remotePort,
                groupId,
                maxSequenceId,
                SERVER_GROUP_NOT_EXIST,
                cProtocol,
                type,
                dataTotalLength);
        size_t bufSize = strlen(buf);
        bool ret = mResponse->sendToOtherParty(buf, bufSize, addressInfo, socketType);
        delete pData;
        pData = NULL;printf("%s:%d\n", __FUNCTION__, __LINE__);
        return false;
    }
    ProtocolGroup * pGroup = it->second;
    if(pGroup->getAssembleComplete()) { // the groupId data has assembled completed, so return.
        mMutexGroupClient.unlock();
        memset(buf, ZERO, sizeof(buf));
        snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%lu,0{}",
                remoteIp.c_str(),
                remotePort,
                groupId,
                maxSequenceId,
                SERVER_ASSEMBLE_COMPLETE,
                cProtocol,
                type,
                dataTotalLength);
        size_t dataSize = strlen(buf);
        mResponse->sendToOtherParty(buf, dataSize, addressInfo, socketType);
        delete pData;
        pData = NULL;printf("%s:%d\n", __FUNCTION__, __LINE__);
        return false;
    }
    if(pGroup->getAssemble()) { // other thread is assembling data, so return.
        mMutexGroupClient.unlock();
        delete pData;
        pData = NULL;printf("%s:%d\n", __FUNCTION__, __LINE__);
        return false;
    }
    pGroup->setAssemble(true);
    mMutexGroupClient.unlock();
    ProtocolSequence * pSequence = new ProtocolSequence();
    pSequence->setSequence(maxSequenceId);
    pSequence->setProtocolData(pData);
    pSequence->setIsMaxSequence(true);
    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, ONE_HUNDRED_AND_TWENTY_EIGHT, "%d,%d", groupId, maxSequenceId);
    std::string key(buf);
    if(!pGroup->addMapSequence(key, pSequence)) {
        delete pSequence;
        pSequence = NULL;
        // here don't return, the ProtocolGroup can have no maxSequence when assemble data.
        // 不用返回, 在组装合并数据时,ProtocolGroup可以没有maxSequence.
    }
    char * data = NULL;
    size_t dataLength = ZERO;
    bool isComplete = true; // The hypothesis all data is collected(假设数据已收齐).
    for(int i = ONE; i < maxSequenceId; ++i) {
        memset(buf, ZERO, sizeof(buf));
        snprintf(buf, sizeof(buf), "%d,%d", groupId, i);
        key = std::string(buf);
        pSequence = pGroup->getSequence(key);
        memset(buf, ZERO, sizeof(buf));
        snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%lu,0{}",
                remoteIp.c_str(),
                remotePort,
                groupId,
                i,
                SERVER_REQUEST_SEQUENCE_DATA,
                cProtocol, // The line has crashed.
                type,
                dataTotalLength);
        size_t dataSize = strlen(buf);
        if (NULL == pSequence) {
            isComplete = false; // The hypothesis is not valid, because the all data are not collected(假设不成立, 数据未全部收齐).
            mResponse->sendToOtherParty(buf, dataSize, addressInfo, socketType);
            continue;
        }
        ProtocolData * tData = pSequence->getProtocolData();
        size_t tDataLength = tData->getDataLength();
        if(tData == NULL || NULL == tData->getData() || ZERO >= tDataLength) {
            isComplete = false;
            mResponse->sendToOtherParty(buf, dataSize, addressInfo, socketType);
            continue;
        }
        if (!isComplete) {
            continue;
        }
        if(data == NULL) {
            if (NULL == tData->getData()) {
                free(data);
                data = NULL;
                continue;
            }
            while (NULL == (data = (char *)malloc(tDataLength + ONE)));
            memset(data, ZERO, tDataLength + ONE);
            memcpy(data, tData->getData(), tDataLength); // The line has crashed.
            dataLength = tDataLength;
            continue;
        }
        if (ZERO >= dataLength || NULL == tData->getData()) {
            if (data != NULL) {
                free(data);
                data = NULL;
            }
            isComplete = false;
            continue;
        }
        char * tmpData = data;
        data = NULL;
        while (NULL == (data = (char *)malloc(dataLength + tDataLength + ONE)));
        memset(data, ZERO, dataLength + tDataLength + ONE);
        memcpy(data, tmpData, dataLength);
        memcpy(data + dataLength, tData->getData(), tDataLength); // The line has crashed.
        dataLength += tDataLength;
        free(tmpData);
        tmpData = NULL;
    }
    if(!isComplete) {
        if (data != NULL) {
            free(data);
            data = NULL;
            dataLength = ZERO;
        }
        pGroup->setAssemble(false);
        // mMutexGroupClient.unlock();
        return false;
    }printf("%s:%d\n", __FUNCTION__, __LINE__);
    if(dataLength != dataTotalLength) { // data length error.
        if (data != NULL) {
            free(data);
            data = NULL;
            dataLength = ZERO;
        }
        pGroup->setAssemble(false);
        // mMutexGroupClient.unlock();
        return false;
    }
    pGroup->setAssembleComplete(true);
    pGroup->setAssemble(false);
    // mMutexGroupClient.unlock();

    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%lu,0{}",
            remoteIp.c_str(),
            remotePort,
            groupId,
            maxSequenceId,
            SERVER_ASSEMBLE_COMPLETE,
            cProtocol,
            type,
            dataTotalLength);
    size_t dataSize = strlen(buf);
    bool ret = mResponse->sendToOtherParty(buf, dataSize, addressInfo, socketType);

    StorageData * receivedData = NULL;
    switch (type)
    {
        case TYPE_JSON: {
            receivedData = new StorageData(data, dataLength, TYPE_JSON);
            mProtocolService->receive(receivedData, addressInfo, socketType);
            break;
        }
        case TYPE_XML: {
            receivedData = new StorageData(data, dataLength, TYPE_XML);
            mProtocolService->receive(receivedData, addressInfo, socketType);
            break;
        }
        case TYPE_TEXT:
        default: // if default is treated as text data.
        {
            receivedData = new StorageData(data, dataLength, TYPE_TEXT);
            mProtocolService->receive(receivedData, addressInfo, socketType);
        }
    }
    delete receivedData;
    receivedData = NULL;
    data = NULL;
    dataLength = ZERO;printf("%s:%d\n", __FUNCTION__, __LINE__);
    return ret;
}

/* client ---> server */
bool HandleClientData::assembleComplete(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    int groupId = pData->getGroup();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    int maxSequenceId = pData->getSequence();
    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%d,0{}",
            remoteIp.c_str(),
            remotePort,
            groupId,
            pData->getSequence(),
            SERVER_ACK_ASSEMBLE_COMPLETE,
            pData->getProtocol().c_str(),
            pData->getType(),
            pData->getDataTotalLength());
    size_t dataSize = strlen(buf);
    bool ret = mProtocolService->assembleComplete(groupSId, maxSequenceId, addressInfo, socketType);
    ret = mResponse->sendToOtherParty(buf, dataSize, addressInfo, socketType);
    delete pData;
    pData = NULL;
    return ret;
}

/* server ---> client */
bool HandleClientData::ackAssembleComplete(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    const int groupId = pData->getGroup();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    mMutexGroupClient.lock();
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupClient.find(groupSId);
    if(it != mMapGroupClient.end()) {
        ProtocolGroup * pGroup = it->second;
        mMapGroupClient.erase(it);
        delete pGroup;
        pGroup = NULL;
    }
    mMutexGroupClient.unlock();
    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%d,0{}",
            remoteIp.c_str(),
            remotePort,
            groupId,
            pData->getSequence(),
            SERVER_ACK_COMPLETED,
            pData->getProtocol().c_str(),
            pData->getType(),
            pData->getDataTotalLength());
    size_t dataSize = strlen(buf);
    bool ret = mResponse->sendToOtherParty(buf, dataSize, addressInfo, socketType);
    delete pData;
    pData = NULL;
    return ret;
}

/* client ---> server */
bool HandleClientData::ackCompleted(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    int groupId = pData->getGroup();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    int maxSequenceId = pData->getSequence();
    delete pData;
    pData = NULL;
    return mProtocolService->ackCompleted(groupSId, maxSequenceId, addressInfo, socketType);
}

/* server ---> client */
bool HandleClientData::reconnectedPeerAck(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    sockets::AddressInfo * address = mResponse->getAddressInfo(addressInfo, socketType);
    if (address == NULL) {
        delete pData;
        pData = NULL;
        return false;
    }
    std::string * remoteIpPtr = new std::string(pData->getRemoteIp());
    unsigned short * remotePortPtr = new unsigned short;
    *remotePortPtr = pData->getRemotePort();
    int * groupIdPtr = new int;
    *groupIdPtr = pData->getGroup();
    SocketType * socketTypePtr = new SocketType;
    *socketTypePtr = socketType;
    MessageBase * message = NULL;
    while (NULL == (message = new Message(CLIENT_RECONNECTED_PEER_ACK)));
    std::vector<void *> * vec = message->getVector();
    vec->push_back(remoteIpPtr);
    vec->push_back(remotePortPtr);
    vec->push_back(groupIdPtr);
    vec->push_back(address);
    vec->push_back(socketTypePtr);
    delete pData;
    pData = NULL;
    return mProtocolService->reconnectedPeerAck(message);
}

/* server ---> client */
bool HandleClientData::closePeer(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    sockets::AddressInfo * address = mResponse->getAddressInfo(addressInfo, socketType);
    if (address == NULL) {
        delete pData;
        pData = NULL;
        return false;
    }
    std::string * remoteIpPtr = new std::string(pData->getRemoteIp());
    unsigned short * remotePortPtr = new unsigned short;
    *remotePortPtr = pData->getRemotePort();
    int * groupIdPtr = new int;
    *groupIdPtr = pData->getGroup();
    SocketType * socketTypePtr = new SocketType;
    *socketTypePtr = socketType;
    MessageBase * message = NULL;
    while (NULL == (message = new Message(CLIENT_CLOSE_PEER)));
    std::vector<void *> * vec = message->getVector();
    vec->push_back(remoteIpPtr);
    vec->push_back(remotePortPtr);
    vec->push_back(groupIdPtr);
    vec->push_back(address);
    vec->push_back(socketTypePtr);
    delete pData;
    pData = NULL;
    return mProtocolService->closePeer(message);
}

/* server ---> client */
bool HandleClientData::closePeerOver(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    sockets::AddressInfo * address = mResponse->getAddressInfo(addressInfo, socketType);
    if (address == NULL) {
        delete pData;
        pData = NULL;
        return false;
    }
    std::string * remoteIpPtr = new std::string(pData->getRemoteIp());
    unsigned short * remotePortPtr = new unsigned short;
    *remotePortPtr = pData->getRemotePort();
    int * groupIdPtr = new int;
    *groupIdPtr = pData->getGroup();
    SocketType * socketTypePtr = new SocketType;
    *socketTypePtr = socketType;
    MessageBase * message = NULL;
    while (NULL == (message = new Message(CLIENT_CLOSE_PEER_OVER)));
    std::vector<void *> * vec = message->getVector();
    vec->push_back(remoteIpPtr);
    vec->push_back(remotePortPtr);
    vec->push_back(groupIdPtr);
    vec->push_back(address);
    vec->push_back(socketTypePtr);
    delete pData;
    pData = NULL;
    return mProtocolService->closePeerOver(message);
}

/* server ---> client */
bool HandleClientData::closePeerOverAck(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    sockets::AddressInfo * address = mResponse->getAddressInfo(addressInfo, socketType);
    if (address == NULL) {
        delete pData;
        pData = NULL;
        return false;
    }
    std::string * remoteIpPtr = new std::string(pData->getRemoteIp());
    unsigned short * remotePortPtr = new unsigned short;
    *remotePortPtr = pData->getRemotePort();
    int * groupIdPtr = new int;
    *groupIdPtr = pData->getGroup();
    SocketType * socketTypePtr = new SocketType;
    *socketTypePtr = socketType;
    MessageBase * message = NULL;
    while (NULL == (message = new Message(CLIENT_CLOSE_PEER_OVER_ACK)));
    std::vector<void *> * vec = message->getVector();
    vec->push_back(remoteIpPtr);
    vec->push_back(remotePortPtr);
    vec->push_back(groupIdPtr);
    vec->push_back(address);
    vec->push_back(socketTypePtr);
    delete pData;
    pData = NULL;
    return mProtocolService->closePeerOverAck(message);
}

void HandleClientData::releaseMapGroupClient()
{
    if (!mMutexGroupClient.try_lock()) {
        return;
    }
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupClient.begin();
    for (; it != mMapGroupClient.end(); ) {
        ProtocolGroup * pGroup = it->second;
        if (pGroup->getAssemble()) { // because endSequence() function is assembling data.
            continue;
        }
        if (pGroup->isTimeout(SIXTY)) {
            it = mMapGroupClient.erase(it);
            mVecReleaseGroup.push_back(pGroup);
        } else {
            ++it;
        }
    }
    std::vector<ProtocolGroup *>::iterator itV = mVecReleaseGroup.begin();
    for (; itV != mVecReleaseGroup.end(); ) {
        ProtocolGroup * pGroup = *itV;
        if (pGroup->getAssemble()) { // because endSequence() function is assembling data.
            continue;
        }
        if (pGroup->isTimeout(ONE_HUNDRED_AND_TWENTY)) {
            itV = mVecReleaseGroup.erase(itV);
            delete pGroup;
            pGroup = NULL;
        } else {
            ++itV;
        }
    }
    mMutexGroupClient.unlock();
}

};
// namespace protocols
