/*
 * =====================================================================================
 *
 *       Filename:  UdpIoSchedule.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2019年01月20日 19时03分09秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef UDP_CLIENT_UDPIOSCHEDULE_H
#define UDP_CLIENT_UDPIOSCHEDULE_H

#include "UdpIoRequest.h"
#include "UdpIoSocket.h"

namespace udpclient
{
class UdpIoRequest;
class UdpIoSocket;
class UdpIoSchedule
{
public:
    enum Status
    {
        eStopped        = 0,
        eStarted        = 1,
        eStopping       = 2,
        eStarting       = 3
    };
    enum OptType
    {
        eGet            = 0,
        eAdd            = 1,
        eRemove         = 2
    };

    virtual ~UdpIoSchedule(){}

    virtual bool start(UdpIoRequest *request, int family = AF_INET, int type = SOCK_DGRAM, int protocol = 0) = 0;
    virtual bool stop(UdpIoRequest *request) = 0;
    virtual bool stopAll() = 0;

    virtual bool close(UdpIoRequest *request) = 0;

    virtual bool post(UdpIoRequest * request) = 0;
    virtual bool sendto(UdpIoRequest * request) = 0;

    virtual UdpIoRequest * allocRequest() = 0;

    virtual UdpIoSocket * associate(int socketFd) = 0;

    virtual void updateHeartbeatPacket(UdpIoRequest *request) = 0;
    virtual void setHeartbeatPacketIntervalTime(UdpIoRequest *request, int heartbeatPacketIntervalTime) = 0;
    virtual bool recvfromCallback(UdpIoRequest *request) = 0;
    virtual void freeRequest(UdpIoRequest * request) = 0;
    virtual bool isReleasing(UdpIoRequest * request) = 0;

    // virtual UdpIoRequest *getRequestForSendto() = 0;

    UdpIoSchedule(const UdpIoSchedule & other) = delete;
    UdpIoSchedule & operator=(const UdpIoSchedule & other) = delete;
protected:
    UdpIoSchedule(){}
};  // UdpIoSchedule
};  // udpclient

#endif // UDP_CLIENT_UDPIOSCHEDULE_H
