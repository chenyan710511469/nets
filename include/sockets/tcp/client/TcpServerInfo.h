/*
 * =====================================================================================
 *
 *       Filename:  TcpServerInfo.h
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
#ifndef TCP_CLIENT_TCPSERVERINFO_H
#define TCP_CLIENT_TCPSERVERINFO_H

#include <string>

#include "sockets/AddressInfo.h"
#include "TcpIoService.h"

namespace tcpclient
{
class TcpServerInfo : public sockets::AddressInfo
{
public:
    explicit TcpServerInfo(std::string remoteIpAddress, unsigned short remotePort, std::string remoteMac, int clientFd);
    explicit TcpServerInfo(std::string remoteIpAddress, unsigned short remotePort);
    ~TcpServerInfo();
    explicit TcpServerInfo(const TcpServerInfo & other);
    explicit TcpServerInfo(const TcpServerInfo * other);
    TcpServerInfo & operator=(const TcpServerInfo & other);

private:
    friend class    TcpIoService;

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
};  // tcpclient
#endif  // TCP_CLIENT_TCPSERVERINFO_H
