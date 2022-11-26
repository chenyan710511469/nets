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
#ifndef TCP_CLIENT_TCPIOCONTROL_H
#define TCP_CLIENT_TCPIOCONTROL_H

#include <mutex>

#include "TcpIoRequest.h"
#include "TcpIoSocket.h"
#include "TcpIoSchedule.h"
#include "TcpMessage.h"
#include "TcpServerInfo.h"
#include "sockets/AddressInfo.h"

class TcpMessage;
namespace tcpclient
{
class TcpMessage;
class TcpIoService
{
public:
    TcpIoService(void *owner = NULL,
            bool (*callbackRecv)(TcpMessage & message, sockets::AddressInfo & serverInfo, void *owner) = NULL,
            bool (*callbackConnect)(sockets::AddressInfo & serverInfo, bool isConnected, void *owner) = NULL,
            bool (*callbackRelease)(sockets::AddressInfo & tcpRelease, void *owner) = NULL);
    ~TcpIoService();

    bool preStart(bool socketIsWait = false, int timeout = 0);

    bool start();

    bool stopAll();

    void setHeartbeatPacketIntervalTime(int heartbeatPacketIntervalTime);

    bool connect(sockets::AddressInfo & serverInfo, void *owner = NULL,
            bool (*callbackRecv)(TcpMessage & message, sockets::AddressInfo & serverInfo, void *owner) = NULL,
            bool (*callbackConnect)(sockets::AddressInfo & serverInfo, bool isConnected, void *owner) = NULL,
            bool (*callbackRelease)(sockets::AddressInfo & tcpRelease, void *owner) = NULL);
    bool sendMsg(TcpMessage & message, sockets::AddressInfo * addressInfo);

    bool stop(sockets::AddressInfo & clientInfo);
    bool close(sockets::AddressInfo & clientInfo);

    TcpIoService(const TcpIoService & other) = delete;
    TcpIoService & operator=(const TcpIoService & other) = delete;
private:
    static bool recvCallback(TcpIoRequest *request, void *owner);
    bool recvCallback(TcpIoRequest *request);

    static bool connectCallback(TcpIoRequest *request, bool isConnected, void *owner);
    bool connectCallback(TcpIoRequest *request, bool isConnected);

    static bool releaseCallback(TcpIoRequest *request, void *owner);
    bool releaseCallback(TcpIoRequest *request);

private:
    TcpIoSchedule       *mSched;

    bool (*mCallbackRecv)(TcpMessage & message, sockets::AddressInfo & serverInfo, void *owner);
    bool (*mCallbackConnect)(sockets::AddressInfo & serverInfo, bool isConnected, void *owner);
    bool (*mCallbackRelease)(sockets::AddressInfo & serverInfo, void *owner);
    void *mOwner;
    std::mutex          mMutex;
};
};  // namespace tcpclient
#endif // TCP_CLIENT_TCPIOCONTROL_H

