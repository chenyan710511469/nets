/*
 * =====================================================================================
 *
 *       Filename:  SenderBase.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/05/2019 07:41:53 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_SENDERBASE_H
#define PROTOCOLS_SENDERBASE_H

#include "json/json.h"
#include "tinyxml2/tinyxml2.h"
#include "protocol/protocol_def.h"
#include "sockets/AddressInfo.h"

namespace protocols
{
class SenderBase
{
public:
    virtual ~SenderBase() {}
    virtual bool send(tinyxml2::XMLDocument * xmlDocument, void * upperPointer) = 0;
    virtual bool send(const char * buffer, size_t bufferSize, void * upperPointer) = 0;
    virtual bool send(Json::Value * rootValue, void * upperPointer) = 0;
    virtual SocketType getSocketType() = 0;
    virtual int getConfigGroupId() = 0;
    virtual sockets::AddressInfo * getAddressInfo() = 0;

protected:
    SenderBase() {}

private:
    SenderBase(const SenderBase & other) = delete;
    SenderBase & operator=(const SenderBase & other) = delete;

};  // class SenderBase
};  // namespace protocols
#endif  // PROTOCOLS_SENDERBASE_H
