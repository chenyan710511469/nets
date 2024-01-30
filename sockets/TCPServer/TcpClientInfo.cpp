/*
 * =====================================================================================
 *
 *       Filename:  TcpClientInfo.cpp
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
#include "sockets/tcp/server/TcpClientInfo.h"

namespace tcpserver
{
TcpClientInfo::TcpClientInfo(unsigned short localPort, std::string localIpAddress, std::string localMacAddress)
 : mLocalIPAddress(localIpAddress)
 , mLocalPort(localPort)
 , mLocalMacAddress(localMacAddress)
 , mRemotePort(0)
 , mServerFd(0)
 , mClientFd(0)
 , mRequest(NULL)
{}

TcpClientInfo::TcpClientInfo(std::string remoteIpAddress, unsigned short remotePort, std::string remoteMac, int serverFd)
 : mRemoteIPAddress(remoteIpAddress)
 , mRemotePort(remotePort)
 , mRemoteMacAddress(remoteMac)
 , mLocalPort(0)
 , mServerFd(serverFd)
 , mClientFd(0)
 , mRequest(NULL)
{}

TcpClientInfo::~TcpClientInfo()
{
    mRequest = NULL;
}

std::string TcpClientInfo::getRemoteIPAddress()
{
    return mRemoteIPAddress;
}

void TcpClientInfo::setRemoteIPAddress(std::string remoteIpAddress)
{
    mRemoteIPAddress = remoteIpAddress;
}

unsigned short TcpClientInfo::getRemotePort()
{
    return mRemotePort;
}

void TcpClientInfo::setRemotePort(unsigned short remotePort)
{
    mRemotePort = remotePort;
}

std::string TcpClientInfo::getRemoteMacAddress()
{
    return mRemoteMacAddress;
}

void TcpClientInfo::setRemoteMacAddress(std::string remoteMac)
{
    mRemoteMacAddress = remoteMac;
}

int TcpClientInfo::getServerFd()
{
    return mServerFd;
}

void TcpClientInfo::setServerFd(int serverFd)
{
    mServerFd = serverFd;
}

int TcpClientInfo::getClientFd()
{
    return mClientFd;
}

void TcpClientInfo::setClientFd(int clientFd)
{
    mClientFd = clientFd;
}

std::string TcpClientInfo::getLocalIPAddress()
{
    return mLocalIPAddress;
}

void TcpClientInfo::setLocalIPAddress(std::string localIpAddress)
{
    mLocalIPAddress = localIpAddress;
}

unsigned short TcpClientInfo::getLocalPort()
{
    return mLocalPort;
}

void TcpClientInfo::setLocalPort(unsigned short localPort)
{
    mLocalPort = localPort;
}

std::string TcpClientInfo::getLocalMacAddress()
{
    return mLocalMacAddress;
}

void TcpClientInfo::setLocalMacAddress(std::string localMac)
{
    mLocalMacAddress = localMac;
}

void * TcpClientInfo::getRequest()
{
    return mRequest;
}

void TcpClientInfo::setRequest(void * request)
{
    mRequest = request;
}

std::string TcpClientInfo::getIpVersion() {
    return mIpVersion;
}

void TcpClientInfo::setIpVersion(std::string ipVersion) {
    mIpVersion = ipVersion;
}

TcpClientInfo::TcpClientInfo(const TcpClientInfo & other)
 : mRemoteIPAddress(other.mRemoteIPAddress)
 , mRemotePort(other.mRemotePort)
 , mRemoteMacAddress(other.mRemoteMacAddress)
 , mLocalPort(other.mLocalPort)
 , mServerFd(other.mServerFd)
 , mClientFd(other.mClientFd)
 , mRequest(other.mRequest)
 , mIpVersion(other.mIpVersion)
{}

TcpClientInfo::TcpClientInfo(const TcpClientInfo * other)
 : mRemoteIPAddress(other->mRemoteIPAddress)
 , mRemotePort(other->mRemotePort)
 , mRemoteMacAddress(other->mRemoteMacAddress)
 , mLocalPort(other->mLocalPort)
 , mServerFd(other->mServerFd)
 , mClientFd(other->mClientFd)
 , mRequest(other->mRequest)
 , mIpVersion(other->mIpVersion)
{}

TcpClientInfo & TcpClientInfo::operator=(const TcpClientInfo & other)
{
    if(this != &other)
    {
        this->mRemoteIPAddress = other.mRemoteIPAddress;
        this->mRemotePort = other.mRemotePort;
        this->mRemoteMacAddress = other.mRemoteMacAddress;
        this->mLocalPort = other.mLocalPort;
        this->mServerFd = other.mServerFd;
        this->mClientFd = other.mClientFd;
        this->mRequest = other.mRequest;
        this->mIpVersion = other.mIpVersion;
    }
    return *this;
}

};  // namespace tcpserver

