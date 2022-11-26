/*
 * =====================================================================================
 *
 *       Filename:  UdpIoService.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月09日 10时35分30秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef UDP_SERVER_UDPIOCONTROL_H
#define UDP_SERVER_UDPIOCONTROL_H


#include "UdpIoRequest.h"
#include "UdpIoSocket.h"
#include "UdpIoSchedule.h"
#include "UdpMessage.h"
#include "UdpClientInfo.h"
#include "sockets/AddressInfo.h"

namespace udpserver
{
class UdpMessage;

class UdpIoService
{
public:
    UdpIoService(void (* callbackRecvfrom)(UdpMessage & message, sockets::AddressInfo & clientInfo, void * owner) = NULL,
            bool (* callbackRelease)(sockets::AddressInfo & tcpRelease, void * owner) = NULL,
            void * owner = NULL);
    ~UdpIoService();

    bool preStart();

    bool start(sockets::AddressInfo & clientInfo,
            void (* callbackRecvfrom)(UdpMessage & message, sockets::AddressInfo & clientInfo, void * owner) = NULL,
            bool (* callbackRelease)(sockets::AddressInfo & tcpRelease, void * owner) = NULL,
            void * owner = NULL
        );

    bool stop(sockets::AddressInfo & clientInfo);
    bool stopAll();

    void setHeartbeatPacketIntervalTime(sockets::AddressInfo & clientInfo, int heartbeatPacketIntervalTime);

    bool sendMsg(UdpMessage & message, sockets::AddressInfo & clientInfo);

    bool close(sockets::AddressInfo & clientInfo);

    UdpIoService(const UdpIoService & other) = delete;
    UdpIoService & operator=(const UdpIoService & other) = delete;

private:
    static bool recvfromCallback(UdpIoRequest * request, void * owner);
    bool recvfromCallback(UdpIoRequest * request);

    static bool releaseCallback(UdpIoRequest * request, void * owner);
    bool releaseCallback(UdpIoRequest * request);

private:
    UdpIoSchedule       * mSched;

    void (* mRecvfromCallback)(UdpMessage & message, sockets::AddressInfo & clientInfo, void * owner);
    bool (* mReleaseCallback)(sockets::AddressInfo & tcpRelease, void * owner);
    void * mOwner;

};
};  // udpserver
#endif // UDP_SERVER_UDPIOCONTROL_H

