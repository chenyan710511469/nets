/*
 * =====================================================================================
 *
 *       Filename:  ChatB.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  12/31/2019 06:38:30 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee)
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef CHATB_H
#define CHATB_H

#include <map>
#include <mutex>
#include <string>

#include "Receiver.h"

#include "protocol/NetworkManager.h"
#include "protocol/NetworkService.h"
#include "protocol/SenderBase.h"
#include "protocol/Configuration.h"

class Receiver;
class ChatD
{
public:
    ChatD();
    ~ChatD();

    bool preStart(std::string & xmlConfigPath);
    bool start();
    void sendMsg();

    void destroySender(protocols::SenderBase * sender);

private:
    ChatD(const ChatD & other) = delete;
    ChatD & operator=(const ChatD & other) = delete;

private:
    Receiver *                                              mReceiver;
    protocols::NetworkManager *                             mNetManager;
    protocols::Configuration *                              mConf;
    std::map<unsigned short, protocols::_ip *> *            mIpMap;
    std::map<unsigned short, protocols::SenderBase *>       mMapSender;
    std::mutex                                              mMutexSender;
};  // class ChatB

#endif  // CHATB_H
