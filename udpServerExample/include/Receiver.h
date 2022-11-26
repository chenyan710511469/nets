/*
 * =====================================================================================
 *
 *       Filename:  Receiver.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  12/31/2019 06:43:05 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee)
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef RECEIVER_H
#define RECEIVER_H

#include "json/json.h"
#include "tinyxml2/tinyxml2.h"

#include "protocol/SenderBase.h"
#include "protocol/ReceiverBase.h"

class Receiver : public protocols::ReceiverBase
{
public:
    Receiver();
    ~Receiver();

private:
    void receive(tinyxml2::XMLDocument * xmlDocument, protocols::SenderBase * sender);
    void receive(char * buffer, size_t bufferSize, protocols::SenderBase * sender);
    void receive(Json::Value * rootValue, protocols::SenderBase * sender);
    void destroySender(protocols::SenderBase * sender);
    void connected(protocols::SenderBase * sender);
    void disconnected(protocols::SenderBase * sender);

};  // class Receiver

#endif  // RECEIVER_H

