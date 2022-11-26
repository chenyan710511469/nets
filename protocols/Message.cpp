/*
 * =====================================================================================
 *
 *       Filename:  Message.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  11/08/2019 07:18:10 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee)
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/Message.h"

namespace protocols
{
Message::Message(const int id)
 : mId(id)
 , mVector(new std::vector<void *>())
{
}

Message::~Message()
{
    if (NULL != mVector) {
        mVector->clear();
        delete mVector;
        mVector = NULL;
    }
}

const int Message::getId()
{
    return mId;
}

std::vector<void *> * Message::getVector()
{
    return mVector;
}
};  // namespace protocols
