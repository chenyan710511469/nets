/*
 * =====================================================================================
 *
 *       Filename:  TcpIoSchedule.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月05日 15时32分15秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef TCP_CLIENT_TCPIOSCHEDULE_H
#define TCP_CLIENT_TCPIOSCHEDULE_H

#include "TcpIoRequest.h"
#include "TcpIoSocket.h"

#define ONE                             1
#define TWO                             2


namespace tcpclient
{
class TcpIoRequest;
class TcpIoSocket;

class TcpIoSchedule
{
public:
    enum Status
    {
        eStopped        = 0,
        eStarted        = 1,
        eStopping       = 2
    };
    enum ReturnType
    {
        eSuccess            = 0,
        eSuccessNeedClose   = 1,
        eErrorNeedClose     = 2,
        eErrorDontClose     = 3,
        eWithoutData        = 4
    };
    enum OptType
    {
        eGet                    = 0,
        eAdd                    = 1,
        eHeartbeatPacket        = 2,
        eRemove                 = 3
    };

    virtual ~TcpIoSchedule()
    {}

    TcpIoSchedule(const TcpIoSchedule & other) = delete;
    TcpIoSchedule & operator=(const TcpIoSchedule & other) = delete;

    virtual bool preStart(int recvThreadNumber = TWO) = 0;
    virtual bool start() = 0;

    virtual bool stop(TcpIoRequest * request) = 0;
    virtual bool close(TcpIoRequest * request) = 0;

    virtual bool stopAll() = 0;

    virtual bool connect(TcpIoRequest * request, int type = SOCK_STREAM, int protocol = 0) = 0;

    virtual bool post(TcpIoRequest * request) = 0;

    virtual TcpIoRequest * allocRequest(int family) = 0;

    virtual TcpIoSocket * associate(int socketFd) = 0;

    virtual Status getStatus() = 0;

    /**
     * default: isBlocking is false, non-blocking. timeOut=0ms, timeOut is insignificance.
     * if isBlocking is true, blocking, timeOut must be not less than 1000ms,
     * otherwise, timeOut is invalid, and forever blocking until client connect.
     */
    virtual void setBlocking(bool isBlocking = false, int timeout = 0) = 0;

    virtual void setHeartbeatPacketIntervalTime(int heartbeatPacketIntervalTime = 120000) = 0;
    virtual void updateHeartbeatPacket(TcpIoRequest *request) = 0;
    virtual bool recvCallback(TcpIoRequest *request) = 0;
    virtual bool releaseCallback(TcpIoRequest * request) = 0;

protected:
    TcpIoSchedule(){}
};
};  // tcpclient
#endif // TCP_CLIENT_TCPIOSCHEDULE_H

