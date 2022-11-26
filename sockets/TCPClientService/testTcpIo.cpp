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
#ifndef TESTTCPIO_CPP
#define TESTTCPIO_CPP

#include <stdlib.h>

#include "sockets/tcp/client/TcpIoService.h"
#include "sockets/tcp/client/TcpMessage.h"
#include "sockets/AddressInfo.h"
#include "sockets/tcp/client/TcpServerInfo.h"

namespace tcpclient
{
class testTcpIo
{
public:
    testTcpIo()
     : mTcpService(new TcpIoService(this,
                 &TcpMessageCallback,
                 &connectCallback,
                 &releaseCallback)
             )
    {
        if(mTcpService->preStart(false, 10000 * 5))
        {
            mTcpService->setHeartbeatPacketIntervalTime(30000);
            mTcpService->start();
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
    void connect()
    {
        TcpServerInfo serverInfo(TcpServerInfo("192.168.2.19", 10000));
        sockets::AddressInfo * info = &serverInfo;
        try
        {
            if(mTcpService->connect(serverInfo))
            {
                std::cout << "connect success: " << info->getClientFd() << std::endl;
            }
            else
            {
                std::cout << "connect failed" << std::endl;
            }
        }
        catch(std::exception & ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    static bool TcpMessageCallback(TcpMessage & message, sockets::AddressInfo & serverInfo, void * ownerCallback)
    {
        return ((testTcpIo *)ownerCallback)->tcpMessage(message, serverInfo);
    }

    bool tcpMessage(TcpMessage & message, sockets::AddressInfo & serverInfo)
    {
        //std::cout << __FILE__ << ":" << __LINE__ << "  " << (char*)message.getData() << std::endl;
        if(mTcpService->sendMsg(message, &serverInfo))
        {
            std::cout << "succesful" << std::endl;
        }
        else
        {
            std::cout << "failed" << std::endl;
        }
        std::cout << "IP : " << serverInfo.getRemoteIPAddress() << std::endl;
        std::cout << "Port : " << serverInfo.getRemotePort() << std::endl;
        std::cout << "Mac : " << serverInfo.getRemoteMacAddress() << std::endl;
        std::cout << "recved data size : " << message.getDataSize() << std::endl;
        return true;
    }

    static bool releaseCallback(sockets::AddressInfo & tcpServer, void * owner)
    {
        printf("Released:\n");
        printf("fd = %d\n", tcpServer.getClientFd());
        printf("ip = %s\n", tcpServer.getRemoteIPAddress().c_str());
        printf("port = %d\n", tcpServer.getRemotePort());
        printf("mac = %s\n", tcpServer.getRemoteMacAddress().c_str());
        return true;
    }

    static bool connectCallback(sockets::AddressInfo & tcpServer, bool isConnected, void * owner)
    {
        return ((testTcpIo *)owner)->connectCallback(tcpServer, isConnected);
    }
    bool connectCallback(sockets::AddressInfo & tcpServer, bool isConnected)
    {
        std::cout << "socket fd is: " << tcpServer.getClientFd() << std::endl;
        return true;
    }
public:
    TcpIoService * mTcpService;
};
};  // tcpclient

int main(int argc, char * argv[])
{
    const char comd[] = {"ulimit -s unlimited"};
    int ret = ::system(comd);
    tcpclient::testTcpIo test;
    test.connect();
    sleep(3600 * 24);
    return 0;
}

#endif // TESTTCPIO_CPP

