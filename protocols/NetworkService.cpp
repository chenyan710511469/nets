/*
 * =====================================================================================
 *
 *       Filename:  NetworkService.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/05/2019 07:27:00 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/NetworkService.h"

#include "tinyxml2/tinyxml2.h"
#include "json/json.h"
#include "protocol/ResponseOtherPartyInfo.h"

namespace protocols
{
NetworkService::NetworkService(NetworkManager * netManager)
 : mNetManager(netManager)
 , mReceiver(NULL)
 , mResponse(NULL)
 , mIsRunning(false)
 , mCounter(ZERO)
{}

NetworkService::~NetworkService()
{
    stop();
    mReceiver = NULL;
    mResponse = NULL;
}

bool NetworkService::preStart(ReceiverBase * receiver)
{
    mReceiver = receiver;
    bool ret = (NULL != mReceiver);
    if (ret && !mIsRunning) {
        mIsRunning = true;
        mVecThread.push_back(new std::thread(&NetworkService::run, this));
        mVecThread.push_back(new std::thread(&NetworkService::run, this));
    }
    return ret;
}

void NetworkService::setResponse(ResponseOtherPartyInfo * response)
{
    mResponse = response;
}

ReceiverBase * NetworkService::getReceiver()
{
    return mReceiver;
}

bool NetworkService::receive(StorageData * receivedData, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    if (NULL == mReceiver) {
        return false;
    }
    if (NULL == mResponse) {
        return false;
    }
    std::string key((const_cast<sockets::AddressInfo *>(addressInfo))->getRemoteIPAddress());
    key.append(":");
    key.append(std::to_string((const_cast<sockets::AddressInfo *>(addressInfo))->getRemotePort()));
    Sender * sender = getSenderByAddressInfo(key);
    if (NULL == sender) {
        sockets::AddressInfo * tmpAddressInfo = getAddressInfo(addressInfo, socketType);
        if (NULL == tmpAddressInfo) {
            return false;
        }
        sender = new Sender(socketType, tmpAddressInfo, this);
        mMutexMap.lock();
        std::pair<std::map<std::string, Sender *>::iterator, bool> ret
        = mMapSender.insert(std::pair<std::string, Sender *>(key, sender));
        if (!ret.second) {
            delete sender;
            sender = ret.first->second;
        }
        mMutexMap.unlock();
    }
    const char * data = receivedData->getData();
    size_t dataLength = receivedData->getDataLength();
    const int typeData = receivedData->getDataType();
    if (typeData == TYPE_JSON) {
        Json::Reader reader;
        Json::Value value;
        if (!reader.parse(data, value)) {
            // treat data as a string, then deliver to upper application.
            // 解析成json数据失败, 把data当作字符串传给上层处理.
            printf("parse JSON data failed.\n");
        }
        else {
            try {
                mReceiver->receive(&value, sender);
                return true;
            } catch (std::exception & ex) {
                printf("%s:%d %s.\n", __FUNCTION__, __LINE__, ex.what());
            } catch (...) {
                printf("%s:%d unkown error.\n", __FUNCTION__, __LINE__);
            }
        }
    }
    else if (typeData == TYPE_XML) {
        tinyxml2::XMLDocument * xmlDoc = new tinyxml2::XMLDocument(true, tinyxml2::COLLAPSE_WHITESPACE);
        if (tinyxml2::XML_SUCCESS != xmlDoc->Parse(data, dataLength)) {
            // treat data as a string, then deliver to upper application.
            // 解析成xml数据失败, 把data当作字符串传给上层处理.
            printf("parse XML data failed.\n");
            delete xmlDoc;
            xmlDoc = NULL;
        }
        else {
            try {
                mReceiver->receive(xmlDoc, sender);
                delete xmlDoc;
                xmlDoc = NULL;
                return true;
            } catch (std::exception & ex) {
                printf("%s:%d %s.\n", __FUNCTION__, __LINE__, ex.what());
            } catch (...) {
                printf("%s:%d unkown error.\n", __FUNCTION__, __LINE__);
            }
        }
    } else {
        try {
            mReceiver->receive(const_cast<char *>(data), dataLength, sender);
            return true;
        } catch (std::exception & ex) {
            printf("%s:%d %s.\n", __FUNCTION__, __LINE__, ex.what());
        } catch (...) {
            printf("%s:%d unkown error.\n", __FUNCTION__, __LINE__);
        }
    }
    return false;
}

int NetworkService::getConfigGroupId(unsigned short localPort, SocketType & socketType)
{
    return mNetManager->getConfigGroupId(localPort, socketType);
}

sockets::AddressInfo * NetworkService::getAddressInfo(const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    return mResponse->getAddressInfo(addressInfo, socketType);
}

void NetworkService::check()
{
    printf("%s:%s:%d.\n", __FILE__, __FUNCTION__, __LINE__);
    if (!mMutexRelease.try_lock()) {
        return;
    }printf("%s:%s:%d.\n", __FILE__, __FUNCTION__, __LINE__);
    if (ZERO == (++mCounter % TEN)) {
        std::vector<Sender *>::iterator itVec = mReleaseSender.begin();
        for ( ; itVec != mReleaseSender.end(); ) {
            Sender * sender = *itVec;
            if (!sender->isTimeout(SIXTY * ONE_THOUSAND)) {
                continue;
            }
            itVec = mReleaseSender.erase(itVec);
            delete sender;
        }
    }
    if (mCounter >= ONE_MILLION) {
        mCounter = ZERO;
    }printf("%s:%s:%d.\n", __FILE__, __FUNCTION__, __LINE__);
    std::map<std::string, Sender *>::iterator itMap;
    const size_t mapSenderSize = mMapSender.size();
    for (itMap = mMapSender.begin(); itMap != mMapSender.end(); ) {
        Sender * sender = itMap->second;
        std::string key(itMap->first);printf("%s:%s:%d.\n", __FILE__, __FUNCTION__, __LINE__);
        if (NULL == sender) {
            mMutexMap.lock();
            if (mMapSender.find(key) == mMapSender.end() || mapSenderSize != mMapSender.size()) {
                mMutexMap.unlock();
                break;
            }
            itMap = mMapSender.erase(itMap);
            mMutexMap.unlock();
            printf("%s:%d sender is NULL.\n", __FUNCTION__, __LINE__);
            continue;
        }printf("%s:%s:%d.\n", __FILE__, __FUNCTION__, __LINE__);
        if (sender->mIsRelease) {
            mMutexMap.lock();
            itMap = mMapSender.erase(itMap);
            mMutexMap.unlock();
            printf("%s:%d destroy Sender: %p\n", __FUNCTION__, __LINE__, sender);
            try
            {
                mReceiver->destroySender(sender);
            }
            catch (std::exception & ex) {
                printf("%s. %s:%d\n", ex.what(), __FUNCTION__, __LINE__);
            }
            catch (...) {
                printf("unkown error. %s:%d.\n", __FUNCTION__, __LINE__);
            }
            mReleaseSender.push_back(sender);
            continue;
        }
        if (!sender->resendEndSequenceData()) {
            if (!sender->requestGroupId()) {
                if (sender->isTimeout(SIXTY * ONE_THOUSAND)) {
                    mMutexMap.lock();
                    if (mMapSender.find(key) == mMapSender.end() || mapSenderSize != mMapSender.size()) {
                        mMutexMap.unlock();
                        break;
                    }
                    itMap = mMapSender.erase(itMap);
                    mMutexMap.unlock();
                    try {
                        printf("%s:%d destroy Sender: %p\n", __FUNCTION__, __LINE__, sender);
                        mReceiver->destroySender(sender);
                    }
                    catch (std::exception & ex) {
                        printf("%s. %s:%d\n", ex.what(), __FUNCTION__, __LINE__);
                    }
                    catch (...) {
                        printf("unkown error. %s:%d.\n", __FUNCTION__, __LINE__);
                    }
                    mReleaseSender.push_back(sender);
                    continue;
                }
            }
        }
        mMutexMap.lock();
        if (mMapSender.find(key) == mMapSender.end() || mapSenderSize != mMapSender.size()) {
            mMutexMap.unlock();
            break;
        }
        ++itMap;
        mMutexMap.unlock();
    }
    mMutexRelease.unlock();
    ProtocolService * protocolService = mNetManager->mProtocolService;
    if (NULL == protocolService) {
        printf("%s:%d mProtocolService is NULL.", __FUNCTION__, __LINE__);
        return;
    }
    protocolService->releaseMapGroup();
}

void NetworkService::run()
{
    while (mIsRunning)
    {std::cout << "thread id: " << std::this_thread::get_id() << "  " << __FUNCTION__ << std::endl;
        MessageBase * message = NULL;
        if (mDequeMessage.empty()) {
            std::mutex mtx;
            std::unique_lock<std::mutex> lck(mtx);
            if (mCondition.wait_for(lck, std::chrono::milliseconds(3000)) == std::cv_status::timeout) {
                check();
            }
            if (!mIsRunning) {
                break;
            }
        }
        mMutexDeque.lock();
        if (mDequeMessage.empty()) {
            mMutexDeque.unlock();
            check();
            continue;
        }
        std::deque<MessageBase *>::iterator it = mDequeMessage.begin();
        message = *it;
        mDequeMessage.erase(it);
        mMutexDeque.unlock();
        if (NULL == message) {
            check();
            continue;
        }
        handleMessage(message);
        check();
    }
}

void NetworkService::handleMessage(MessageBase * message)
{
    if (NULL == message) {
        return;
    }
    std::vector<void *> * vecObj = message->getVector();
    const int id = message->getId();
    switch (id) {
        case SERVER_REPLY_GROUP:
            serverReplyGroup(vecObj);
            break;
        case SERVER_SEQUENCE_DATA:
            serverSequenceData(vecObj);
            break;
        case SERVER_ACK_SEQUENCE:
            serverAckSequence(vecObj);
            break;
        case SERVER_ASSEMBLE_COMPLETE:
            serverAssembleComplete(vecObj);
            break;
        case SERVER_ACK_COMPLETED:
            serverAckCompleted(vecObj);
            break;
        case SERVER_RECONNECTED_PEER:
            serverReconnectedPeer(vecObj);
            break;
        case SERVER_CLOSE_PEER:
            serverClosePeer(vecObj);
            break;
        case SERVER_CLOSE_PEER_OVER:
            serverClosePeerOver(vecObj);
            break;
        case SERVER_CLOSE_PEER_OVER_ACK:
            serverClosePeerOverAck(vecObj);
            break;

        case CLIENT_REPLY_GROUP:
            clientReplyGroup(vecObj);
            break;
        case CLIENT_SEQUENCE_DATA:
            clientSequenceData(vecObj);
            break;
        case CLIENT_ACK_SEQUENCE:
            clientAckSequence(vecObj);
            break;
        case CLIENT_ASSEMBLE_COMPLETE:
            clientAssembleComplete(vecObj);
            break;
        case CLIENT_ACK_COMPLETED:
            clientAckCompleted(vecObj);
            break;
        case CLIENT_RECONNECTED_PEER_ACK:
            clientReconnectedPeerAck(vecObj);
            break;
        case CLIENT_CLOSE_PEER:
            clientClosePeer(vecObj);
            break;
        case CLIENT_CLOSE_PEER_OVER:
            clientClosePeerOver(vecObj);
            break;
        case CLIENT_CLOSE_PEER_OVER_ACK:
            clientClosePeerOverAck(vecObj);
            break;

        case REQUEST_GROUP_ID:
            handleRequestSend(vecObj);
            break;
    }
    delete message;
    message = NULL;
}

bool NetworkService::stop()
{
    if (!mIsRunning) {
        return false;
    }
    mIsRunning = false;
    std::vector<std::thread *>::iterator it = mVecThread.begin();
    for (; it != mVecThread.end(); ) {
        std::thread * tmpThread = *it;
        mCondition.notify_all();
        tmpThread->join();
        it = mVecThread.erase(it);
        delete tmpThread;
        tmpThread = NULL;
        printf("thread is destroyed in %s.\n", __FILE__);
    }
    mMutexDeque.lock();
    std::deque<MessageBase *>::iterator itMsg = mDequeMessage.begin();
    for (; itMsg != mDequeMessage.end(); ) {
        MessageBase * message = *itMsg;
        itMsg = mDequeMessage.erase(itMsg);
        std::vector<void *> * vec = message->getVector();
        std::vector<void *>::iterator itVec = vec->begin();
        for (; itVec != vec->end(); ) {
            delete ((char *)*itVec);
            vec->erase(itVec);
            itVec = vec->begin();
        }
        delete message;
    }
    mMutexDeque.unlock();
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator itMap = mMapSender.begin();
    for (; itMap != mMapSender.end(); ) {
        delete itMap->second;
        itMap = mMapSender.erase(itMap);
    }
    mMutexMap.unlock();
    return true;
}

SenderBase * NetworkService::createSenderForTcpClient(int configGroupId, std::string & remoteIp, unsigned short remotePort)
{
    try
    {
        if (!mNetManager->mTcpClientState) {
            throw std::runtime_error("please call startTcpClient() successfully.\n");
        }
        if (ZERO >= configGroupId) {
            throw("parameter configGroupId must be more than 0.\n");
        }
        if (remoteIp.empty()) {
            throw std::runtime_error("parameter remoteIp can't be empty.\n");
        }
        if (ZERO >= remotePort) {
            throw std::runtime_error("parameter remotePort must be more than 0.\n");
        }
    }
    catch (std::exception & ex) {
        printf("%s", ex.what());
        throw ex;
    }
    SenderBase * sender = NULL;
    std::string key(remoteIp);
    key.append(":");
    key.append(std::to_string(remotePort));
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it != mMapSender.end()) {
        sender = it->second;
    }
    mMutexMap.unlock();
    if (NULL != sender) {
        return sender;
    }
    _tcpclient * tcpclient = mNetManager->mConfiguration->getTcpclient();
    std::map<unsigned short, _ip *> * ipMap = tcpclient->getIpMap();
    std::map<unsigned short, _ip *>::iterator itIp = ipMap->begin();
    unsigned short localPort = ZERO;
    std::string localIpAddress;
    std::string ipVersion;
    bool isStarted = false;
    _ip * ip = NULL;
    int index = 0;
    for (; itIp != ipMap->end(); ++itIp) {
        unsigned short tmpLocalPort = itIp->first;
        ip = itIp->second;
        if (configGroupId != ip->configGroupId) {
            continue;
        }
        if (ZERO != remoteIp.compare(ip->remoteIpAddress)) {
            continue;
        }
        std::vector<struct _ports>::iterator itPort = ip->ports.begin();
        index = 0;
        for (; itPort != ip->ports.end(); ++itPort, ++index) {
            struct _ports ports = *itPort;
            if (remotePort != ports.remotePort) {
                continue;
            }
            if (tmpLocalPort != ports.localPort) {
                continue;
            }
            localPort = ports.localPort;
            isStarted = ports.started;
            localIpAddress = ip->localIpAddress;
            ipVersion = ip->ipVersion;
            break;
        }
        if (ZERO < localPort) {
            break;
        }
    }
    if (ZERO >= localPort) {
        try {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX,
                "don't find localPort by configGroupId, remoteIp and remotePort. Did you config is right in config file? %s.\n",
                mNetManager->mConfiguration->getXmlConfigPath().c_str());
            throw std::runtime_error(buf);
        }
        catch (std::exception & ex) {
            printf("%s", ex.what());
            throw ex;
        }
    }
    if (!isStarted) {
        tcpclient::TcpServerInfo address(remoteIp, remotePort);
        sockets::AddressInfo * addressBase = &address;
        addressBase->setLocalIPAddress(localIpAddress);
        addressBase->setLocalPort(localPort);
        addressBase->setIpVersion(ipVersion);
        isStarted = (NULL != mNetManager->tcpClientConnect(&address));
        if (isStarted) {
            ip->ports[index].started = isStarted;
        }
        else {
            try {
                char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
                snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX, "connect %s:%hu failed.\n", remoteIp.c_str(), remotePort);
                throw std::runtime_error(buf);
            }
            catch (std::exception & ex) {
                printf("%s", ex.what());
                throw ex;
            }
        }
    }
    ProtocolService * protocolService = mNetManager->mProtocolService;
    if (NULL == protocolService) {
        printf("%s:%d mProtocolService is NULL.", __FUNCTION__, __LINE__);
        return NULL;
    }
    sockets::AddressInfo * addressInfo = protocolService->getTcpClientAddressInfo(localPort);
    if (NULL == addressInfo) {
        try {
            printf("localPort: %d\n", localPort);
            throw std::runtime_error("don't find addressInfo.\n");
        }
        catch (std::exception & ex) {
            printf("%s", ex.what());
            throw ex;
        }
    }
    sockets::AddressInfo * tmpAddressInfo = getAddressInfo(addressInfo, TCP_CLIENT);
    if (NULL == tmpAddressInfo) {
        try {
            throw std::runtime_error("create AddressInfo failed.\n");
        }
        catch (std::exception & ex) {
            printf("%s", ex.what());
            throw ex;
        }
    }
    Sender * tmpSender = new Sender(TCP_CLIENT, tmpAddressInfo, this);
    mMutexMap.lock();
    std::pair<std::map<std::string, Sender *>::iterator, bool> ret
    = mMapSender.insert(std::pair<std::string, Sender *>(key, tmpSender));
    if (!ret.second) {
        delete tmpSender;
        tmpSender = ret.first->second;
    }
    mMutexMap.unlock();
    sender = tmpSender;
    return sender;
}

SenderBase * NetworkService::createSenderForUdpClient(int configGroupId, std::string & remoteIp, unsigned short remotePort)
{
    try
    {
        if (!mNetManager->mUdpClientState) {
            throw std::runtime_error("please call startUdpClient() successfully.\n");
        }
        if (ZERO >= configGroupId) {
            throw std::runtime_error("parameter configGroupId must be more than 0.\n");
        }
        if (remoteIp.empty()) {
            throw std::runtime_error("parameter remoteIp can't be empty.\n");
        }
        if (ZERO >= remotePort) {
            throw std::runtime_error("parameter remotePort must be more than 0.\n");
        }
    }
    catch (std::exception & ex) {
        printf("%s", ex.what());
        throw ex;
    }
    std::string key(remoteIp);
    key.append(":");
    key.append(std::to_string(remotePort));
    SenderBase * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    sender = it->second;
    mMutexMap.unlock();
    if (NULL != sender) {
        return sender;
    }
    _udpclient * udpclient = mNetManager->mConfiguration->getUdpclient();
    std::map<unsigned short, _ip *> * ipMap = udpclient->getIpMap();
    std::map<unsigned short, _ip *>::iterator itIp = ipMap->begin();
    unsigned short localPort = ZERO;
    std::string localIpAddress;
    bool isStarted = false;
    _ip * ip = NULL;
    int index = 0;
    for (; itIp != ipMap->end(); ++itIp) {
        unsigned short tmpLocalPort = itIp->first;
        ip = itIp->second;
        if (configGroupId != ip->configGroupId) {
            continue;
        }
        if (ZERO != remoteIp.compare(ip->remoteIpAddress)) {
            continue;
        }
        std::vector<struct _ports>::iterator itPort = ip->ports.begin();
        index = 0;
        for (; itPort != ip->ports.end(); ++itPort, ++index) {
            struct _ports ports = *itPort;
            if (remotePort != ports.remotePort) {
                continue;
            }
            if (tmpLocalPort != ports.localPort) {
                continue;
            }
            isStarted = ports.started;
            localIpAddress = ip->localIpAddress;
            localPort = tmpLocalPort;
            break;
        }
        if (ZERO < localPort) {
            break;
        }
    }
    if (ZERO >= localPort) {
        try {
            char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
            snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX,
                "don't find localPort by configGroupId, remoteIp and remotePort. Did you config is right in config file? %s.\n",
                mNetManager->mConfiguration->getXmlConfigPath().c_str());
            throw std::runtime_error(buf);
        }
        catch (std::exception & ex) {
            printf("%s", ex.what());
            throw ex;
        }
    }
    if (!isStarted) {
        isStarted = mNetManager->udpClientBind(remoteIp, remotePort, localPort, localIpAddress);
        if (isStarted) {
            ip->ports[index].started = isStarted;
        }
        else {
            try {
                char buf[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
                snprintf(buf, TWO_HUNDRED_AND_FIFTY_SIX, "bind %s:%hu failed.\n", localIpAddress.c_str(), localPort);
                throw std::runtime_error(buf);
            }
            catch (std::exception & ex) {
                printf("%s", ex.what());
                throw ex;
            }
        }
    }
    ProtocolService * protocolService = mNetManager->mProtocolService;
    if (NULL == protocolService) {
        printf("%s:%d mProtocolService is NULL.", __FUNCTION__, __LINE__);
        return NULL;
    }
    sockets::AddressInfo * addressInfo = protocolService->getUdpClientAddressInfo(localPort);
    if (NULL == addressInfo) {
        try {
            throw std::runtime_error("don't find addressInfo.\n");
        }
        catch (std::exception & ex) {
            printf("%s", ex.what());
            throw ex;
        }
    }
    sockets::AddressInfo * tmpAddressInfo = getAddressInfo(addressInfo, UDP_CLIENT);
    if (NULL == tmpAddressInfo) {
        try {
            throw std::runtime_error("create AddressInfo failed.\n");
        }
        catch (std::exception & ex) {
            printf("%s", ex.what());
            throw ex;
        }
    }
    Sender * tmpSender = new Sender(UDP_CLIENT, tmpAddressInfo, this);
    mMutexMap.lock();
    std::pair<std::map<std::string, Sender *>::iterator, bool> ret
    = mMapSender.insert(std::pair<std::string, Sender *>(key, tmpSender));
    if (!ret.second) {
        delete tmpSender;
        tmpSender = ret.first->second;
    }
    mMutexMap.unlock();
    sender = tmpSender;
    return sender;
}

Sender * NetworkService::getSenderByAddressInfo(std::string & key)
{
    Sender * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it != mMapSender.end()) {
        sender = it->second;
        sender->updateTime();
    }
    mMutexMap.unlock();
    return sender;
}

bool NetworkService::sendToOtherParty(char * data, size_t dataSize, const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    if (NULL == addressInfo) {
        printf("%s:%d addressInfo is NULL.\n", __FUNCTION__, __LINE__);
        return false;
    }
    return mResponse->sendToOtherParty(data, dataSize, addressInfo, socketType);
}

void NetworkService::notifyRequestGroupIdFailed(sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
}

void NetworkService::notifyRequestGroupIdSuccess(const std::string & groupSId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    MessageBase * message = NULL;
    if (socketType == SocketType::TCP_CLIENT || socketType == UDP_CLIENT) {
        message = new Message(CLIENT_REPLY_GROUP);
    }
    else if (socketType == SocketType::TCP_SERVER || socketType == UDP_SERVER) {
        message = new Message(SERVER_REPLY_GROUP);
    }
    else {
        printf("%s:%d socketType is error.\n", __FUNCTION__, __LINE__);
        return;
    }printf("%s:%d\n", __FUNCTION__, __LINE__);
    sockets::AddressInfo * addressInfoPtr = getAddressInfo(addressInfo, socketType);
    if (NULL == addressInfoPtr) {
        delete message;
        message = NULL;
        printf("%s:%d addressInfoPtr is NULL\n", __FUNCTION__, __LINE__);
        return;
    }
    std::string * groupSIdPtr = new std::string(groupSId);
    SocketType * socketTypePtr = new SocketType;
    *socketTypePtr = socketType;
    std::vector<void *> * vec = message->getVector();
    vec->push_back(groupSIdPtr);
    vec->push_back(addressInfoPtr);
    vec->push_back(socketTypePtr);
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
}

bool NetworkService::serverReplyGroup(std::vector<void *> * vecObj)
{
    std::string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(ONE);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(TWO);
    vecObj->clear();

    std::string remoteIp(addressInfoPtr->getRemoteIPAddress());
    unsigned short remotePort = addressInfoPtr->getRemotePort();
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIp.c_str(), remotePort);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorServerReplyGroup;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (NULL == sender) {
        printf("%s:%d sender is NULL.", __FUNCTION__, __LINE__);
        goto errorServerReplyGroup;
    }
    sender->updateTime();
    if (sender->packet(*groupSIdPtr)) {
        ret = sender->requestSend(*groupSIdPtr);
    } else {
        sender->resendEndSequenceData();
    }

errorServerReplyGroup:
    delete groupSIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return ret;
}

bool NetworkService::clientReplyGroup(std::vector<void *> * vecObj)
{
    std::string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(ONE);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(TWO);
    vecObj->clear();

    std::string remoteIp(addressInfoPtr->getRemoteIPAddress());
    unsigned short remotePort = addressInfoPtr->getRemotePort();
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIp.c_str(), remotePort);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        printf("%s:%d don't find Sender.\n", __FUNCTION__, __LINE__);
        goto errorClientReplyGroup;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (NULL == sender) {
        printf("%s:%d sender is NULL.", __FUNCTION__, __LINE__);
        goto errorClientReplyGroup;
    }
    sender->updateTime();
    if (sender->packet(*groupSIdPtr)) {
        ret = sender->requestSend(*groupSIdPtr);
    } else {
        sender->resendEndSequenceData();
    }

errorClientReplyGroup:
    delete groupSIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return ret;
}

bool NetworkService::requestSequenceData(std::string & groupSId, int sequenceId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    MessageBase * message = NULL;
    if (socketType == SocketType::TCP_CLIENT || socketType == SocketType::UDP_CLIENT) {
        message = new Message(CLIENT_SEQUENCE_DATA);
    }
    else if (socketType == SocketType::TCP_SERVER || socketType == SocketType::UDP_SERVER) {
        message = new Message(SERVER_SEQUENCE_DATA);
    }
    else {
        return false;
    }
    sockets::AddressInfo * addressInfoPtr = getAddressInfo(addressInfo, socketType);
    if (NULL == addressInfoPtr) {printf("%s:%d.\n", __FUNCTION__, __LINE__);
        delete message;
        message = NULL;
        return false;
    }
    std::string * groupSIdPtr = new std::string(groupSId);
    int * sequenceIdPtr = new int;
    *sequenceIdPtr = sequenceId;
    SocketType * socketTypePtr = new SocketType;
    *socketTypePtr = socketType;
    std::vector<void *> * vec = message->getVector();
    vec->push_back(groupSIdPtr);
    vec->push_back(sequenceIdPtr);
    vec->push_back(addressInfoPtr);
    vec->push_back(socketTypePtr);
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}

bool NetworkService::serverSequenceData(std::vector<void *> * vecObj)
{
    std::string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    int * sequenceIdPtr = (int *)vecObj->at(ONE);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(TWO);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(THREE);
    vecObj->clear();

    std::string remoteIp(addressInfoPtr->getRemoteIPAddress());
    unsigned short remotePort = addressInfoPtr->getRemotePort();
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIp.c_str(), remotePort);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorServerSequenceData;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (NULL == sender) {
        printf("%s:%d sender is NULL.", __FUNCTION__, __LINE__);
        goto errorServerSequenceData;
    }
    sender->updateTime();
    ret = sender->requestSend(*groupSIdPtr, *sequenceIdPtr);

errorServerSequenceData:
    delete groupSIdPtr;
    delete sequenceIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return ret;
}

bool NetworkService::clientSequenceData(std::vector<void *> * vecObj)
{
    std::string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    int * sequenceIdPtr = (int *)vecObj->at(ONE);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(TWO);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(THREE);
    vecObj->clear();

    std::string remoteIp(addressInfoPtr->getRemoteIPAddress());
    unsigned short remotePort = addressInfoPtr->getRemotePort();
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIp.c_str(), remotePort);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorClientSequenceData;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (NULL == sender) {
        printf("%s:%d sender is NULL.", __FUNCTION__, __LINE__);
        goto errorClientSequenceData;
    }
    sender->updateTime();
    ret = sender->requestSend(*groupSIdPtr, *sequenceIdPtr);

errorClientSequenceData:
    delete groupSIdPtr;
    delete sequenceIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return ret;
}


bool NetworkService::ackSequenceData(std::string & groupSId, int sequenceId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    MessageBase * message = NULL;
    if (socketType == SocketType::TCP_CLIENT || socketType == SocketType::UDP_CLIENT) {
        message = new Message(CLIENT_ACK_SEQUENCE);
    }
    else if (socketType == SocketType::TCP_SERVER || socketType == SocketType::UDP_SERVER) {
        message = new Message(SERVER_ACK_SEQUENCE);
    }
    else {
        return false;
    }
    sockets::AddressInfo * addressInfoPtr = getAddressInfo(addressInfo, socketType);
    if (NULL == addressInfoPtr) {
        delete message;
        message = NULL;
        return false;
    }
    std::string * groupSIdPtr = new std::string(groupSId);
    int * sequenceIdPtr = new int;
    *sequenceIdPtr = sequenceId;
    SocketType * socketTypePtr = new SocketType;
    *socketTypePtr = socketType;
    std::vector<void *> * vec = message->getVector();
    vec->push_back(groupSIdPtr);
    vec->push_back(sequenceIdPtr);
    vec->push_back(addressInfoPtr);
    vec->push_back(socketTypePtr);
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}

bool NetworkService::serverAckSequence(std::vector<void *> * vecObj)
{
    std::string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    int * sequenceIdPtr = (int *)vecObj->at(ONE);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(TWO);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(THREE);
    vecObj->clear();

    delete groupSIdPtr;
    delete sequenceIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return true;
}

bool NetworkService::clientAckSequence(std::vector<void *> * vecObj)
{
    std::string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    int * sequenceIdPtr = (int *)vecObj->at(ONE);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(TWO);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(THREE);
    vecObj->clear();

    delete groupSIdPtr;
    delete sequenceIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return true;
}

bool NetworkService::assembleComplete(std::string & groupSId, int maxSequenceId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    MessageBase * message = NULL;
    if (socketType == SocketType::TCP_CLIENT || socketType == UDP_CLIENT) {
        message = new Message(CLIENT_ASSEMBLE_COMPLETE);
    }
    else if (socketType == SocketType::TCP_SERVER || socketType == UDP_SERVER) {
        message = new Message(SERVER_ASSEMBLE_COMPLETE);
    }
    else {
        return false;
    }
    sockets::AddressInfo * addressInfoPtr = getAddressInfo(addressInfo, socketType);
    if (NULL == addressInfoPtr) {
        delete message;
        message = NULL;
        return false;
    }
    std::string * groupSIdPtr = new std::string(groupSId);
    int * maxSequenceIdPtr = new int;
    *maxSequenceIdPtr = maxSequenceId;
    SocketType * socketTypePtr = new SocketType;
    *socketTypePtr = socketType;
    std::vector<void *> * vec = message->getVector();
    vec->push_back(groupSIdPtr);
    vec->push_back(maxSequenceIdPtr);
    vec->push_back(addressInfoPtr);
    vec->push_back(socketTypePtr);
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}

bool NetworkService::serverAssembleComplete(std::vector<void *> * vecObj)
{
    std::string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    int * maxSequenceIdPtr = (int *)vecObj->at(ONE);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(TWO);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(THREE);
    vecObj->clear();

    std::string remoteIp(addressInfoPtr->getRemoteIPAddress());
    unsigned short remotePort = addressInfoPtr->getRemotePort();
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIp.c_str(), remotePort);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        printf("%s:%d not found sender.\n", __FUNCTION__, __LINE__);
        goto errorServerAssembleComplete;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (NULL == sender) {
        printf("%s:%d sender is NULL.", __FUNCTION__, __LINE__);
        goto errorServerAssembleComplete;
    }
    sender->updateTime();
    ret = sender->assembleComplete(*groupSIdPtr, *maxSequenceIdPtr, true);

errorServerAssembleComplete:
    delete groupSIdPtr;
    delete maxSequenceIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return ret;
}

bool NetworkService::clientAssembleComplete(std::vector<void *> * vecObj)
{
    std::string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    int * maxSequenceIdPtr = (int *)vecObj->at(ONE);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(TWO);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(THREE);
    vecObj->clear();

    std::string remoteIp(addressInfoPtr->getRemoteIPAddress());
    unsigned short remotePort = addressInfoPtr->getRemotePort();
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIp.c_str(), remotePort);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        printf("%s:%d not found sender.\n", __FUNCTION__, __LINE__);
        goto errorClientAssembleComplete;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (NULL == sender) {
        printf("%s:%d sender is NULL.", __FUNCTION__, __LINE__);
        goto errorClientAssembleComplete;
    }
    sender->updateTime();
    ret = sender->assembleComplete(*groupSIdPtr, *maxSequenceIdPtr, true);

errorClientAssembleComplete:
    delete groupSIdPtr;
    delete maxSequenceIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return ret;
}

bool NetworkService::ackCompleted(std::string & groupSId, int maxSequenceId,
        sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    MessageBase * message = NULL;
    if (socketType == SocketType::TCP_CLIENT || socketType == UDP_CLIENT) {
        message = new Message(CLIENT_ACK_COMPLETED);
    }
    else if (socketType == SocketType::TCP_SERVER || socketType == UDP_SERVER) {
        message = new Message(SERVER_ACK_COMPLETED);
    }
    else {
        return false;
    }
    sockets::AddressInfo * addressInfoPtr = getAddressInfo(addressInfo, socketType);
    if (NULL == addressInfoPtr) {
        delete message;
        message = NULL;
        return false;
    }
    std::string * groupSIdPtr = new std::string(groupSId);
    int * maxSequenceIdPtr = new int;
    *maxSequenceIdPtr = maxSequenceId;
    SocketType * socketTypePtr = new SocketType;
    *socketTypePtr = socketType;
    std::vector<void *> * vec = message->getVector();
    vec->push_back(groupSIdPtr);
    vec->push_back(maxSequenceIdPtr);
    vec->push_back(addressInfoPtr);
    vec->push_back(socketTypePtr);
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}

bool NetworkService::serverAckCompleted(std::vector<void *> * vecObj)
{
    std::string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    int * maxSequenceIdPtr = (int *)vecObj->at(ONE);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(TWO);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(THREE);
    vecObj->clear();

    std::string remoteIp(addressInfoPtr->getRemoteIPAddress());
    unsigned short remotePort = addressInfoPtr->getRemotePort();
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIp.c_str(), remotePort);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorServerAckCompleted;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (NULL == sender) {
        printf("%s:%d sender is NULL.", __FUNCTION__, __LINE__);
        goto errorServerAckCompleted;
    }
    sender->updateTime();
    ret = sender->ackCompleted(*groupSIdPtr, *maxSequenceIdPtr);

errorServerAckCompleted:
    delete groupSIdPtr;
    delete maxSequenceIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return ret;
}

bool NetworkService::clientAckCompleted(std::vector<void *> * vecObj)
{
    string * groupSIdPtr = (std::string *)vecObj->at(ZERO);
    int * maxSequenceIdPtr = (int *)vecObj->at(ONE);
    sockets::AddressInfo * addressInfoPtr = (sockets::AddressInfo *)vecObj->at(TWO);
    SocketType * socketTypePtr = (SocketType *)vecObj->at(THREE);
    vecObj->clear();

    std::string remoteIp(addressInfoPtr->getRemoteIPAddress());
    unsigned short remotePort = addressInfoPtr->getRemotePort();
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIp.c_str(), remotePort);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorClientAckCompleted;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (NULL == sender) {
        goto errorClientAckCompleted;
    }
    sender->updateTime();
    ret = sender->ackCompleted(*groupSIdPtr, *maxSequenceIdPtr);

errorClientAckCompleted:
    delete groupSIdPtr;
    delete maxSequenceIdPtr;
    delete addressInfoPtr;
    delete socketTypePtr;
    return ret;
}

/* client ---> server */
bool NetworkService::reconnectedPeer(MessageBase * message)
{
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}
bool NetworkService::serverReconnectedPeer(std::vector<void *> * vecObj)
{
    std::string *           remoteIpPtr     = (std::string *)vecObj->at(ZERO);
    unsigned short *        remotePortPtr   = (unsigned short *)vecObj->at(ONE);
    int *                   groupIdPtr      = (int *)vecObj->at(TWO);
    sockets::AddressInfo *  addressInfo     = (sockets::AddressInfo *)vecObj->at(THREE);
    SocketType *            socketTypePtr   = (SocketType *)vecObj->at(FOUR);

    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIpPtr->c_str(), *remotePortPtr);
    std::string key(cKey);
    bool ret = false;
    Sender * tmpSender = NULL;
    Sender * sender = NULL;
    std::pair<std::map<std::string, Sender *>::iterator, bool> retMap;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorServerReconnectedPeer;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (sender == NULL) {
        goto errorServerReconnectedPeer;
    }
    sender->updateTime();
    mMutexMap.lock();
    mMapSender.erase(key);
    mMutexMap.unlock();
    memset(cKey, ZERO, sizeof(cKey));
    snprintf(cKey, sizeof(cKey), "%s:%d", addressInfo->getRemoteIPAddress().c_str(), addressInfo->getRemotePort());
    key = std::string(cKey);
    mMutexMap.lock();
    retMap = mMapSender.insert(std::pair<std::string, Sender *>(key, sender));
    if (!retMap.second) {
        tmpSender = retMap.first->second;
        mMapSender.erase(key);
        delete tmpSender;
        tmpSender = NULL;
        mMapSender.insert(std::pair<std::string, Sender *>(key, sender));
    }
    mMutexMap.unlock();
    ret = sender->requestSend(addressInfo);

errorServerReconnectedPeer:
    delete remoteIpPtr;
    delete remotePortPtr;
    delete groupIdPtr;
    delete addressInfo;
    delete socketTypePtr;
    return ret;
}

/* server ---> client */
bool NetworkService::reconnectedPeerAck(MessageBase * message)
{
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}
bool NetworkService::clientReconnectedPeerAck(std::vector<void *> * vecObj)
{
    std::string *           remoteIpPtr     = (std::string *)vecObj->at(ZERO);
    unsigned short *        remotePortPtr   = (unsigned short *)vecObj->at(ONE);
    int *                   groupIdPtr      = (int *)vecObj->at(TWO);
    sockets::AddressInfo *  addressInfo     = (sockets::AddressInfo *)vecObj->at(THREE);
    SocketType *            socketTypePtr   = (SocketType *)vecObj->at(FOUR);

    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIpPtr->c_str(), *remotePortPtr);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    std::pair<std::map<std::string, Sender *>::iterator, bool> retMap;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorClientReconnectedPeerAck;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (sender == NULL) {
        goto errorClientReconnectedPeerAck;
    }

errorClientReconnectedPeerAck:
    delete remoteIpPtr;
    delete remotePortPtr;
    delete groupIdPtr;
    delete addressInfo;
    delete socketTypePtr;
    return ret;
}

/* server <---> client */
bool NetworkService::closePeer(MessageBase * message)
{
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}
bool NetworkService::serverClosePeer(std::vector<void *> * vecObj)
{
    std::string *           remoteIpPtr     = (std::string *)vecObj->at(ZERO);
    unsigned short *        remotePortPtr   = (unsigned short *)vecObj->at(ONE);
    int *                   groupIdPtr      = (int *)vecObj->at(TWO);
    sockets::AddressInfo *  addressInfo     = (sockets::AddressInfo *)vecObj->at(THREE);
    SocketType *            socketTypePtr   = (SocketType *)vecObj->at(FOUR);

    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIpPtr->c_str(), *remotePortPtr);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    std::pair<std::map<std::string, Sender *>::iterator, bool> retMap;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorServerClosePeer;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (sender == NULL) {
        goto errorServerClosePeer;
    }
    ret = sender->closeFromPeer();

errorServerClosePeer:
    delete remoteIpPtr;
    delete remotePortPtr;
    delete groupIdPtr;
    delete addressInfo;
    delete socketTypePtr;
    return ret;
}
bool NetworkService::clientClosePeer(std::vector<void *> * vecObj)
{
    std::string *           remoteIpPtr     = (std::string *)vecObj->at(ZERO);
    unsigned short *        remotePortPtr   = (unsigned short *)vecObj->at(ONE);
    int *                   groupIdPtr      = (int *)vecObj->at(TWO);
    sockets::AddressInfo *  addressInfo     = (sockets::AddressInfo *)vecObj->at(THREE);
    SocketType *            socketTypePtr   = (SocketType *)vecObj->at(FOUR);

    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIpPtr->c_str(), *remotePortPtr);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    std::pair<std::map<std::string, Sender *>::iterator, bool> retMap;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorClientClosePeer;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (sender == NULL) {
        goto errorClientClosePeer;
    }
    ret = sender->closeFromPeer();

errorClientClosePeer:
    delete remoteIpPtr;
    delete remotePortPtr;
    delete groupIdPtr;
    delete addressInfo;
    delete socketTypePtr;
    return ret;
}

/* server <---> client */
bool NetworkService::closePeerOver(MessageBase * message)
{
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}
bool NetworkService::serverClosePeerOver(std::vector<void *> * vecObj)
{
    std::string *           remoteIpPtr     = (std::string *)vecObj->at(ZERO);
    unsigned short *        remotePortPtr   = (unsigned short *)vecObj->at(ONE);
    int *                   groupIdPtr      = (int *)vecObj->at(TWO);
    sockets::AddressInfo *  addressInfo     = (sockets::AddressInfo *)vecObj->at(THREE);
    SocketType *            socketTypePtr   = (SocketType *)vecObj->at(FOUR);

    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIpPtr->c_str(), *remotePortPtr);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    std::pair<std::map<std::string, Sender *>::iterator, bool> retMap;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorServerClosePeerOver;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (sender == NULL) {
        goto errorServerClosePeerOver;
    }
    ret = sender->closeOverFromPeer();

errorServerClosePeerOver:
    delete remoteIpPtr;
    delete remotePortPtr;
    delete groupIdPtr;
    delete addressInfo;
    delete socketTypePtr;
    return ret;
}
bool NetworkService::clientClosePeerOver(std::vector<void *> * vecObj)
{
    std::string *           remoteIpPtr     = (std::string *)vecObj->at(ZERO);
    unsigned short *        remotePortPtr   = (unsigned short *)vecObj->at(ONE);
    int *                   groupIdPtr      = (int *)vecObj->at(TWO);
    sockets::AddressInfo *  addressInfo     = (sockets::AddressInfo *)vecObj->at(THREE);
    SocketType *            socketTypePtr   = (SocketType *)vecObj->at(FOUR);

    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIpPtr->c_str(), *remotePortPtr);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    std::pair<std::map<std::string, Sender *>::iterator, bool> retMap;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorClientClosePeerOver;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (sender == NULL) {
        goto errorClientClosePeerOver;
    }
    ret = sender->closeOverFromPeer();

errorClientClosePeerOver:
    delete remoteIpPtr;
    delete remotePortPtr;
    delete groupIdPtr;
    delete addressInfo;
    delete socketTypePtr;
    return ret;
}

/* server <---> client */
bool NetworkService::closePeerOverAck(MessageBase * message)
{
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}
bool NetworkService::serverClosePeerOverAck(std::vector<void *> * vecObj)
{
    std::string *           remoteIpPtr     = (std::string *)vecObj->at(ZERO);
    unsigned short *        remotePortPtr   = (unsigned short *)vecObj->at(ONE);
    int *                   groupIdPtr      = (int *)vecObj->at(TWO);
    sockets::AddressInfo *  addressInfo     = (sockets::AddressInfo *)vecObj->at(THREE);
    SocketType *            socketTypePtr   = (SocketType *)vecObj->at(FOUR);
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIpPtr->c_str(), *remotePortPtr);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    std::pair<std::map<std::string, Sender *>::iterator, bool> retMap;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorServerClosePeerOverAck;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (sender == NULL) {
        goto errorServerClosePeerOverAck;
    }
    // ret = sender->closeOverAckFromPeer();

errorServerClosePeerOverAck:
    delete remoteIpPtr;
    delete remotePortPtr;
    delete groupIdPtr;
    delete addressInfo;
    delete socketTypePtr;
    return true;
}
bool NetworkService::clientClosePeerOverAck(std::vector<void *> * vecObj)
{
    std::string *           remoteIpPtr     = (std::string *)vecObj->at(ZERO);
    unsigned short *        remotePortPtr   = (unsigned short *)vecObj->at(ONE);
    int *                   groupIdPtr      = (int *)vecObj->at(TWO);
    sockets::AddressInfo *  addressInfo     = (sockets::AddressInfo *)vecObj->at(THREE);
    SocketType *            socketTypePtr   = (SocketType *)vecObj->at(FOUR);
    char cKey[TWO_HUNDRED_AND_FIFTY_SIX] = {ZERO};
    snprintf(cKey, sizeof(cKey), "%s:%d", remoteIpPtr->c_str(), *remotePortPtr);
    std::string key(cKey);
    bool ret = false;
    Sender * sender = NULL;
    std::pair<std::map<std::string, Sender *>::iterator, bool> retMap;
    mMutexMap.lock();
    std::map<std::string, Sender *>::iterator it = mMapSender.find(key);
    if (it == mMapSender.end()) {
        mMutexMap.unlock();
        goto errorClientClosePeerOverAck;
    }
    sender = it->second;
    mMutexMap.unlock();
    if (sender == NULL) {
        goto errorClientClosePeerOverAck;
    }
    // ret = sender->closeOverAckFromPeer();

errorClientClosePeerOverAck:
    delete remoteIpPtr;
    delete remotePortPtr;
    delete groupIdPtr;
    delete addressInfo;
    delete socketTypePtr;
    return ret;
}

bool NetworkService::requestGroupId(Sender * sender)
{
    if (NULL == sender) {
        return false;
    }
    MessageBase * message = new Message(REQUEST_GROUP_ID);
    std::vector<void *> * vec = message->getVector();
    vec->push_back(sender);
    mMutexDeque.lock();
    mDequeMessage.push_back(message);
    mMutexDeque.unlock();
    mCondition.notify_all();
    return true;
}

bool NetworkService::handleRequestSend(std::vector<void *> * vecObj)
{
    Sender * sender = (Sender *)vecObj->at(ZERO);
    if (NULL == sender) {
        return false;
    }
    return sender->requestGroupId();
}

int NetworkService::getClientGroupId(const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    if (!(socketType == UDP_CLIENT || socketType == TCP_CLIENT)) {
        return MINUS_ONE;
    }
    unsigned short localPort = (const_cast<sockets::AddressInfo *>(addressInfo))->getLocalPort();
    return getConfigGroupId(localPort, const_cast<SocketType &>(socketType));
}

bool NetworkService::receiveTcpClientConnect(sockets::AddressInfo * addressInfo, bool isConnected)
{
    return false;
}

bool NetworkService::releaseTcpServer(sockets::AddressInfo * addressInfo)
{
printf("%s:%s:%d release request.\n", __FILE__, __FUNCTION__, __LINE__);
    std::map<std::string, Sender *>::iterator itMap;
    for (itMap = mMapSender.begin(); itMap != mMapSender.end(); ++itMap) {
        Sender * sender = itMap->second;
        if (sender == NULL) {
            continue;
        }
        sockets::AddressInfo * address = sender->getAddressInfo();
        if (address == NULL) {
            continue;
        }
        if (addressInfo->getRequest() == address->getRequest()) {
            // The address temporarily unavailable, when the bottom layer releases resources.
            address->setRequest(NULL);
            sender->mIsConnected = false;  // Not connected.
            sender->mIsRelease = true;     // will release the sender.
            printf("setRequest(NULL) remote address:%s:%d, local address:%s:%d.\n",
                    address->getRemoteIPAddress().c_str(), address->getRemotePort(),
                    address->getLocalIPAddress().c_str(), address->getLocalPort());
        }
    }
printf("%s:%s:%d release request.\n", __FILE__, __FUNCTION__, __LINE__);
    return true;
}

bool NetworkService::releaseTcpClient(sockets::AddressInfo * addressInfo)
{
    // Because tcp server, tcp client, udp server, and udp client are all handled the same way,
    // could just call releaseTcpServer directly.
    return releaseTcpServer(addressInfo);
}

bool NetworkService::releaseUdpServer(sockets::AddressInfo * addressInfo)
{
    // Because tcp server, tcp client, udp server, and udp client are all handled the same way,
    // could just call releaseTcpServer directly.
    return releaseTcpServer(addressInfo);
}

bool NetworkService::releaseUdpClient(sockets::AddressInfo * addressInfo)
{
    // Because tcp server, tcp client, udp server, and udp client are all handled the same way,
    // could just call releaseTcpServer directly.
    return releaseTcpServer(addressInfo);
}

sockets::AddressInfo * NetworkService::tcpClientReconnect(sockets::AddressInfo * addressInfo)
{
    return mNetManager->tcpClientConnect(addressInfo, true);
}

bool NetworkService::closeClientSocketFd(sockets::AddressInfo * addressInfo, SocketType & socketType)
{
    ProtocolService * protocolService = mNetManager->mProtocolService;
    if (NULL == protocolService) {
        printf("%s:%d mProtocolService is NULL.", __FUNCTION__, __LINE__);
        return false;
    }
    return protocolService->closeClientSocketFd(addressInfo, socketType);
}

void NetworkService::replyUpper(Sender * sender, bool result, void * upperPointer)
{
    if (NULL != mReceiver) {
        mReceiver->replyUpper(sender, result, upperPointer);
    }
}

};  // namespace protocols

