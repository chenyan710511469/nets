/*
 * =====================================================================================
 *
 *       Filename:  Message.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  11/08/2019 07:10:46 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee)
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_MESSAGE_H
#define PROTOCOLS_MESSAGE_H
#include <vector>
#include <iostream>
#include "protocol/MessageBase.h"

namespace protocols
{
class Message : public MessageBase
{
public:
    Message(const int id);
    ~Message();

private:
    const int getId();
    std::vector<void *> * getVector();

private:
    const int                       mId;
    std::vector<void *>*            mVector;
};  // Message
};  // namespace protocols
#endif  // PROTOCOLS_MESSAGE_H
