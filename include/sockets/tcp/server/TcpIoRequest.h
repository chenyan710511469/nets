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
#ifndef TCP_SERVER_TCPIOREQUEST_H
#define TCP_SERVER_TCPIOREQUEST_H

#include <memory>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#include "TcpIoSocket.h"
#include "TcpIoScheduleImpl.h"

namespace tcpserver
{
struct _recv
{
    unsigned int    mBufferSize;
    unsigned int    mReceivedBytes;
    int             mFlags;
    char            mBuffer[4096];
};

struct _send
{
    unsigned int    mBufferSize;
    unsigned int    mSentBytes;
    int             mFlags;
    int             mTimeout;
    char            mBuffer[4096];
};

struct _accept
{
    unsigned int        mAddrLen;
    unsigned short      mRemotePort;
    TcpIoSocket         * mSocket;
    struct sockaddr     * mAddr;
    std::string         mRemoteIP;
    std::string         mRemoteMac;
};

struct _bind
{
    unsigned int        mAddrLen;
    unsigned short      mLocalPort;
    struct sockaddr     * mAddr;
    TcpIoSocket         * mSocket;
    std::string         mLocalIp;
    std::string         mLocalMac;
};

struct _args
{
    struct _recv    Recv;
    struct _send    Send;
    struct _accept  Accept;
    struct _bind    Bind;
};

class TcpIoRequest
{
public:
    TcpIoRequest(const TcpIoRequest & other) = delete;
    TcpIoRequest & operator=(const TcpIoRequest & other) = delete;

    struct _args Args;

    int getErrno();
    int getServerFd();

    bool post(const char * data, const size_t dataSize);
    int getFamily();

private:
    TcpIoRequest(int family, void (* recvCallback)(TcpIoRequest * request, void * owner), void * owner);
    ~TcpIoRequest();
    void init();
    friend class TcpIoSocket;
    friend class TcpIoScheduleImpl;

private:
    int mErrno;

    void (* mRecvCallback)(TcpIoRequest * request, void * owner);
    void * mOwner;

    struct _HeartbeatPacket {
        long long               mLastTime;
        int                     mThreeTimes;
        char                    *mBuffer;
        int                     mBufferSize;
    };
    struct _HeartbeatPacket HeartbeatPacket;
    int mServerFd;

    pthread_mutex_t         mSendMutex;
    // pthread_cond_t          mSendCond;
    bool                    mSending;
    int                     mFamily;
};
};  // tcpserver

#endif // TCP_SERVER_TCPIOREQUEST_H

