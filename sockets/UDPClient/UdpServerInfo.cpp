/*
 * =====================================================================================
 *
 *       Filename:  UdpServerInfo.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2018年12月23日 21时52分01秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee)
 *   Organization:
 *
 * =====================================================================================
 */
#include "sockets/udp/client/UdpServerInfo.h"

namespace udpclient
{
UdpServerInfo::UdpServerInfo(std::string remoteIpAddress, unsigned short remotePort, int clientFd)
 : mRemoteIPAddress(remoteIpAddress)
 , mRemotePort(remotePort)
 , mLocalPort(0)
 , mServerFd(0)
 , mClientFd(clientFd)
 , mRequest(NULL)
{}

UdpServerInfo::UdpServerInfo(std::string remoteIpAddress, unsigned short remotePort, unsigned short localPort, std::string localIpAddress)
 : mRemoteIPAddress(remoteIpAddress)
 , mRemotePort(remotePort)
 , mLocalIPAddress(localIpAddress)
 , mLocalPort(localPort)
 , mServerFd(0)
 , mClientFd(0)
 , mRequest(NULL)
{}

UdpServerInfo::UdpServerInfo(std::string remoteIpAddress, unsigned short remotePort, std::string remoteMacAddress, int clientFd, unsigned short localPort,
        std::string localMacAddress, std::string localIpAddress)
 : mRemoteIPAddress(remoteIpAddress)
 , mRemotePort(remotePort)
 , mRemoteMacAddress(remoteMacAddress)
 , mLocalIPAddress(localIpAddress)
 , mLocalPort(localPort)
 , mLocalMacAddress(localMacAddress)
 , mServerFd(0)
 , mClientFd(clientFd)
 , mRequest(NULL)
{}

UdpServerInfo::~UdpServerInfo()
{
    mRequest = NULL;
}

std::string UdpServerInfo::getRemoteIPAddress()
{
    return mRemoteIPAddress;
}

void UdpServerInfo::setRemoteIPAddress(std::string remoteIpAddress)
{
    mRemoteIPAddress = remoteIpAddress;
}

unsigned short UdpServerInfo::getRemotePort()
{
    return mRemotePort;
}

void UdpServerInfo::setRemotePort(unsigned short remotePort)
{
    mRemotePort = remotePort;
}

std::string UdpServerInfo::getRemoteMacAddress()
{
    return mRemoteMacAddress;
}

void UdpServerInfo::setRemoteMacAddress(std::string remoteMacAddress)
{
    mRemoteMacAddress = remoteMacAddress;
}

int UdpServerInfo::getServerFd()
{
    return mServerFd;
}

void UdpServerInfo::setServerFd(int serverFd)
{
    mServerFd = serverFd;
}

int UdpServerInfo::getClientFd()
{
    return mClientFd;
}

void UdpServerInfo::setClientFd(int clientFd)
{
    mClientFd = clientFd;
}

std::string UdpServerInfo::getLocalIPAddress()
{
    return mLocalIPAddress;
}

void UdpServerInfo::setLocalIPAddress(std::string localIpAddress)
{
    mLocalIPAddress = localIpAddress;
}

unsigned short UdpServerInfo::getLocalPort()
{
    return mLocalPort;
}

void UdpServerInfo::setLocalPort(unsigned short localPort)
{
    mLocalPort = localPort;
}

std::string UdpServerInfo::getLocalMacAddress()
{
    return mLocalMacAddress;
}

void UdpServerInfo::setLocalMacAddress(std::string localMacAddress)
{
    mLocalMacAddress = localMacAddress;
}

void * UdpServerInfo::getRequest()
{
    return mRequest;
}

void UdpServerInfo::setRequest(void * request)
{
    mRequest = request;
}

std::string UdpServerInfo::getIpVersion() {
    return mIpVersion;
}

void UdpServerInfo::setIpVersion(std::string ipVersion) {
    mIpVersion = ipVersion;
}

UdpServerInfo::UdpServerInfo(const UdpServerInfo & other)
 : mRemoteIPAddress(other.mRemoteIPAddress)
 , mRemotePort(other.mRemotePort)
 , mRemoteMacAddress(other.mRemoteMacAddress)
 , mLocalIPAddress(other.mLocalIPAddress)
 , mLocalPort(other.mLocalPort)
 , mLocalMacAddress(other.mLocalMacAddress)
 , mServerFd(other.mServerFd)
 , mClientFd(other.mClientFd)
 , mRequest(other.mRequest)
 , mIpVersion(other.mIpVersion)
{
}

UdpServerInfo::UdpServerInfo(const UdpServerInfo * other)
 : mRemoteIPAddress(other->mRemoteIPAddress)
 , mRemotePort(other->mRemotePort)
 , mRemoteMacAddress(other->mRemoteMacAddress)
 , mLocalIPAddress(other->mLocalIPAddress)
 , mLocalPort(other->mLocalPort)
 , mLocalMacAddress(other->mLocalMacAddress)
 , mServerFd(other->mServerFd)
 , mClientFd(other->mClientFd)
 , mRequest(other->mRequest)
 , mIpVersion(other->mIpVersion)
{
}

UdpServerInfo & UdpServerInfo::operator=(const UdpServerInfo & other)
{
    if(this != &other)
    {
        this->mRemoteIPAddress = other.mRemoteIPAddress;
        this->mRemotePort = other.mRemotePort;
        this->mRemoteMacAddress = other.mRemoteMacAddress;
        this->mLocalIPAddress = other.mLocalIPAddress;
        this->mLocalPort = other.mLocalPort;
        this->mLocalMacAddress = other.mLocalMacAddress;
        this->mServerFd = other.mServerFd;
        this->mClientFd = other.mClientFd;
        this->mRequest = other.mRequest;
        this->mIpVersion = other.mIpVersion;
    }
    return *this;
}

};  // namespace udpclient

