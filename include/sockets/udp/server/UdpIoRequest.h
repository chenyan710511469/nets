/*
 * =====================================================================================
 *
 *       Filename:  UdpIoRequest.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2019年01月20日 08时59分44秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef UDP_SERVER_UDPIOREQUEST_H
#define UDP_SERVER_UDPIOREQUEST_H

#include <memory>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include "UdpIoSocket.h"

namespace udpserver
{
class UdpIoSocket;
struct _recvfrom
{
    unsigned int    mBufferSize;
    unsigned int    mReceivedBytes;
    int             mFlags;
    void            *mCallbackRecv;
    std::string     mIP;
    unsigned short  mPort;
    std::string     mMac;
    char            mBuffer[4096];
};

struct _sendto
{
    unsigned int    mBufferSize;
    unsigned int    mSentBytes;
    int             mFlags;
    int             mTimeout;
    std::string     mIP;
    unsigned short  mPort;
    std::string     mMac;
    char            mBuffer[4096];
};

struct _bind
{
    UdpIoSocket     *mSocket;
    std::string     mIP;
    unsigned short  mPort;
    std::string     mMac;
};
struct _args
{
    struct _recvfrom    Recvfrom;
    struct _sendto      Sendto;
    struct _bind        Bind;
    void                *mCallbackRelease;
    void                *mOwner;
};

class UdpIoRequest
{
public:
    UdpIoRequest(const UdpIoRequest & other) = delete;
    UdpIoRequest & operator=(const UdpIoRequest & other) = delete;

    struct _args Args;

    bool post(const char * data, const size_t dataSize, std::string ip, unsigned short port);

private:
    UdpIoRequest();
    ~UdpIoRequest();
    friend class UdpIoSocket;
    friend class UdpIoScheduleImpl;
    void init();
    bool handleError(int theErrno);

private:
    pthread_mutex_t         mSendMutex;
    bool                    mSending;

};
};  // udpserver

#endif // UDP_SERVER_UDPIOREQUEST_H
