/*
 * =====================================================================================
 *
 *       Filename:  TcpIoService.h
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
#ifndef TCP_SERVER_TCPIOCONTROL_H
#define TCP_SERVER_TCPIOCONTROL_H

#include <mutex>

#include "TcpIoRequest.h"
#include "TcpIoSocket.h"
#include "TcpIoSchedule.h"
#include "TcpMessage.h"
#include "TcpClientInfo.h"
#include "sockets/AddressInfo.h"

namespace tcpserver
{
class TcpMessage;

class TcpIoService
{
public:
    TcpIoService(void (* callbackRecv)(TcpMessage & message, sockets::AddressInfo & clientInfo, void * owner),
            bool (* callbackRelease)(sockets::AddressInfo & tcpRelease, void * owner),
            void * owner);
    ~TcpIoService();
    enum ServerOrClient
    {
        eClient = 0,
        eServer = 1
    };

    bool preStart(bool socketAcceptIsWait = false, int acceptTimeout = 0);

    bool start(sockets::AddressInfo & clientInfo);

    bool stop();

    void setHeartbeatPacketIntervalTime(int heartbeatPacketIntervalTime);

    bool sendMsg(TcpMessage & message, sockets::AddressInfo * addressInfo);

    bool close(ServerOrClient type, sockets::AddressInfo * clientInfo);

    TcpIoService(const TcpIoService & other) = delete;
    TcpIoService & operator=(const TcpIoService & other) = delete;

private:
    static void recvCallback(TcpIoRequest * request, void * owner);
    void recvCallback(TcpIoRequest * request);

    static bool releaseCallback(TcpIoRequest * request, void * owner);
    bool releaseCallback(TcpIoRequest * request);

private:
    TcpIoSchedule       * mSched;

    int                 mPort;
    std::string         mIP;

    void (* mRecvCallback)(TcpMessage & message, sockets::AddressInfo & clientInfo, void * owner);
    bool (* mReleaseCallback)(sockets::AddressInfo & tcpRelease, void * owner);
    void * mOwner;
    std::mutex          mMutex;
};  // class TcpIoService
};  // namespace tcpserver
#endif // TCP_SERVER_TCPIOCONTROL_H

