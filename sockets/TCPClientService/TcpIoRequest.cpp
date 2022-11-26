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
#include "sockets/tcp/client/TcpIoRequest.h"

namespace tcpclient
{
TcpIoRequest::TcpIoRequest(int family)
 : mErrno(0)
 , mSending(false)
 , mFamily(family)
{
    // 注意: 这一步当有std::string成员时,对std::string类型成员赋值会出现段错误.必要时这一步可以去掉对Args的memset().
    // memset(&Args, 0, sizeof(Args));
    memset(&HeartbeatPacket, 0, sizeof(HeartbeatPacket));

    Args.mOwner = NULL;
    Args.mCallbackRelease = NULL;
    memset(Args.Recv.mBuffer, 0, sizeof(Args.Recv.mBuffer));
    Args.Recv.mBufferSize = 0;
    Args.Recv.mReceivedBytes = 0;
    Args.Recv.mFlags = 0;
    Args.Recv.mCallbackRecv = NULL;

    memset(Args.Recv.mBuffer, 0, sizeof(Args.Recv.mBuffer));
    Args.Send.mBufferSize = 0;
    Args.Send.mSentBytes = 0;
    Args.Send.mFlags = 0;
    Args.Send.mTimeout = 0;

    Args.Connect.mAddr = NULL;
    Args.Connect.mAddrLen = 0;
    Args.Connect.mSocket = NULL;
    Args.Connect.mPort = 0;
    Args.Connect.mLocalPort = 0;
    Args.Connect.mCallbackConnect = NULL;

    pthread_mutex_init(&mSendMutex, NULL);
}

TcpIoRequest::~TcpIoRequest()
{
    pthread_mutex_lock(&mSendMutex);
    if(NULL != Args.Connect.mAddr)
    {
        free(Args.Connect.mAddr);
        Args.Connect.mAddr = NULL;
    }
    if(NULL != Args.Connect.mSocket)
    {
        delete Args.Connect.mSocket;
        Args.Connect.mSocket = NULL;
    }
    Args.Recv.mBufferSize = 0;
    Args.Recv.mReceivedBytes = 0;
    Args.Recv.mCallbackRecv = NULL;
    Args.Send.mBufferSize = 0;
    Args.Send.mBufferSize = 0;
    Args.Send.mSentBytes = 0;
    Args.Connect.mAddrLen = 0;
    Args.Connect.mCallbackConnect = NULL;
    Args.mCallbackRelease = NULL;
    Args.mOwner = NULL;

    pthread_mutex_unlock(&mSendMutex);
    mSending = false;
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

bool TcpIoRequest::post(const char * data, const size_t dataSize)
{
    if (data == NULL || 0 >= dataSize) {
        printf("data dataSize=%ld.\n", strlen(data));
        return false;
    }
    pthread_mutex_lock(&mSendMutex);
    if (NULL == Args.Connect.mSocket) {
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
        if (arraySize <= copyLength) {
            copyLength = arraySize;
        }
        memset(p, 0, arraySize);
        memcpy(p, sendingData, copyLength); // The line has crashed.
        Args.Send.mBufferSize = copyLength;
        if (!Args.Connect.mSocket->send(this)) {
            if(EPIPE == mErrno) {
                printf("%s:%s:%d EPIPE error, 对方异常退出.\n", __FILE__, __FUNCTION__, __LINE__);
                break;
            }
            else if (0 >= Args.Connect.mSocket->descriptor()) {
                printf("%s:%s:%d socket fd 已被关闭.\n", __FILE__, __FUNCTION__, __LINE__);
                break;
            }
            else if (!Args.Connect.mSocket->send(this)) {
                break;
            }
        }
    }
    printf("%s:%d, remote ip:%s, remote port:%d, local ip:%s, local port:%d. data:%s\n",
            __FUNCTION__, __LINE__,
            Args.Connect.mIP.c_str(),
            Args.Connect.mPort,
            Args.Connect.mLocalIp.c_str(),
            Args.Connect.mLocalPort,
            data);
    mSending = false;
    pthread_mutex_unlock(&mSendMutex);
    return (dataSize == Args.Send.mSentBytes);
}

int TcpIoRequest::getFamily() {
    return mFamily;
}

};  // tcpclient

