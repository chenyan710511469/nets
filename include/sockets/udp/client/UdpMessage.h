/*
 * =====================================================================================
 *
 *       Filename:  UdpMessage.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月09日 10时41分24秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef UDP_CLIENT_UDPMESSAGE_H
#define UDP_CLIENT_UDPMESSAGE_H
#include "UdpIoService.h"
#include <iostream>
#include <string>

using namespace std;

namespace udpclient
{
class UdpMessage
{
public:
    UdpMessage(int socketFd);
    ~UdpMessage();

    UdpMessage(const UdpMessage & other);
    UdpMessage & operator=(const UdpMessage & other);

    size_t getDataSize();
    char * getData();
    bool setData(const char * data, const size_t dataSize);

    int getSocketFd();

    bool append(const char * data, const size_t dataSize);

private:

    bool checkData(const char * theData, const size_t theDataSize);

private:
    int             mSocketFd;
    char            * mUdpIoData;
    size_t          mUdpIoDataSize;
};
};  // udpclient
#endif  // UDP_CLIENT_UDPMESSAGE_H

