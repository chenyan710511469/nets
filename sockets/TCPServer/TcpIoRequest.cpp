/*
 * =====================================================================================
 *
 *       Filename:  TcpIoRequest.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月05日 14时48分32秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#include "sockets/tcp/server/TcpIoRequest.h"

namespace tcpserver
{
TcpIoRequest::TcpIoRequest(int family, void (* recvCallback)(TcpIoRequest * request, void * owner), void * owner)
 : mErrno(0)
 , mRecvCallback(recvCallback)
 , mOwner(owner)
 , mServerFd(-1)
 , mSending(false)
 , mFamily(family)
{
    // 注意: 这一步当有std::string成员时,对std::string类型成员赋值会出现段错误.必要时这一步可以去掉对Args的memset().
    // memset(&Args, 0, sizeof(Args));
    memset(&HeartbeatPacket, 0, sizeof(HeartbeatPacket));

    memset(Args.Recv.mBuffer, 0, sizeof(Args.Recv.mBuffer));
    Args.Recv.mBufferSize = 0;
    Args.Recv.mReceivedBytes = 0;
    Args.Recv.mFlags = 0;

    memset(Args.Recv.mBuffer, 0, sizeof(Args.Recv.mBuffer));
    Args.Send.mBufferSize = 0;
    Args.Send.mSentBytes = 0;
    Args.Send.mFlags = 0;
    Args.Send.mTimeout = 0;

    Args.Accept.mAddr = NULL;
    Args.Accept.mAddrLen = 0;
    Args.Accept.mSocket = NULL;

    Args.Bind.mAddr = NULL;
    Args.Bind.mAddrLen = 0;
    Args.Bind.mSocket = NULL;
    Args.Bind.mLocalPort = 0;

    pthread_mutex_init(&mSendMutex, NULL);
}

TcpIoRequest::~TcpIoRequest()
{
    if(NULL != Args.Accept.mAddr)
    {
        free(Args.Accept.mAddr);
        Args.Accept.mAddr = NULL;
    }
    if(NULL != Args.Accept.mSocket)
    {
        delete Args.Accept.mSocket;
        Args.Accept.mSocket = NULL;
    }
    if(NULL != Args.Bind.mAddr)
    {
        free(Args.Bind.mAddr);
        Args.Bind.mAddr = NULL;
    }
    if(NULL != Args.Bind.mSocket)
    {
        delete Args.Bind.mSocket;
        Args.Bind.mSocket = NULL;
    }
    Args.Recv.mBufferSize = 0;
    Args.Recv.mReceivedBytes = 0;
    Args.Send.mBufferSize = 0;
    Args.Send.mBufferSize = 0;
    Args.Send.mSentBytes = 0;
    Args.Accept.mAddrLen = 0;
    Args.Bind.mAddrLen = 0;
    mRecvCallback = NULL;
    mOwner = NULL;

    mSending = false;
    pthread_mutex_unlock(&mSendMutex);
    pthread_mutex_destroy(&mSendMutex);
}

void TcpIoRequest::init()
{
    struct timespec aTime;
    memset(&aTime, 0, sizeof(struct timespec));
    if (0 != clock_gettime(CLOCK_BOOTTIME, &aTime)) {
        std::runtime_error runtimeError("clock_gettime() error.");
        throw runtimeError;
    }
    long long millisecond = (aTime.tv_sec * 1000) + (aTime.tv_nsec / 1000000);
    HeartbeatPacket.mLastTime = millisecond;
    HeartbeatPacket.mBuffer = (char *)"\\0";
    HeartbeatPacket.mBufferSize = strlen(HeartbeatPacket.mBuffer);
    HeartbeatPacket.mThreeTimes = 0;
}

int TcpIoRequest::getErrno()
{
    return mErrno;
}

int TcpIoRequest::getServerFd()
{
    return mServerFd;
}

bool TcpIoRequest::post(const char * data, const size_t dataSize)
{
    if (data == NULL || 0 >= dataSize) {
        printf("data dataSize=%ld.\n", strlen(data));
        return false;
    }
    pthread_mutex_lock(&mSendMutex);
    if (NULL == Args.Accept.mSocket) {
        pthread_mutex_unlock(&mSendMutex);
        return false;
    }
    mSending = true;
    char * const p = Args.Send.mBuffer;
    Args.Send.mFlags = MSG_DONTROUTE;
    Args.Send.mSentBytes = 0;
    size_t arraySize = sizeof(Args.Send.mBuffer);
    // printf("arraySize=%ld, dataSize=%ld, Args.Send.mSentBytes=%u. data=\n%s\n",
    //         arraySize, dataSize, Args.Send.mSentBytes, data);
    while (dataSize > Args.Send.mSentBytes) {
        Args.Send.mBufferSize = 0;
        const char * sendingData = data + Args.Send.mSentBytes;
        size_t copyLength = dataSize - Args.Send.mSentBytes;
        if(arraySize <= copyLength) {
            copyLength = arraySize;
        }
        memset(p, 0, arraySize);
        memcpy(p, sendingData, copyLength); // The line has crashed.
        Args.Send.mBufferSize = copyLength;
        if (!Args.Accept.mSocket->send(this)) {
            if(EPIPE == mErrno) {
                printf("%s:%s:%d EPIPE error, 对方异常退出.\n", __FILE__, __FUNCTION__, __LINE__);
                break;
            }
            else if (0 >= Args.Accept.mSocket->descriptor()) {
                printf("%s:%s:%d socket fd 已被关闭.\n", __FILE__, __FUNCTION__, __LINE__);
                break;
            }
            else if (!Args.Accept.mSocket->send(this)) {
                break;
            }
        }
    }
    mSending = false;
    pthread_mutex_unlock(&mSendMutex);
    return (dataSize == Args.Send.mSentBytes);
}

int TcpIoRequest::getFamily() {
    return mFamily;
}

};  // tcpserver

