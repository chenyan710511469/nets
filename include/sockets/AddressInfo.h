/*
 * =====================================================================================
 *
 *       Filename:  AddressInfo.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  05/31/2019 06:11:06 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef SOCKETS_ADDRESSINFO_H
#define SOCKETS_ADDRESSINFO_H
#include <string>

#define IPv4                "IPv4"
#define IPv6                "IPv6"

namespace sockets
{
class AddressInfo
{
public:
    AddressInfo()
    {}
    virtual ~AddressInfo()
    {}
    virtual std::string         getRemoteIPAddress() = 0;
    virtual unsigned short      getRemotePort() = 0;
    virtual std::string         getRemoteMacAddress() = 0;
    virtual std::string         getLocalIPAddress() = 0;
    virtual unsigned short      getLocalPort() = 0;
    virtual std::string         getLocalMacAddress() = 0;
    virtual int                 getServerFd() = 0;
    virtual int                 getClientFd() = 0;
    virtual void *              getRequest() = 0;
    virtual std::string         getIpVersion() = 0;

    virtual void setRemoteIPAddress(std::string remoteIpAddress) = 0;
    virtual void setRemotePort(unsigned short remotePort) = 0;
    virtual void setRemoteMacAddress(std::string remoteMac) = 0;
    virtual void setLocalIPAddress(std::string localIpAddress) = 0;
    virtual void setLocalPort(unsigned short localPort) = 0;
    virtual void setLocalMacAddress(std::string localMac) = 0;
    virtual void setServerFd(int serverFd) = 0;
    virtual void setClientFd(int clientFd) = 0;
    virtual void setRequest(void * request) = 0;
    virtual void setIpVersion(std::string ipVersion) = 0;
};
};
#endif  // SOCKETS_ADDRESSINFO_H
