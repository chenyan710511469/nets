/*
 * =====================================================================================
 *
 *       Filename:  MessageBase.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  11/08/2019 07:01:09 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author: 陈焱(Lee)
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_MESSAGE_BASE_H
#define PROTOCOLS_MESSAGE_BASE_H
#include <vector>

namespace protocols
{
class MessageBase
{
public:
    MessageBase() {}
    virtual ~MessageBase() {}

    virtual const int getId() = 0;
    virtual std::vector<void *> * getVector() = 0;

};  // MessageBase
};  // namespace protocols
#endif  // PROTOCOLS_MESSAGE_BASE_H
