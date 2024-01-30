/*
 * =====================================================================================
 *
 *       Filename:  ChatA.h
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

#include <string>

#include "Receiver.h"

#include "protocol/NetworkManager.h"
#include "protocol/NetworkService.h"
#include "protocol/SenderBase.h"

class ChatA
{
public:
    ChatA();
    ~ChatA();

    bool preStart(std::string & xmlConfigPath);
    bool start();

private:
    ChatA(const ChatA & other) = delete;
    ChatA & operator=(const ChatA & other) = delete;

private:
    Receiver *                      mReceiver;
    protocols::NetworkManager *     mNetManager;
};  // class ChatA
