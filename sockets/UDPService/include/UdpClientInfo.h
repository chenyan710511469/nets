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
#ifndef UDPCLIENTINFO_H
#define UDPCLIENTINFO_H

#include <string>

#include "UdpIoService.h"

namespace udpserver
{

class UdpClientInfo
{
public:
    UdpClientInfo(unsigned short localPort, std::string localIpAddress = "");
    UdpClientInfo(std::string remoteIpAddress, unsigned short remotePort, std::string remoteMac, int socketFd,
            unsigned short localPort, std::string localMac, std::string localIpAddress = "");
    virtual ~UdpClientInfo();

    std::string                 getRemoteIPAddress();
    void                        setRemoteIPAddress(std::string remoteIpAddress);
    unsigned short              getRemotePort();
    void                        setRemotePort(unsigned short remotePort);
    std::string                 getRemoteMacAddress();

    std::string                 getLocalIPAddress();
    unsigned short              getLocalPort();
    std::string                 getLocalMacAddress();

    int                         getSocketFd();

private:
    friend class UdpIoService;
    void setRemoteMacAddress(std::string remoteMac);
    void setLocalMacAddress(std::string localMac);
    void setSocketFd(int socketFd);

private:
    std::string                 mRemoteIPAddress;
    unsigned short              mRemotePort;
    std::string                 mRemoteMacAddress;

    std::string                 mLocalIPAddress;
    unsigned short              mLocalPort;
    std::string                 mLocalMacAddress;

    int                         mSocketFd;
};
};  // udpserver
#endif  // UDPCLIENTINFO_H

