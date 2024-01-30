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

#include "sockets/udp/client/UdpIoService.h"
#include "sockets/udp/client/UdpMessage.h"
#include "sockets/udp/client/UdpServerInfo.h"
#include "sockets/AddressInfo.h"

namespace udpclient
{
class testUdpIo
{
public:
    testUdpIo()
    : mUdpService(new UdpIoService(&testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this))
    , mUdpServerInfo("192.168.2.19", 20000, 20000, "192.168.2.16")
    , mUdpServerInfo1("192.168.2.19", 20001, 20001, "192.168.2.16")
    , mUdpServerInfo2("192.168.2.19", 20002, 20002, "192.168.2.16")
    , mUdpServerInfo3("192.168.2.19", 20003, 20003, "192.168.2.16")
    , mUdpServerInfo4("192.168.2.19", 20004, 20004, "192.168.2.16")
    , mUdpServerInfo5("192.168.2.19", 20005, 20005, "192.168.2.16")
    , mUdpServerInfo6("192.168.2.19", 20006, 20006, "192.168.2.16")
    , mUdpServerInfo7("192.168.2.19", 10007, 20007, "192.168.2.16")
    , mUdpServerInfo8("192.168.2.19", 10008, 20008, "192.168.2.16")
    {
        if(mUdpService->preStart())
        {
            mUdpService->setHeartbeatPacketIntervalTime(mUdpServerInfo, 30000);
            mUdpService->start(mUdpServerInfo, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpServerInfo1, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpServerInfo2, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpServerInfo3, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpServerInfo4, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpServerInfo5, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpServerInfo6, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpServerInfo7, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
            mUdpService->start(mUdpServerInfo8, &testUdpIo::UdpMessageCallback, &testUdpIo::releaseCallback, this);
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

    static void UdpMessageCallback(UdpMessage & message, sockets::AddressInfo & serverInfo, void * ownerCallback)
    {
        ((testUdpIo *)ownerCallback)->udpMessage(message, serverInfo);
    }

    void udpMessage(UdpMessage & message, sockets::AddressInfo & serverInfo)
    {
        if(mUdpService->sendMsg(message, serverInfo))
        {
            //std::cout << "succesful" << std::endl;
        }
        else
        {
            std::cout << "failed" << std::endl;
        }
        //std::cout << "IP : " << serverInfo.getIPAddress() << std::endl;
        //std::cout << "Port : " << serverInfo.getPort() << std::endl;
        //std::cout << "Mac : " << serverInfo.getMacAddress() << std::endl;
        //std::cout << "recved data size : " << message.getDataSize() << std::endl;
    }

    static bool releaseCallback(sockets::AddressInfo & udpServer, void * owner)
    {
        printf("Released:\n");
        printf("fd = %d\n", udpServer.getClientFd());
        printf("ip = %s\n", udpServer.getRemoteIPAddress().c_str());
        printf("port = %d\n", udpServer.getRemotePort());
        printf("mac = %s\n", udpServer.getRemoteMacAddress().c_str());
        return true;
    }

    void startTest()
    {
        char buffer[] = {"Hello world."};
        sockets::AddressInfo * addressInfo = &mUdpServerInfo;
        UdpMessage message(addressInfo->getClientFd());
        message.setData(buffer, sizeof(buffer));
        udpMessage(message, mUdpServerInfo);
        udpMessage(message, mUdpServerInfo1);
        udpMessage(message, mUdpServerInfo2);
        udpMessage(message, mUdpServerInfo3);
        udpMessage(message, mUdpServerInfo4);
        udpMessage(message, mUdpServerInfo5);
        udpMessage(message, mUdpServerInfo6);
        udpMessage(message, mUdpServerInfo7);
        //udpMessage(message, mUdpServerInfo8);
    }

public:
    UdpIoService        *mUdpService;
    UdpServerInfo        mUdpServerInfo;
    UdpServerInfo        mUdpServerInfo1;
    UdpServerInfo        mUdpServerInfo2;
    UdpServerInfo        mUdpServerInfo3;
    UdpServerInfo        mUdpServerInfo4;
    UdpServerInfo        mUdpServerInfo5;
    UdpServerInfo        mUdpServerInfo6;
    UdpServerInfo        mUdpServerInfo7;
    UdpServerInfo        mUdpServerInfo8;
};
};  // udpclient

int main(int argc, char * argv[])
{
    const char comd[] = {"ulimit -s unlimited"};
    int ret = ::system(comd);
    udpclient::testUdpIo test;
    test.startTest();
    sleep(3600 * 8);
    return 0;
}

#endif // TESTUDPIO_CPP

