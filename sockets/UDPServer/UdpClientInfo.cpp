/*
 * =====================================================================================
 *
 *       Filename:  UdpClientInfo.cpp
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
#include "sockets/udp/server/UdpClientInfo.h"

namespace udpserver
{
UdpClientInfo::UdpClientInfo(unsigned short localPort, std::string localIpAddress)
 : mRemotePort(0)
 , mLocalIPAddress(localIpAddress)
 , mLocalPort(localPort)
 , mServerFd(0)
 , mClientFd(0)
 , mRequest(NULL)
{
}

UdpClientInfo::UdpClientInfo(std::string remoteIpAddress, unsigned short remotePort, std::string remoteMac, int serverFd,
        unsigned short localPort, std::string localMac, std::string localIpAddress)
 : mRemoteIPAddress(remoteIpAddress)
 , mRemotePort(remotePort)
 , mRemoteMacAddress(remoteMac)
 , mLocalIPAddress(localIpAddress)
 , mLocalPort(localPort)
 , mLocalMacAddress(localMac)
 , mServerFd(serverFd)
 , mClientFd(0)
 , mRequest(NULL)
{
}

UdpClientInfo::~UdpClientInfo()
{
    mRequest = NULL;
}

std::string UdpClientInfo::getRemoteIPAddress()
{
    return mRemoteIPAddress;
}

void UdpClientInfo::setRemoteIPAddress(std::string remoteIpAddress)
{
    mRemoteIPAddress = remoteIpAddress;
}

unsigned short UdpClientInfo::getRemotePort()
{
    return mRemotePort;
}

void UdpClientInfo::setRemotePort(unsigned short remotePort)
{
    mRemotePort = remotePort;
}

std::string UdpClientInfo::getRemoteMacAddress()
{
    return mRemoteMacAddress;
}

void UdpClientInfo::setRemoteMacAddress(std::string remoteMac)
{
    mRemoteMacAddress = remoteMac;
}

int UdpClientInfo::getServerFd()
{
    return mServerFd;
}

void UdpClientInfo::setServerFd(int serverFd)
{
    mServerFd = serverFd;
}

int UdpClientInfo::getClientFd()
{
    return mClientFd;
}

void UdpClientInfo::setClientFd(int clientFd)
{
    mClientFd = clientFd;
}

std::string UdpClientInfo::getLocalIPAddress()
{
    return mLocalIPAddress;
}

void UdpClientInfo::setLocalIPAddress(std::string localIpAddress)
{
    mLocalIPAddress = localIpAddress;
}

unsigned short UdpClientInfo::getLocalPort()
{
    return mLocalPort;
}

void UdpClientInfo::setLocalPort(unsigned short localPort)
{
    mLocalPort = localPort;
}

std::string UdpClientInfo::getLocalMacAddress()
{
    return mLocalMacAddress;
}

void UdpClientInfo::setLocalMacAddress(std::string localMac)
{
    mLocalMacAddress = localMac;
}

void * UdpClientInfo::getRequest()
{
    return mRequest;
}

void UdpClientInfo::setRequest(void * request)
{
    mRequest = request;
}

std::string UdpClientInfo::getIpVersion() {
    return mIpVersion;
}

void UdpClientInfo::setIpVersion(std::string ipVersion) {
    mIpVersion = ipVersion;
}

UdpClientInfo::UdpClientInfo(const UdpClientInfo & other)
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
{}

UdpClientInfo::UdpClientInfo(const UdpClientInfo * other)
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
{}

UdpClientInfo & UdpClientInfo::operator=(const UdpClientInfo & other)
{
    if(this != &other) {
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

};  // udpserver

