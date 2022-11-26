/*
 * =====================================================================================
 *
 *       Filename:  TcpClientInfo.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2018年12月23日 21时45分04秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee)
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef TCP_SERVER_TCPCLIENTINFO_H
#define TCP_SERVER_TCPCLIENTINFO_H

#include <string>
#include "TcpIoService.h"
#include "sockets/AddressInfo.h"

namespace tcpserver
{
class TcpIoService;
class TcpClientInfo : public sockets::AddressInfo
{
public:
    explicit TcpClientInfo(unsigned short localPort, std::string localIpAddress = "", std::string localMacAddress = "");
    explicit TcpClientInfo(std::string remoteIpAddress, unsigned short remotePort, std::string remoteMac, int serverFd);
    ~TcpClientInfo();

    explicit TcpClientInfo(const TcpClientInfo & other);
    explicit TcpClientInfo(const TcpClientInfo * other);
    TcpClientInfo & operator=(const TcpClientInfo & other);

private:
    std::string         getRemoteIPAddress();
    unsigned short      getRemotePort();
    std::string         getRemoteMacAddress();
    std::string         getLocalIPAddress();
    unsigned short      getLocalPort();
    std::string         getLocalMacAddress();
    int                 getServerFd();
    int                 getClientFd();
    void *              getRequest();
    std::string         getIpVersion();

    void setRemoteIPAddress(std::string remoteIpAddress);
    void setRemotePort(unsigned short remotePort);
    void setRemoteMacAddress(std::string remoteMac);
    void setLocalIPAddress(std::string localIpAddress);
    void setLocalPort(unsigned short localPort);
    void setLocalMacAddress(std::string localMac);
    void setServerFd(int serverFd);
    void setClientFd(int clientFd);
    void setRequest(void * request);
    void setIpVersion(std::string ipVersion);

private:
    friend class                TcpIoService;

    std::string                 mRemoteIPAddress;
    unsigned short              mRemotePort;
    std::string                 mRemoteMacAddress;
    std::string                 mLocalIPAddress;
    unsigned short              mLocalPort;
    std::string                 mLocalMacAddress;
    int                         mServerFd;
    int                         mClientFd;
    void *                      mRequest;
    std::string                 mIpVersion;
};
};  // tcpserver
#endif  // TCP_SERVER_TCPCLIENTINFO_H

