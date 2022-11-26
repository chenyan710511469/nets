/*
 * =====================================================================================
 *
 *       Filename:  UdpServerInfo.h
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
#ifndef UDP_CLIENT_UDPSERVERINFO_H
#define UDP_CLIENT_UDPSERVERINFO_H

#include <string>

#include "UdpIoService.h"
#include "sockets/AddressInfo.h"

namespace udpclient
{
class UdpIoService;
class UdpServerInfo : public sockets::AddressInfo
{
public:
    explicit UdpServerInfo(std::string remoteIpAddress, unsigned short remotePort, int clientFd);
    explicit UdpServerInfo(std::string remoteIpAddress, unsigned short remotePort, unsigned short localPort, std::string localIpAddress = "");
    explicit UdpServerInfo(std::string remoteIpAddress, unsigned short remotePort, std::string remoteMacAddress, int socketFd, unsigned short localPort,
            std::string localMacAddress, std::string localIpAddress = "");

    virtual ~UdpServerInfo();

    explicit UdpServerInfo(const UdpServerInfo & other);
    explicit UdpServerInfo(const UdpServerInfo * other);
    UdpServerInfo & operator=(const UdpServerInfo & other);

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
    void setRemoteMacAddress(std::string remoteMacAddress);
    void setLocalIPAddress(std::string localIpAddress);
    void setLocalPort(unsigned short localPort);
    void setLocalMacAddress(std::string localMacAddress);
    void setServerFd(int serverFd);
    void setClientFd(int clientFd);
    void setRequest(void * request);
    void setIpVersion(std::string ipVersion);

private:
    friend class                UdpIoService;

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
};  // udpclient
#endif  // UDP_CLIENT_UDPSERVERINFO_H

