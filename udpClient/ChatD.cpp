/*
 * =====================================================================================
 *
 *       Filename:  ChatD.cpp
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
#include "ChatD.h"

#include <stdio.h>
#include <stdlib.h>

#include <exception>
#include <string>

#include "sockets/AddressInfo.h"

ChatD::ChatD()
 : mReceiver(new Receiver(this))
 , mNetManager(NULL)
 , mConf(NULL) {
}

ChatD::~ChatD() {
    if (NULL != mNetManager) {
        delete mNetManager;
        mNetManager = NULL;
    }
    delete mReceiver;
    mReceiver = NULL;
}

bool ChatD::preStart(std::string & xmlConfigPath)
{
    try {
        mNetManager = new protocols::NetworkManager(xmlConfigPath);
        if (mNetManager->preStart(mReceiver)) {
            mConf = mNetManager->getConfiguration();
            return true;
        }
    }
    catch (std::exception & ex) {
        printf("%s\n", ex.what());
    }
    return false;
}

bool ChatD::start()
{
    if (NULL == mNetManager) {
        return false;
    }
    try {
        mMapSender.clear();
        // bool ret = mNetManager->startTcpClient();
        bool ret = mNetManager->startUdpClient();
        if (ret) {
            mIpMap = mConf->getUdpclient()->getIpMap();
            std::map<unsigned short, protocols::_ip *>::iterator it = mIpMap->begin();
            for (; it != mIpMap->end(); ++it) {
                unsigned short localPort = it->first;
                protocols::_ip * ip = it->second;
                int configGroupId = ip->configGroupId;
                std::string name(ip->name);
                std::string remoteIp(ip->remoteIpAddress);
                std::string localIp(ip->localIpAddress);
                std::vector<struct protocols::_ports> portsVec(ip->ports);
                std::vector<struct protocols::_ports>::iterator it = portsVec.begin();
                for (; it != portsVec.end(); ++it) {
                    protocols::_ports ports(*it);
                    unsigned short remotePort = ports.remotePort;
                    ports.localPort;
                    ports.started;
                    protocols::SenderBase * sender = mNetManager->createSenderForUdpClient(configGroupId, remoteIp, remotePort);
                    mMapSender.insert(std::pair<unsigned short, protocols::SenderBase *>(remotePort, sender));
                    printf("%s:%d\n", __FUNCTION__, __LINE__);
                }
                printf("%s:%d\n", __FUNCTION__, __LINE__);
            }
        }printf("%s:%d\n", __FUNCTION__, __LINE__);
    }
    catch (std::exception & ex) {
        printf("%s\n", ex.what());
    }printf("%s:%d\n", __FUNCTION__, __LINE__);
    return !mMapSender.empty();
}

void ChatD::destroySender(protocols::SenderBase * sender)
{
    sockets::AddressInfo * addressInfo = sender->getAddressInfo();
    unsigned short remotePort = addressInfo->getRemotePort();
    printf("remotePort=%d, sender=%p.\n", remotePort, sender);
    mMutexSender.lock();
    mMapSender.erase(remotePort);
    mMutexSender.unlock();
}

void ChatD::sendMsg() {
    mMutexSender.lock();
    std::map<unsigned short, protocols::SenderBase *>::iterator it = mMapSender.begin();
    struct timespec oldTime;
    clock_gettime(CLOCK_BOOTTIME, &oldTime);
    printf("%s:%d. tv_sec=%ld, tv_nsec=%ld.\n",
            __FUNCTION__, __LINE__, oldTime.tv_sec, oldTime.tv_nsec);
    for (; it != mMapSender.end(); ++it) {
        protocols::SenderBase * sender = it->second;
        char buffer[] = {"ABC aaaaaaaaaaaaaaAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccdddddddddddddddddddddddddddddddddddddddddDDDDDDDDDDDDDDDDDFFFFFFFFFFFFFFFFfffffffffffffffffffffffffffffffffffffffffffffffffffffffgggggggggg的大概个问问；‘发呢我咖啡味儿范围内快来围观哪个艾尔frwagwerogregrwe'爱过你看看raegj'wjnkg个个会计人官方把客人付款更方便热kgVB客服部开始打开vHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMXXXXXXXXOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOXXXXXXXXXXXXXX XYZ"};
        bool ret = sender->send(buffer, strlen(buffer));
        printf("%s %s:%d  ret=%d\n", __FILE__, __FUNCTION__, __LINE__, ret);
        printf("remotePort=%d, sender=%p.\n", it->first, sender);
        // sleep(1);
    }
    mMutexSender.unlock();
}

int main(int argc, char * argv[])
{
    std::string xmlConfigPath("/home/chenyan/projects/project1/udpClient/configuration.xml");
    ChatD * chatB = new ChatD();
    if (chatB->preStart(xmlConfigPath)) {
        if (chatB->start()) {
            int counter = 0;
            while(true) {
                chatB->sendMsg();
                usleep(5000000);
                // if (++counter > 2) {
                //     break;
                // }
            }
        }
    }
    delete chatB;
    chatB = NULL;
}

