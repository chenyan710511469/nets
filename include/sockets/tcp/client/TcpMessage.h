/*
 * =====================================================================================
 *
 *       Filename:  TcpMessage.h
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
#ifndef TCP_CLIENT_TCPMESSAGE_H
#define TCP_CLIENT_TCPMESSAGE_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <mutex>

using namespace std;

namespace tcpclient
{
class TcpMessage
{
public:
    TcpMessage(int socketFd);
    ~TcpMessage();

    TcpMessage(const TcpMessage & other);
    TcpMessage & operator=(const TcpMessage & other);

    size_t getDataSize();
    char * getData();
    bool setData(const char * data, const size_t dataSize);

    int getSocketFd();
    bool append(const char * data, const size_t dataSize);

private:
    int             mSocketFd;
    char            * mTcpIoData;
    size_t          mTcpIoDataSize;
    std::mutex      mMutex;
};
};  // namespace tcpclient
#endif // TCP_CLIENT_TCPMESSAGE_H

