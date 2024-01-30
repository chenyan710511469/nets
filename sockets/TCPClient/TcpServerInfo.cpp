/*
 * =====================================================================================
 *
 *       Filename:  TcpServerInfo.cpp
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
#include "sockets/tcp/client/TcpServerInfo.h"

namespace tcpclient
{
TcpServerInfo::TcpServerInfo(std::string remoteIpAddress, unsigned short remotePort, std::string remoteMac, int clientFd)
 : mRemoteIPAddress(remoteIpAddress)
 , mRemotePort(remotePort)
 , mRemoteMacAddress(remoteMac)
 , mLocalPort(0)
 , mServerFd(0)
 , mClientFd(clientFd)
 , mRequest(NULL)
{}

TcpServerInfo::TcpServerInfo(std::string remoteIpAddress, unsigned short remotePort)
 : mRemoteIPAddress(remoteIpAddress)
 , mRemotePort(remotePort)
 , mLocalPort(0)
 , mServerFd(0)
 , mClientFd(0)
 , mRequest(NULL)
{}

TcpServerInfo::~TcpServerInfo()
{
    mRequest = NULL;
}

std::string TcpServerInfo::getRemoteIPAddress()
{
    return mRemoteIPAddress;
}

void TcpServerInfo::setRemoteIPAddress(std::string remoteIpAddress)
{
    mRemoteIPAddress = remoteIpAddress;
}

unsigned short TcpServerInfo::getRemotePort()
{
    return mRemotePort;
}

void TcpServerInfo::setRemotePort(unsigned short remotePort)
{
    mRemotePort = remotePort;
}

std::string TcpServerInfo::getRemoteMacAddress()
{
    return mRemoteMacAddress;
}

void TcpServerInfo::setRemoteMacAddress(std::string remoteMac)
{
    mRemoteMacAddress = remoteMac;
}

int TcpServerInfo::getServerFd()
{
    return mServerFd;
}

void TcpServerInfo::setServerFd(int serverFd)
{
    mServerFd = serverFd;
}

int TcpServerInfo::getClientFd()
{
    return mClientFd;
}

void TcpServerInfo::setClientFd(int clientFd)
{
    mClientFd = clientFd;
}

std::string TcpServerInfo::getLocalIPAddress()
{
    return mLocalIPAddress;
}

void TcpServerInfo::setLocalIPAddress(std::string localIpAddress)
{
    mLocalIPAddress = localIpAddress;
}

unsigned short TcpServerInfo::getLocalPort()
{
    return mLocalPort;
}

void TcpServerInfo::setLocalPort(unsigned short localPort)
{
    mLocalPort = localPort;
}

std::string TcpServerInfo::getLocalMacAddress()
{
    return mLocalMacAddress;
}

void TcpServerInfo::setLocalMacAddress(std::string localMac)
{
    mLocalMacAddress = localMac;
}

void * TcpServerInfo::getRequest()
{
    return mRequest;
}

void TcpServerInfo::setRequest(void * request)
{
    mRequest = request;
}

std::string TcpServerInfo::getIpVersion() {
    return mIpVersion;
}

void TcpServerInfo::setIpVersion(std::string ipVersion) {
    mIpVersion = ipVersion;
}

TcpServerInfo::TcpServerInfo(const TcpServerInfo & other)
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

TcpServerInfo::TcpServerInfo(const TcpServerInfo * other)
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

TcpServerInfo & TcpServerInfo::operator=(const TcpServerInfo & other)
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

};  // tcpclient
