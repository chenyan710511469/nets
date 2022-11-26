/*
 * =====================================================================================
 *
 *       Filename:  ResponseOtherPartyInfo.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/23/2019 07:32:54 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/ResponseOtherPartyInfo.h"
#include "execinfo.h"


namespace protocols
{
ResponseOtherPartyInfo::ResponseOtherPartyInfo()
{}

ResponseOtherPartyInfo::~ResponseOtherPartyInfo()
{}

bool ResponseOtherPartyInfo::sendToOtherParty(char * data, size_t dataSize, const sockets::AddressInfo * addressInfo,
        const SocketType & socketType)
{
    if (NULL == data) {
        printf("%s:%d data is NULL.", __FUNCTION__, __LINE__);
        return false;
    }
    if (ZERO >= dataSize) {
        printf("%s:%d dataSize is less than or equal 0.", __FUNCTION__, __LINE__);
        return false;
    }
    // std::cout << __FUNCTION__ << ":" <<  __LINE__ << ", thread id: " << std::this_thread::get_id() << std::endl;
    bool ret = false;
    char * pTmpData = data;
    size_t tmpDataSize = dataSize;
    addLength(&pTmpData, &tmpDataSize);
    printf("remotePort:%d after adding length:\n%ld:%s\n",
    const_cast<sockets::AddressInfo*>(addressInfo)->getRemotePort(), tmpDataSize, pTmpData);
    switch (socketType) {
        case UDP_SERVER: {
            sockets::AddressInfo * address = getAddressInfo(addressInfo, socketType);
            if (NULL == address) {
                free(pTmpData);
                printf("%s:%d, address is NULL.", __FUNCTION__, __LINE__);
                return ret;
            }
            udpserver::UdpMessage message(0);
            while (!message.setData(pTmpData, tmpDataSize)) {
                std::cout << __FUNCTION__ << ":" <<  __LINE__ << ", thread id: " << std::this_thread::get_id() << std::endl;
            }
            if (!(ret = mUdpService->sendMsg(message, *address))) {
                printf("error: send data to udp client failed.\n");
            }
            delete address;
            break;
        }
        case UDP_CLIENT: {
            sockets::AddressInfo * address = getAddressInfo(addressInfo, socketType);
            if (NULL == address) {
                free(pTmpData);
                printf("%s:%d, address is NULL.", __FUNCTION__, __LINE__);
                return ret;
            }
            udpclient::UdpMessage message(0);
            while (!message.setData(pTmpData, tmpDataSize)) {
                std::cout << __FUNCTION__ << ":" <<  __LINE__ << ", thread id: " << std::this_thread::get_id() << std::endl;
            }
            if (!(ret = mUdpClientService->sendMsg(message, *address))) {
                printf("error: send data to udp server failed.\n");
            }
            delete address;
            break;
        }
        case TCP_SERVER: {
            int socketFd = (const_cast<sockets::AddressInfo *>(addressInfo))->getClientFd();
            tcpserver::TcpMessage message(socketFd);
            while (!message.setData(pTmpData, tmpDataSize)) {
                std::cout << __FUNCTION__ << ":" <<  __LINE__ << ", thread id: " << std::this_thread::get_id() << std::endl;
            }
            if (!(ret = mTcpService->sendMsg(message, const_cast<sockets::AddressInfo *>(addressInfo)))) {
                printf("error: send data to tcp client failed.\n");
            }
            break;
        }
        case TCP_CLIENT: {
            int socketFd = (const_cast<sockets::AddressInfo *>(addressInfo))->getClientFd();
            tcpclient::TcpMessage message(socketFd);
            while (!message.setData(pTmpData, tmpDataSize)) {
                std::cout << __FUNCTION__ << ":" <<  __LINE__ << ", thread id: " << std::this_thread::get_id() << std::endl;
            }
            if (!(ret = mTcpClientService->sendMsg(message, const_cast<sockets::AddressInfo *>(addressInfo)))) {
                printf("error: send data to tcp server failed.\n");
            }
            break;
        }
        default:
            break;
    }
    free(pTmpData);
    pTmpData = NULL;
    return ret;
}

sockets::AddressInfo * ResponseOtherPartyInfo::getAddressInfo(const sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    sockets::AddressInfo * address = NULL;
    int counter = 0;
repeat:
    try {
        switch(socketType) {
            case UDP_SERVER: {
                address = new udpserver::UdpClientInfo((udpserver::UdpClientInfo *)addressInfo);
                break;
            }
            case UDP_CLIENT: {
                address = new udpclient::UdpServerInfo((udpclient::UdpServerInfo *)addressInfo);
                break;
            }
            case TCP_SERVER: {
                address = new tcpserver::TcpClientInfo((tcpserver::TcpClientInfo *)addressInfo);
                break;
            }
            case TCP_CLIENT: {
                address = new tcpclient::TcpServerInfo((tcpclient::TcpServerInfo *)addressInfo);
                break;
            }
            default: {
                printf("%s:%d socketType is error:%d\n", __FUNCTION__, __LINE__, static_cast<int>(socketType));
                printStacktrace();
                break;
            }
        }
    }
    catch (std::exception & ex) {
        printf("error: %s\n", ex.what());
        if (NULL != address) {
            delete address;
            address = NULL;
        }
        if (TEN >= counter) {
            ++counter;
            goto repeat;
        }
    }
    return address;
}

void ResponseOtherPartyInfo::printStacktrace()
{
    const int size = ONE_HUNDRED_AND_TWENTY_EIGHT;
    void * array[size] = {ZERO};
    const int stackNum = backtrace(array, size);
    char ** stacktrace = backtrace_symbols(array, stackNum);
    if (NULL == stacktrace) {
        printf("%s:%d error.\n", __FUNCTION__, __LINE__);
    }
    for (int i = ZERO; i < stackNum; ++i) {
        printf("%s\n", stacktrace[i]);
    }
    free(stacktrace);
}

void ResponseOtherPartyInfo::setUdpService(udpserver::UdpIoService * udpService)
{
    mUdpService = udpService;
}

void ResponseOtherPartyInfo::setUdpClientService(udpclient::UdpIoService * udpClientService)
{
    mUdpClientService = udpClientService;
}

void ResponseOtherPartyInfo::setTcpService(tcpserver::TcpIoService * tcpService)
{
    mTcpService = tcpService;
}

void ResponseOtherPartyInfo::setTcpClientService(tcpclient::TcpIoService * tcpClientService)
{
    mTcpClientService = tcpClientService;
}

bool ResponseOtherPartyInfo::addLength(char ** buf, size_t *bufSize)
{
    if(NULL == buf || NULL == *buf) {
        return false;
    }
    if(0 >= *bufSize) {
        return false;
    }
    /* *****************addComma******************** */
    char * buff = NULL;
    while (NULL == (buff = (char *)malloc(*bufSize + TWO)));
    memset(buff, 0, *bufSize + TWO);
    // memcpy(buff, ",", 1);
    // memcpy(buff + 1, *buf, *bufSize);
    snprintf(buff, *bufSize + TWO, ",%s", *buf);
    *buf = buff;
    *bufSize = *bufSize + ONE;

    /* ****************addLength******************** */
    char cLength[TWENTY] = {ZERO};
    sprintf(cLength, "%ld", *bufSize);
    int x = strlen(cLength);
    int y = *bufSize + x;
    memset(cLength, 0, sizeof(cLength));
    sprintf(cLength, "%d", y);
    int z = strlen(cLength);
    int length = z + *bufSize;
    memset(cLength, 0, sizeof(cLength));
    sprintf(cLength, "%d", length);
    char * buffer = NULL;
    while (NULL == (buffer = (char *)malloc(length + ONE)));
    memset(buffer, 0, length + ONE);
    memcpy(buffer, cLength, strlen(cLength));
    memcpy(buffer + strlen(cLength), *buf, *bufSize);
    *bufSize = length;
    *buf = buffer;
    free(buff);
    buff = NULL;
    return true;
}

bool ResponseOtherPartyInfo::sendProtocolData(ProtocolData * pData, sockets::AddressInfo * addressInfo, const SocketType & socketType)
{
    bool ret = false;
    if(NULL == pData) {
        return ret;
    }
    char *data = pData->getData();
    if (NULL == data) {
        return ret;
    }
    size_t length = pData->getLength();
    char * pTmpData = NULL;
    while (NULL == (pTmpData = (char *)malloc(pData->getLength() + ONE)));
    memset(pTmpData, 0, length + ONE);
    int code = MINUS_ONE;
    if (socketType == SocketType::UDP_SERVER || socketType == SocketType::TCP_SERVER) {
        code = CLIENT_REPLACE_WITH_SEQUENCE_DATA;  // send to client parse.
    }
    else {
        code = SERVER_REPLACE_WITH_SEQUENCE_DATA;  // send to server parse.
    }
    snprintf(pTmpData, length, "%d,%d,%d,%d,%s,%d,%d,%d{%s}",
            pData->getLength(),
            pData->getGroup(),
            pData->getSequence(),
            code,
            pData->getProtocol().c_str(),
            pData->getType(),
            pData->getDataTotalLength(),
            pData->getDataLength(),
            pData->getData()
        );
    sockets::AddressInfo * address = getAddressInfo(addressInfo, socketType);
    if (NULL == address) {
        free(pTmpData);
        pTmpData = NULL;
        return ret;
    }
    switch(socketType) {
        case UDP_SERVER: {
            if (NULL == mUdpService) {
                break;
            }
            udpserver::UdpMessage message(0);
            message.setData(pTmpData, length);
            if (!(ret = mUdpService->sendMsg(message, *address))) {
                printf("error: send data to udp client failed.\n");
            }
        }
        case UDP_CLIENT: {
            if (NULL == mUdpClientService) {
                break;
            }
            udpclient::UdpMessage message(0);
            message.setData(pTmpData, length);
            if (!(ret = mUdpClientService->sendMsg(message, *address))) {
                printf("error: send data to udp server failed.\n");
            }
        }
        case TCP_SERVER: {
            if (NULL == mTcpService) {
                break;
            }
            tcpserver::TcpMessage message(addressInfo->getServerFd());
            message.setData(pTmpData, length);
            if (!(ret = mTcpService->sendMsg(message, addressInfo))) {
                printf("error: send data to tcp client failed.\n");
            }
        }
        case TCP_CLIENT: {
            if (NULL == mTcpClientService) {
                break;
            }
            tcpclient::TcpMessage message(addressInfo->getClientFd());
            message.setData(pTmpData, length);
            if (!(ret = mTcpClientService->sendMsg(message, addressInfo))) {
                printf("error: send data to tcp server failed.\n");
            }
        }
        default:
            break;
    }
    free(pTmpData);
    pTmpData = NULL;
    delete address;
    address = NULL;
    return ret;
}

};  // protocols
