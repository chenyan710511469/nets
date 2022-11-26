/*
 * =====================================================================================
 *
 *       Filename:  Receiver.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/01/2020 08:22:13 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee)
 *   Organization:
 *
 * =====================================================================================
 */
#include "Receiver.h"

Receiver::Receiver(ChatB * chatB)
 : mChatB(chatB)
{
}

Receiver::~Receiver()
{
}

void Receiver::receive(tinyxml2::XMLDocument * xmlDocument, protocols::SenderBase * sender)
{
}

void Receiver::receive(char * buffer, size_t bufferSize, protocols::SenderBase * sender)
{
    struct timespec newTime;
    clock_gettime(CLOCK_BOOTTIME, &newTime);
    printf("%s:%d, tv_sec=%ld, tv_nsec=%ld.\n",
            __FUNCTION__, __LINE__, newTime.tv_sec, newTime.tv_nsec);
    printf("%ld:%s\n", bufferSize, buffer);
    // sender->send(buffer, bufferSize);
}

void Receiver::receive(Json::Value * rootValue, protocols::SenderBase * sender)
{
}

void Receiver::destroySender(protocols::SenderBase * sender)
{
    mChatB->destroySender(sender);
}

void Receiver::connected(protocols::SenderBase * sender)
{
}

void Receiver::disconnected(protocols::SenderBase * sender)
{
}

void Receiver::replyUpper(protocols::SenderBase * sender, bool result, void * upperPointer)
{
    printf("sender=%p, result=%d, upperPointer=%p", sender, result, upperPointer);
}

