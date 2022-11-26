/*
 * =====================================================================================
 *
 *       Filename:  TcpIoRequest.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月05日 14时49分15秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef TCP_CLIENT_TCPIOREQUEST_H
#define TCP_CLIENT_TCPIOREQUEST_H

#include <memory>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include "TcpIoSocket.h"
#include "TcpIoScheduleImpl.h"
#include "TcpServerInfo.h"

namespace tcpclient
{
struct _recv
{
    char            mBuffer[4096];
    unsigned int    mBufferSize;
    unsigned int    mReceivedBytes;
    int             mFlags;
    void            *mCallbackRecv;
};

struct _send
{
    char            mBuffer[4096];
    unsigned int    mBufferSize;
    unsigned int    mSentBytes;
    int             mFlags;
    int             mTimeout;
};

struct _connect
{
    struct sockaddr *mAddr;
    unsigned int    mAddrLen;
    TcpIoSocket     *mSocket;
    std::string     mIP;
    unsigned short  mPort;
    std::string     mLocalIp;
    unsigned short  mLocalPort;
    std::string     mMac;
    void            *mCallbackConnect;
};

struct _args
{
    struct _recv        Recv;
    struct _send        Send;
    struct _connect     Connect;
    void                *mCallbackRelease;
    void                *mOwner;
};

class TcpIoRequest
{
public:
    TcpIoRequest(const TcpIoRequest & other) = delete;
    TcpIoRequest & operator=(const TcpIoRequest & other) = delete;

    struct _args Args;

    int getErrno();
    bool post(const char * data, const size_t dataSize);
    int getFamily();

private:
    TcpIoRequest(int family);
    ~TcpIoRequest();
    void init();
    friend class TcpIoSocket;
    friend class TcpIoScheduleImpl;

private:
    int mErrno;
    struct _HeartbeatPacket
    {
        long long               mLastTime;
        int                     mThreeTimes;
        char                    *mBuffer;
        int                     mBufferSize;
    };
    struct _HeartbeatPacket HeartbeatPacket;

    pthread_mutex_t         mSendMutex;
    bool                    mSending;
    int                     mFamily;
};

};  // tcpclient
#endif // TCP_CLIENT_TCPIOREQUEST_H

