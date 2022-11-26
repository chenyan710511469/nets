/*
 * =====================================================================================
 *
 *       Filename:  HandleServerData.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/16/2019 06:58:10 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/HandleServerData.h"

namespace protocols
{
HandleServerData::HandleServerData(ProtocolService * pService, ResponseOtherPartyInfo * response, HandleClientData * handleClientData)
 : mProtocolService(pService)
 , mResponse(response)
 , mHandleClientData(handleClientData)
{
}

HandleServerData::~HandleServerData()
{
    mMutexGroupServer.lock();
    std::map<std::string, ProtocolGroup *>::iterator itMap = mMapGroupServer.begin();
    while (itMap != mMapGroupServer.end()) {
        ProtocolGroup * pGroup = itMap->second;
        itMap = mMapGroupServer.erase(itMap);
        delete pGroup;
    }
    std::vector<ProtocolGroup *>::iterator itVec = mVecReleaseGroup.begin();
    while (itVec != mVecReleaseGroup.end()) {
        ProtocolGroup * pGroup = *itVec;
        itVec = mVecReleaseGroup.erase(itVec);
        delete pGroup;
    }
    mMutexGroupServer.unlock();
}

bool HandleServerData::parseProtocolData(char * data, const size_t dataSize,
    const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    if(NULL == data) {
        return false;
    }
    if(0 >= dataSize) {
        return false;
    }
    if(socketType <= SOCKET_MIN || socketType >= SOCKET_MAX) {
        return false;
    }
    size_t parseDataSize = 0;
    // start to parse protocol data.
    char * p = data;
unpack:
    if(parseDataSize >= dataSize) {
        printf("parse completed.\n");
        return true;
    }
    int index = 0;
    char * pTmp = p;
    printf("port=%d.\npTmp=%s\n", const_cast<sockets::AddressInfo *>(addressInfo)->getRemotePort(), pTmp);
    while(*p)
    {
        char c = *p;
        if(c == ',') {
            break;
        }
        if(c > '9' || c < '0') {
            printf("protocol data error:length.\n");
            p = checkNextPackageData(p);
            if (p == NULL) {
                return false;
            } else {
                parseDataSize = 0;
                goto unpack;
            }
        }
        ++index;
        ++p;
    }
    if(0 >= index) {
        printf("protocol data error:length.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    char cLength[index + 1] = {0};
    memcpy(cLength, pTmp, index);
    int length = atoi(cLength);
    parseDataSize += length;
    if (parseDataSize > dataSize) {
        printf("protocol data error: parseDataSize(%ld) is more than dataSize(%ld).\n",
                parseDataSize, dataSize);
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    char * next = pTmp + length;
    char ch = *(next - 1);
    if(ch != '}') // end of one pack data is '}'.
    {
        printf("end of one pack data is not '}'.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    pTmp = ++p;
    index = 0;
    int myCountor = 0;
    std::string remoteIp;
    unsigned short remotePort = 0;
    while(*p)
    {
        char c = *p;
        if(c == ',') {
            break;
        }
        if (c == '_') {
            ++myCountor;
            if (ONE == myCountor) {
                char cIp[index + ONE]= {ZERO};
                memcpy(cIp, pTmp, index);
                remoteIp = std::string(cIp);
                index = 0;
                pTmp = ++p;
                continue;
            }
            else if (TWO == myCountor) {
                char cPort[index + ONE] = {ZERO};
                memcpy(cPort, pTmp, index);
                remotePort = (unsigned short)atoi(cPort);
                index = 0;
                pTmp = ++p;
                continue;
            }
            else {
                printf("protocol data error: ip:port.\n");
                p = checkNextPackageData(p);
                if (p == NULL) {
                    return false;
                } else {
                    parseDataSize = 0;
                    goto unpack;
                }
            }
        }
        if (myCountor >= ONE) {
            if(c > '9' || c < '0') {
                printf("protocol data error:group or port.\n");
                p = checkNextPackageData(p);
                if (p == NULL) {
                    return false;
                } else {
                    parseDataSize = 0;
                    goto unpack;
                }
            }
        }
        ++p;
        ++index;
    }
    if(0 >= index) {
        printf("protocol data error:group.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    char cGroup[index + 1] = {0};
    memcpy(cGroup, pTmp, index);
    int group = atoi(cGroup);
    pTmp = ++p;
    index = 0;
    while(*p)
    {
        char c = *p;
        if(c == ',') {
            break;
        }
        if(c > '9' || c < '0') {
            printf("protocol data error:sequence.\n");
            p = checkNextPackageData(p);
            if (p == NULL) {
                return false;
            } else {
                parseDataSize = 0;
                goto unpack;
            }
        }
        ++p;
        ++index;
    }
    if(0 >= index) {
        printf("protocol data error:sequence.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    char cSequence[index + 1] = {0};
    memcpy(cSequence, pTmp, index);
    int sequenceId = atoi(cSequence);
    pTmp = ++p;
    index = 0;
    while(*p)
    {
        char c = *p;
        if(c == ',') {
            break;
        }
        ++index;
        ++p;
    }
    if(0 >= index) {
        printf("protocol data error:code.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    char cCode[index + 1] = {0};
    memcpy(cCode, pTmp, index);
    int code = atoi(cCode);
    pTmp = ++p;
    index = 0;
    while(*p)
    {
        char c = *p;
        if(c == ',') {
            break;
        }
        if(c != 'u' && c != 'd' && c != 'p' && c != 't' && c != 'c') {
            printf("protocol data error:protocol.\n");
            p = checkNextPackageData(p);
            if (p == NULL) {
                return false;
            } else {
                parseDataSize = 0;
                goto unpack;
            }
        }
        ++index;
        ++p;
    }
    if(0 >= index) {
        printf("protocol data error:protocol.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    char cProtocol[index + 1] = {0};
    memcpy(cProtocol, pTmp, index);
    pTmp = ++p;
    index = 0;
    while(*p)
    {
        char c = *p;
        if(c == ',') {
            break;
        }
        if(c > '9' || c < '0') {
            printf("protocol data error:type.\n");
            p = checkNextPackageData(p);
            if (p == NULL) {
                return false;
            } else {
                parseDataSize = 0;
                goto unpack;
            }
        }
        ++p;
        ++index;
    }
    if(0 >= index) {
        printf("protocol data error:type.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    char cType[index + 1] = {0};
    memcpy(cType, pTmp, index);
    int type = atoi(cType);
    pTmp = ++p;
    index = 0;
    while(*p)
    {
        char c = *p;
        if(c == ',') {
            break;
        }
        if(c > '9' || c < '0') {
            printf("protocol data error:data_total_length.\n");
            p = checkNextPackageData(p);
            if (p == NULL) {
                return false;
            } else {
                parseDataSize = 0;
                goto unpack;
            }
        }
        ++p;
        ++index;
    }
    if(0 >= index) {
        printf("protocol data error:data_total_length.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    char cDataTotalLength[index + 1] = {0};
    memcpy(cDataTotalLength, pTmp, index);
    int dataTotalLength = atoi(cDataTotalLength);
    pTmp = ++p;
    index = 0;
    while(*p)
    {
        char c = *p;
        if(c == '{') {
            break;
        }
        if(c > '9' || c < '0') {
            printf("protocol data error:data_length.\n");
            p = checkNextPackageData(p);
            if (p == NULL) {
                return false;
            } else {
                parseDataSize = 0;
                goto unpack;
            }
        }
        ++p;
        ++index;
    }
    if(0 >= index) {
        printf("protocol data error:data_length.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }
    char cDataLength[index + 1] = {0};
    memcpy(cDataLength, pTmp, index);
    int dataLength = atoi(cDataLength);
    pTmp = ++p;
    p = p + dataLength; // end of one pack data is in the '}' position.
    ch = *p;
    if(ch != '}') {
        printf("protocol data error: end data of (p + dataLength) is not '}'.\n");
        p = checkNextPackageData(p);
        if (p == NULL) {
            return false;
        } else {
            parseDataSize = 0;
            goto unpack;
        }
    }

    if(group == ZERO) {
        if (!checkCode(code)) {
            printf("protocol data error: group is 0. code: %d\n", code);
            p = checkNextPackageData(p);
            if (p == NULL) {
                return false;
            } else {
                parseDataSize = 0;
                goto unpack;
            }
        }
    }

    ProtocolData * pData = new ProtocolData();
    pData->setLength(length);
    pData->setRemoteIp(remoteIp);
    pData->setRemotePort(remotePort);
    pData->setGroup(group);
    pData->setSequence(sequenceId);
    pData->setCode(code);
    pData->setProtocol(cProtocol);
    pData->setType(type);
    pData->setDataTotalLength(dataTotalLength);
    char aData[dataLength + 1] = {0};
    memcpy(aData, pTmp, dataLength);
    pData->setData(aData, dataLength);
    if(!handleProtocolData(pData, addressInfo, socketType)) {
        printf("handle protocol data error.\n");
    }
    if(*(++p)) {
        goto unpack;
    }
    return true;
}

bool HandleServerData::checkCode(const int code)
{
    if (code == SERVER_REQUEST_GROUP) {
        return true;
    }
    else if (code == SERVER_RECONNECTED_PEER) {
        return true;
    }
    else if (code == SERVER_CLOSE_PEER) {
        return true;
    }
    else if (code == SERVER_CLOSE_PEER_OVER) {
        return true;
    }
    else if (code == SERVER_CLOSE_PEER_OVER_ACK) {
    }


    else if (code == CLIENT_REQUEST_GROUP) {
        return true;
    }
    else if (code == CLIENT_RECONNECTED_PEER_ACK) {
        return true;
    }
    else if (code == CLIENT_CLOSE_PEER) {
        return true;
    }
    else if (code == CLIENT_CLOSE_PEER_OVER) {
        return true;
    }
    else if (code == CLIENT_CLOSE_PEER_OVER_ACK) {
    }
    return false;
}

char* HandleServerData::checkNextPackageData(char *p)
{
    while (*(++p)) {
        char c = *p;
        if (c == '}') {
            if (*(++p)) {
                return p;
            }
        }
    }
    return NULL;
}

bool HandleServerData::handleProtocolData(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    if(pData == NULL) {
        return false;
    }
    if (socketType <= SOCKET_MIN || socketType >= SOCKET_MAX) {
        delete pData;
        pData = NULL;
        return false;
    }
    if (socketType == UDP_CLIENT || socketType == TCP_CLIENT) {
        return mHandleClientData->handleProtocolData(pData, addressInfo, socketType);
    }
    const int code = pData->getCode();
    switch(code)
    {
        case SERVER_REQUEST_GROUP: {
            return requestGroup(pData, addressInfo, socketType);
        }
        case SERVER_REPLY_GROUP: {
            return replyGroup(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case SERVER_ACK_GROUP: {
            return ackGroup(pData, addressInfo, socketType);
        }
        case SERVER_GROUP_NOT_EXIST: {
            return groupNotExist(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case SERVER_REDUNDANT_GROUP: {
            return redundantGroup(pData, addressInfo, socketType);
        }
        case SERVER_SEQUENCE_DATA:
        case SERVER_REPLACE_WITH_SEQUENCE_DATA: {
            return sequenceData(pData, addressInfo, socketType);
        }
        case SERVER_REQUEST_SEQUENCE_DATA: {
            return requestSequenceData(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case SERVER_ACK_SEQUENCE: {
            return ackSequenceData(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case SERVER_END_SEQUENCE: {
            return endSequence(pData, addressInfo, socketType);
        }
        case SERVER_ASSEMBLE_COMPLETE: {
            return assembleComplete(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case SERVER_ACK_ASSEMBLE_COMPLETE: {
            return ackAssembleComplete(pData, addressInfo, socketType);
        }
        case SERVER_ACK_COMPLETED: {
            return ackCompleted(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case SERVER_RECONNECTED_PEER: {
            return reconnectedPeer(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case SERVER_CLOSE_PEER: {
            return closePeer(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case SERVER_CLOSE_PEER_OVER: {
            return closePeerOver(pData, const_cast<sockets::AddressInfo *>(addressInfo), socketType);
        }
        case SERVER_CLOSE_PEER_OVER_ACK: {
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

/* client ---> server */  // client send to server.
bool HandleServerData::requestGroup(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    // groupId starts at one, not zero. sequence is the same too in protocol data.
    // but sequence with 0 save in the mMapSequence, and without any data.
    int groupId = ONE;
    std::string remoteIp(const_cast<sockets::AddressInfo *>(addressInfo)->getRemoteIPAddress());
    unsigned short remotePort(const_cast<sockets::AddressInfo *>(addressInfo)->getRemotePort());
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    mMutexGroupServer.lock();
    bool flag = false;
    if(!mMapGroupServer.empty()) {
        size_t size = mMapGroupServer.size();
        for (int i=ONE; i <= size; ++i) {
            if (i >= INT_MAX) {
                i = ONE;
            }
            memset(buf, ZERO, sizeof(buf));
            snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, i);
            std::string tmpGroupSId(buf);
            std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupServer.find(tmpGroupSId);
            if (it == mMapGroupServer.end()) {
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
        ret = mMapGroupServer.insert(std::pair<std::string, ProtocolGroup *>(keyGroupSId, pGroup));
        flag = ret.second;
        if (!flag) {
            ++groupId;
            memset(buf, 0, TWO_HUNDRED_AND_FIFTY_SIX);
            snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
            keyGroupSId = std::string(buf);
        }
    } while(!flag);
    mMutexGroupServer.unlock();

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
            CLIENT_REPLY_GROUP,
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

/* server ---> client */
bool HandleServerData::replyGroup(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    int groupId = pData->getGroup();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%d,0{}",
            remoteIp.c_str(),
            remotePort,
            groupId,
            pData->getSequence(),
            CLIENT_ACK_GROUP,
            pData->getProtocol().c_str(),
            pData->getType(),
            pData->getDataTotalLength());
    size_t dataSize = strlen(buf);
    mResponse->sendToOtherParty(buf, dataSize, addressInfo, socketType);
    delete pData;
    pData = NULL;
    // 这里通知上层发送数据.
    mProtocolService->notifyRequestGroupIdSuccess(groupSId, addressInfo, socketType);
    return true;
}

/* client ---> server */
bool HandleServerData::ackGroup(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    int groupId = pData->getGroup();
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    mMutexGroupServer.lock();
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupServer.find(groupSId);
    if (it != mMapGroupServer.end()) {
        ProtocolGroup * pGroup = it->second;
        pGroup->setAckGroup(true);
        mMutexGroupServer.unlock();
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
    = mMapGroupServer.insert(std::pair<std::string, ProtocolGroup *>(groupSId, pGroup));
    mMutexGroupServer.unlock();
    if (!ret.second) {
        delete pGroup;
        pGroup = NULL;
    }
    return ret.second;
}

/* server ---> client */
bool HandleServerData::groupNotExist(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    // 再次向server端请求groupId ?
    delete pData;
    pData = NULL;
    return true;
}

/* client ---> server */
bool HandleServerData::redundantGroup(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    delete pData;
    pData = NULL;
    return true;
}

/* client ---> server */
bool HandleServerData::sequenceData(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
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
    mMutexGroupServer.lock();
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupServer.find(groupSId);
    ProtocolGroup * pGroup = NULL;
    if(it == mMapGroupServer.end()) {
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
            mMutexGroupServer.unlock();
            return false;
        }
        std::pair<std::map<std::string, ProtocolGroup *>::iterator, bool> ret
        = mMapGroupServer.insert(std::pair<std::string, ProtocolGroup *>(groupSId, pGroup));
        if (!ret.second) {
            delete pGroup;
            pGroup = ret.first->second;
        }
    } else {
        pGroup = it->second;
    }
    mMutexGroupServer.unlock();
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
            CLIENT_ACK_SEQUENCE,
            pData->getProtocol().c_str(),
            type,
            dataTotalLength);
    size_t size = strlen(buf);
    return mResponse->sendToOtherParty(buf, size, addressInfo, socketType);
}

/* server ---> client */
bool HandleServerData::requestSequenceData(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
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

/* server ---> client */
bool HandleServerData::ackSequenceData(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
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

/* client ---> server */
bool HandleServerData::endSequence(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
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
    mMutexGroupServer.lock();
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupServer.find(groupSId);
    if(it == mMapGroupServer.end()) {
        mMutexGroupServer.unlock();
        memset(buf, ZERO, sizeof(buf));
        snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%lu,0{}",
                remoteIp.c_str(),
                remotePort,
                groupId,
                maxSequenceId,
                CLIENT_GROUP_NOT_EXIST,
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
        mMutexGroupServer.unlock();
        memset(buf, ZERO, sizeof(buf));
        snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%lu,0{}",
                remoteIp.c_str(),
                remotePort,
                groupId,
                maxSequenceId,
                CLIENT_ASSEMBLE_COMPLETE,
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
        mMutexGroupServer.unlock();
        delete pData;
        pData = NULL;printf("%s:%d\n", __FUNCTION__, __LINE__);
        return false;
    }
    pGroup->setAssemble(true);
    mMutexGroupServer.unlock();
    ProtocolSequence * pSequence = new ProtocolSequence();
    pSequence->setSequence(maxSequenceId);
    pSequence->setProtocolData(pData);
    pSequence->setIsMaxSequence(true);
    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d,%d", groupId, maxSequenceId);
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
                CLIENT_REQUEST_SEQUENCE_DATA,
                cProtocol,
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
            memcpy(data, tData->getData(), tDataLength);
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
        memcpy(data + dataLength, tData->getData(), tDataLength);
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
        pGroup->setAssemble(false);printf("%s:%d\n", __FUNCTION__, __LINE__);
        return false;
    }
    if(dataLength != dataTotalLength) { // data length error.
        if (data != NULL) {
            free(data);
            data = NULL;
            dataLength = ZERO;
        }
        pGroup->setAssemble(false);printf("%s:%d\n", __FUNCTION__, __LINE__);
        return false;
    }
    pGroup->setAssembleComplete(true);
    pGroup->setAssemble(false);

    memset(buf, sizeof(buf), sizeof(buf));
    snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%lu,0{}",
            remoteIp.c_str(),
            remotePort,
            groupId,
            maxSequenceId,
            CLIENT_ASSEMBLE_COMPLETE,
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
    dataLength = ZERO;printf("%s:%d assemble Complete\n", __FUNCTION__, __LINE__);
    return ret;
}

/* server ---> client */
bool HandleServerData::assembleComplete(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
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
            CLIENT_ACK_ASSEMBLE_COMPLETE,
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

/* client ---> server */
bool HandleServerData::ackAssembleComplete(ProtocolData * pData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    std::string remoteIp(pData->getRemoteIp());
    unsigned short remotePort(pData->getRemotePort());
    const int groupId = pData->getGroup();
    char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(buf, sizeof(buf), "%s_%d_%d", remoteIp.c_str(), remotePort, groupId);
    std::string groupSId(buf);
    mMutexGroupServer.lock();
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupServer.find(groupSId);
    if(it != mMapGroupServer.end()) {
        ProtocolGroup * pGroup = it->second;
        mMapGroupServer.erase(it);
        delete pGroup;
        pGroup = NULL;
    }
    mMutexGroupServer.unlock();
    memset(buf, ZERO, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s_%d_%d,%d,%d,%s,%d,%d,0{}",
            remoteIp.c_str(),
            remotePort,
            groupId,
            pData->getSequence(),
            CLIENT_ACK_COMPLETED,
            pData->getProtocol().c_str(),
            pData->getType(),
            pData->getDataTotalLength());
    size_t dataSize = strlen(buf);
    bool ret = mResponse->sendToOtherParty(buf, dataSize, addressInfo, socketType);
    delete pData;
    pData = NULL;
    return ret;
}

/* server ---> client */
bool HandleServerData::ackCompleted(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
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

/* client ---> server */
bool HandleServerData::reconnectedPeer(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
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
    while (NULL == (message = new Message(SERVER_RECONNECTED_PEER)));
    std::vector<void *> * vec = message->getVector();
    vec->push_back(remoteIpPtr);
    vec->push_back(remotePortPtr);
    vec->push_back(groupIdPtr);
    vec->push_back(address);
    vec->push_back(socketTypePtr);
    delete pData;
    pData = NULL;
    return mProtocolService->reconnectedPeer(message);
}

/* client ---> server */
bool HandleServerData::closePeer(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
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
    while (NULL == (message = new Message(SERVER_CLOSE_PEER)));
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

/* client ---> server */
bool HandleServerData::closePeerOver(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
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
    while (NULL == (message = new Message(SERVER_CLOSE_PEER_OVER)));
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

/* client ---> server */
bool HandleServerData::closePeerOverAck(ProtocolData *pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
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
    while (NULL == (message = new Message(SERVER_CLOSE_PEER_OVER_ACK)));
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

void HandleServerData::releaseMapGroupServer()
{
    if (!mMutexGroupServer.try_lock()) {
        return;
    }
    std::map<std::string, ProtocolGroup *>::iterator it = mMapGroupServer.begin();
    for (; it != mMapGroupServer.end(); ) {
        ProtocolGroup * pGroup = it->second;
        if (pGroup->getAssemble()) { // because endSequence() function is assembling data.
            continue;
        }
        if (pGroup->isTimeout(SIXTY)) {
            it = mMapGroupServer.erase(it);
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
    mMutexGroupServer.unlock();
}

};  // namespace protocols
