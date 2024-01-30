/*
 * =====================================================================================
 *
 *       Filename:  UdpIoRequest.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2019年01月20日 11时23分02秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "sockets/udp/server/UdpIoRequest.h"

namespace udpserver
{
UdpIoRequest::UdpIoRequest()
 : mSending(false)
{
    // 不能对Args执行memset()初始化,不然Args的std::string类型成员在赋值时会出现段错误.
    //memset(&Args, 0, sizeof(Args));
    Args.mCallbackRelease = NULL;
    Args.mOwner = NULL;
    memset(Args.Recvfrom.mBuffer, 0, sizeof(Args.Recvfrom.mBuffer));
    Args.Recvfrom.mBufferSize = 0;
    Args.Recvfrom.mReceivedBytes = 0;
    Args.Recvfrom.mFlags = 0;
    Args.Recvfrom.mCallbackRecv = NULL;
    Args.Recvfrom.mPort = 0;

    memset(Args.Sendto.mBuffer, 0, sizeof(Args.Recvfrom.mBuffer));
    Args.Sendto.mBufferSize = 0;
    Args.Sendto.mSentBytes = 0;
    Args.Sendto.mFlags = 0;
    Args.Sendto.mTimeout = 0;
    Args.Sendto.mPort = 0;

    Args.Bind.mSocket = NULL;
    pthread_mutex_init(&mSendMutex, NULL);
}

UdpIoRequest::~UdpIoRequest()
{
    Args.mCallbackRelease = NULL;
    Args.mOwner = NULL;
    Args.Recvfrom.mBufferSize = 0;
    Args.Recvfrom.mReceivedBytes = 0;
    Args.Recvfrom.mFlags = 0;
    Args.Recvfrom.mCallbackRecv = NULL;
    Args.Recvfrom.mPort = 0;

    Args.Sendto.mBufferSize = 0;
    Args.Sendto.mSentBytes = 0;
    Args.Sendto.mFlags = 0;
    Args.Sendto.mTimeout = 0;
    Args.Sendto.mPort = 0;

    if(NULL != Args.Bind.mSocket) {
        delete Args.Bind.mSocket;
        Args.Bind.mSocket = NULL;
    }
    mSending = false;
    pthread_mutex_unlock(&mSendMutex);
    pthread_mutex_destroy(&mSendMutex);
}

void UdpIoRequest::init()
{
}

bool UdpIoRequest::post(const char * data, const size_t dataSize, std::string ip, unsigned short port)
{
    if (data == NULL || 0 >= dataSize) {
        printf("data dataSize=%ld.\n", strlen(data));
        return false;
    }
    pthread_mutex_lock(&mSendMutex);
    if (NULL == Args.Bind.mSocket) {
        pthread_mutex_unlock(&mSendMutex);
        return false;
    }
    Args.Sendto.mIP = ip;
    Args.Sendto.mPort = port;
    mSending = true;
    char * const p = Args.Sendto.mBuffer;
    Args.Sendto.mFlags = MSG_DONTROUTE;
    Args.Sendto.mSentBytes = 0;
    size_t arraySize = sizeof(Args.Sendto.mBuffer);
    // printf("arraySize=%ld, dataSize=%ld, Args.Sendto.mSentBytes=%u. data=\n%s\n",
    //         arraySize, dataSize, Args.Sendto.mSentBytes, data);
    while (dataSize > Args.Sendto.mSentBytes) {
        Args.Sendto.mBufferSize = 0;
        const char * sendingData = data + Args.Sendto.mSentBytes;
        size_t copyLength = dataSize - Args.Sendto.mSentBytes;
        if(arraySize <= copyLength) {
            copyLength = arraySize;
        }
        memset(p, 0, arraySize);
        memcpy(p, sendingData, copyLength); // The line has crashed.
        Args.Sendto.mBufferSize = copyLength;
        if (!Args.Bind.mSocket->sendto(this)) {
            if (!Args.Bind.mSocket->sendto(this)) {
                break;
            }
        }
    }
    mSending = false;
    pthread_mutex_unlock(&mSendMutex);
    return (dataSize == Args.Sendto.mSentBytes);
}

};  // udpserver

