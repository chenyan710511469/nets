/*
 * =====================================================================================
 *
 *       Filename:  ReceiverBase.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/27/2019 06:49:27 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_RECEIVER_BASE_H
#define PROTOCOLS_RECEIVER_BASE_H

#include "protocol/SenderBase.h"
#include "tinyxml2/tinyxml2.h"
#include "json/json.h"

namespace protocols
{
class SenderBase;
class ReceiverBase
{
public:
    virtual ~ReceiverBase() {}

    virtual void receive(tinyxml2::XMLDocument * xmlDocument, SenderBase * sender) = 0;
    virtual void receive(char * buffer, size_t bufferSize, SenderBase * sender) = 0;
    virtual void receive(Json::Value * rootValue, SenderBase * sender) = 0;
    virtual void destroySender(SenderBase * sender) = 0;
    virtual void connected(SenderBase * sender) = 0;
    virtual void disconnected(SenderBase * sender) = 0;
    virtual void replyUpper(SenderBase * sender, bool result, void * upperPointer) = 0;

protected:
    ReceiverBase() {}

private:
    ReceiverBase(const ReceiverBase & other) = delete;
    ReceiverBase & operator=(const ReceiverBase & other) = delete;
};  // class ReceiverBase
};  // namespace
#endif  // PROTOCOLS_RECEIVER_BASE_H
