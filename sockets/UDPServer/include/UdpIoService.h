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
#ifndef UDPIOCONTROL_H
#define UDPIOCONTROL_H


#include "UdpIoRequest.h"
#include "UdpIoSocket.h"
#include "UdpIoSchedule.h"
#include "UdpMessage.h"
#include "UdpClientInfo.h"

namespace udpserver
{
class UdpMessage;
class UdpClientInfo;

class UdpIoService
{
public:
    UdpIoService(void (* callbackRecvfrom)(UdpMessage & message, UdpClientInfo & clientInfo, void * owner) = NULL,
            bool (* callbackRelease)(UdpClientInfo & tcpRelease, void * owner) = NULL,
            void * owner = NULL);
    ~UdpIoService();

    bool preStart();

    bool start(UdpClientInfo & clientInfo,
            void (* callbackRecvfrom)(UdpMessage & message, UdpClientInfo & clientInfo, void * owner) = NULL,
            bool (* callbackRelease)(UdpClientInfo & tcpRelease, void * owner) = NULL,
            void * owner = NULL
            );

    bool stop(UdpClientInfo & clientInfo);
    bool stopAll();

    void setHeartbeatPacketIntervalTime(UdpClientInfo & clientInfo, int heartbeatPacketIntervalTime);

    bool sendMsg(UdpMessage & message, UdpClientInfo & clientInfo);

    std::string getIPAddress(int fd);
    unsigned short getPort(int fd);
    std::string getMacAddress(int fd);

    bool close(UdpClientInfo & clientInfo);

    UdpIoService(const UdpIoService & other) = delete;
    UdpIoService & operator=(const UdpIoService & other) = delete;
private:
    static bool recvfromCallback(UdpIoRequest * request, void * owner);
    bool recvfromCallback(UdpIoRequest * request);

    static bool releaseCallback(UdpIoRequest * request, void * owner);
    bool releaseCallback(UdpIoRequest * request);

    UdpIoRequest * getRequest(UdpClientInfo & clientInfo);

private:
    UdpIoSchedule       * mSched;

    void (* mRecvfromCallback)(UdpMessage & message, UdpClientInfo & clientInfo, void * owner);
    bool (* mReleaseCallback)(UdpClientInfo & tcpRelease, void * owner);
    void * mOwner;

};
};  // udpserver
#endif // UDPIOCONTROL_H

