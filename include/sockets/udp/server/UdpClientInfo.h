/*
 * =====================================================================================
 *
 *       Filename:  UdpClientInfo.h
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
#ifndef UDP_SERVER_UDPCLIENTINFO_H
#define UDP_SERVER_UDPCLIENTINFO_H

#include <string>

#include "UdpIoService.h"
#include "sockets/AddressInfo.h"

namespace udpserver
{
class UdpClientInfo : public sockets::AddressInfo
{
public:
    explicit UdpClientInfo(unsigned short localPort, std::string localIpAddress = "");
    explicit UdpClientInfo(std::string remoteIpAddress, unsigned short remotePort, std::string remoteMac, int socketFd,
            unsigned short localPort, std::string localMac, std::string localIpAddress = "");
    virtual ~UdpClientInfo();

    explicit UdpClientInfo(const UdpClientInfo & other);
    explicit UdpClientInfo(const UdpClientInfo * other);
    UdpClientInfo & operator=(const UdpClientInfo & other);

private:
    std::string                 getRemoteIPAddress();
    unsigned short              getRemotePort();
    std::string                 getRemoteMacAddress();
    std::string                 getLocalIPAddress();
    unsigned short              getLocalPort();
    std::string                 getLocalMacAddress();
    int                         getServerFd();
    int                         getClientFd();
    void *                      getRequest();
    std::string                 getIpVersion();

    void setRemoteIPAddress(std::string remoteIpAddress);
    void setRemotePort(unsigned short remotePort);
    void setRemoteMacAddress(std::string remoteMac);
    void setLocalIPAddress(std::string localIpAddress);
    void setLocalPort(unsigned short localPort);
    void setLocalMacAddress(std::string localMacAddress);
    void setServerFd(int serverFd);
    void setClientFd(int clientFd);
    void setRequest(void * request);
    void setIpVersion(std::string ipVersion);

private:
    friend class UdpIoService;
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
};  // udpserver
#endif  // UDP_SERVER_UDPCLIENTINFO_H

