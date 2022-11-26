/*
 * =====================================================================================
 *
 *       Filename:  ChatA.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/01/2020 06:00:17 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee)
 *   Organization:
 *
 * =====================================================================================
 */
#include "ChatA.h"

#include <stdio.h>
#include <stdlib.h>

#include <exception>

ChatA::ChatA()
 : mReceiver(new Receiver())
 , mNetManager(NULL)
{
}

ChatA::~ChatA()
{
    if (NULL != mNetManager) {
        delete mNetManager;
        mNetManager = NULL;
    }
    delete mReceiver;
    mReceiver = NULL;
}

bool ChatA::preStart(std::string & xmlConfigPath)
{
    try {
        mNetManager = new protocols::NetworkManager(xmlConfigPath);
        return mNetManager->preStart(mReceiver);
    }
    catch (std::exception & ex) {
        printf("%s\n", ex.what());
    }
    return false;
}

bool ChatA::start()
{
    try {
        return mNetManager->startTcpServer();
        // if (mNetManager->startTcpServer()) {
        //     return mNetManager->startUdpServer();
        // }
    }
    catch (std::exception & ex) {
        printf("%s\n", ex.what());
    }
    return false;
}

int main(int argc, char * argv[])
{
    std::string xmlConfigPath("/home/chenyan/projects/project1/tcpServer/configuration.xml");
    ChatA * chatA = new ChatA();
    if (chatA->preStart(xmlConfigPath)) {
        if (chatA->start()) {
            int counter = 0;
            while(true) {
                sleep(1 * 20);
                // if (++counter > 2) {
                //     break;
                // }
            }
        }
    }
    delete chatA;
    chatA = NULL;
}

