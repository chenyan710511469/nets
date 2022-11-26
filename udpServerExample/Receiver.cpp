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

#include <stdio.h>
#include <stdlib.h>

Receiver::Receiver()
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
    printf("%ld:\n%s\n", bufferSize, buffer);
    sender->send(buffer, bufferSize);
}

void Receiver::receive(Json::Value * rootValue, protocols::SenderBase * sender)
{
}

void Receiver::destroySender(protocols::SenderBase * sender)
{
}

void Receiver::connected(protocols::SenderBase * sender)
{
}

void Receiver::disconnected(protocols::SenderBase * sender)
{
}

