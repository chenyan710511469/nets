/*
 * =====================================================================================
 *
 *       Filename:  testUdpIo.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月09日 15时24分38秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef TESTUDPIO_CPP
#define TESTUDPIO_CPP

#include <stdlib.h>

#include "sockets/udp/server/UdpIoService.h"
#include "sockets/udp/server/UdpMessage.h"
#include "sockets/udp/server/UdpClientInfo.h"
#include "sockets/AddressInfo.h"

namespace udpserver
{
class testUdpIo
{
public:
    testUdpIo()
    : mUdpService(new UdpIoService(&testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this))
    , mUdpClientInfo(10000, "192.168.2.19")
    , mUdpClientInfo1(10001, "192.168.2.19")
    , mUdpClientInfo2(10002, "192.168.2.19")
    , mUdpClientInfo3(10003, "192.168.2.19")
    , mUdpClientInfo4(10004, "192.168.2.19")
    , mUdpClientInfo5(10005, "192.168.2.19")
    , mUdpClientInfo6(10006, "192.168.2.19")
    , mUdpClientInfo7(10007, "192.168.2.19")
    , mUdpClientInfo8(10008, "192.168.2.19")
    {
        if(mUdpService->preStart())
        {
            mUdpService->setHeartbeatPacketIntervalTime(mUdpClientInfo, 30000);
            mUdpService->start(mUdpClientInfo, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpClientInfo1, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpClientInfo2, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpClientInfo3, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpClientInfo4, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpClientInfo5, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpClientInfo6, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpClientInfo7, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpClientInfo8, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
        }
    }

    ~testUdpIo()
    {
        if(NULL != mUdpService)
        {
            delete mUdpService;
            mUdpService = NULL;
        }
    }

    static void UdpMessageCallback(UdpMessage & message, sockets::AddressInfo & clientInfo, void * ownerCallback)
    {
        ((testUdpIo *)ownerCallback)->udpMessage(message, clientInfo);
    }

    void udpMessage(UdpMessage & message, sockets::AddressInfo & clientInfo)
    {
        if(mUdpService->sendMsg(message, clientInfo))
        {
            //std::cout << "succesful" << std::endl;
        }
        else
        {
            std::cout << "failed" << std::endl;
        }
        //std::cout << "IP : " << clientInfo.getIPAddress() << std::endl;
        //std::cout << "Port : " << clientInfo.getPort() << std::endl;
        //std::cout << "Mac : " << clientInfo.getMacAddress() << std::endl;
        //std::cout << "recved data size : " << message.getDataSize() << std::endl;
    }

    static bool releaseCallback(sockets::AddressInfo & udpClient, void * owner)
    {
        printf("Released:\n");
        printf("fd = %d\n", udpClient.getServerFd());
        printf("ip = %s\n", udpClient.getRemoteIPAddress().c_str());
        printf("port = %d\n", udpClient.getRemotePort());
        printf("mac = %s\n", udpClient.getRemoteMacAddress().c_str());
        return true;
    }

public:
    UdpIoService *      mUdpService;
    UdpClientInfo       mUdpClientInfo;
    UdpClientInfo       mUdpClientInfo1;
    UdpClientInfo       mUdpClientInfo2;
    UdpClientInfo       mUdpClientInfo3;
    UdpClientInfo       mUdpClientInfo4;
    UdpClientInfo       mUdpClientInfo5;
    UdpClientInfo       mUdpClientInfo6;
    UdpClientInfo       mUdpClientInfo7;
    UdpClientInfo       mUdpClientInfo8;
};
};  // udpserver

int main(int argc, char * argv[])
{
    const char comd[] = {"ulimit -s unlimited"};
    int ret = ::system(comd);
    udpserver::testUdpIo test;
    sleep(3600 * 8);
    return 0;
}

#endif // TESTUDPIO_CPP

