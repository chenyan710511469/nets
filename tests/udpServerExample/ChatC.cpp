/*
 * =====================================================================================
 *
 *       Filename:  ChatC.cpp
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
#include "ChatC.h"

#include <stdio.h>
#include <stdlib.h>

#include <exception>

ChatC::ChatC()
 : mReceiver(new Receiver())
 , mNetManager(NULL)
{
}

ChatC::~ChatC()
{
    if (NULL != mNetManager) {
        delete mNetManager;
        mNetManager = NULL;
    }
    delete mReceiver;
    mReceiver = NULL;
}

bool ChatC::preStart(std::string & xmlConfigPath)
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

bool ChatC::start()
{
    try {
        return mNetManager->startUdpServer();
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
    std::string xmlConfigPath("/home/chenyan/projects/project1/chatC/configuration.xml");
    ChatC * chatC = new ChatC();
    if (chatC->preStart(xmlConfigPath)) {
        if (chatC->start()) {
            int counter = 0;
            while(true) {
                sleep(1 * 20);
                // if (++counter > 2) {
                //     break;
                // }
            }
        }
    }
    delete chatC;
    chatC = NULL;
}

