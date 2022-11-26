/*
 * =====================================================================================
 *
 *       Filename:  testTcpIo.cpp
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
#ifndef TCP_SERVER_TESTTCPIO_CPP
#define TCP_SERVER_TESTTCPIO_CPP

#include <stdlib.h>

#include "sockets/tcp/server/TcpIoService.h"
#include "sockets/tcp/server/TcpMessage.h"
#include "sockets/tcp/server/TcpClientInfo.h"
#include "sockets/AddressInfo.h"

namespace tcpserver
{
class testTcpIo
{
public:
    testTcpIo()
     : mTcpService(new TcpIoService(&testTcpIo::TcpMessageCallback, &testTcpIo::releaseCallback, this))
     , mClientInfo1(TcpClientInfo(10000, ""))
     , mClientInfo2(TcpClientInfo(20000, ""))
    {
        if(mTcpService->preStart(false, 10000 * 5))
        {
            mTcpService->setHeartbeatPacketIntervalTime(30000);
            mTcpService->start(mClientInfo1);
            mTcpService->start(mClientInfo2);
        }
    }

    ~testTcpIo()
    {
        if(NULL != mTcpService)
        {
            delete mTcpService;
            mTcpService = NULL;
        }
    }

    static void TcpMessageCallback(TcpMessage & message, sockets::AddressInfo & clientInfo, void * ownerCallback)
    {
        ((testTcpIo *)ownerCallback)->tcpMessage(message, clientInfo);
    }

    void tcpMessage(TcpMessage & message, sockets::AddressInfo & clientInfo)
    {
        if(mTcpService->sendMsg(message, &clientInfo))
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

    static bool releaseCallback(sockets::AddressInfo & tcpClient, void * owner)
    {
        printf("Released:\n");
        printf("fd = %d\n", tcpClient.getServerFd());
        printf("ip = %s\n", tcpClient.getRemoteIPAddress().c_str());
        printf("port = %d\n", tcpClient.getRemotePort());
        printf("mac = %s\n", tcpClient.getRemoteMacAddress().c_str());
        return true;
    }

private:
    TcpIoService                * mTcpService;
    TcpClientInfo               mClientInfo1;
    TcpClientInfo               mClientInfo2;
};
};  // tcpserver

int main(int argc, char * argv[])
{
    const char comd[] = {"ulimit -s unlimited"};
    int ret = ::system(comd);
    tcpserver::testTcpIo test;
    sleep(60 * 5);
    return 0;
}

#endif // TCP_SERVER_TESTTCPIO_CPP

