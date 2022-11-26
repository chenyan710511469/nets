/*
 * =====================================================================================
 *
 *       Filename:  TcpConfig.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月05日 18时27分46秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef TCP_CLIENT_TCPCONFIG_H
#define TCP_CLIENT_TCPCONFIG_H

namespace tcpclient
{
class TcpConfig
{
public:
    TcpConfig(unsigned int acceptThreadNumber, unsigned int recvThreadNumber, unsigned int epollSize);
    static const unsigned int mAcceptThreadNumber = 2;
    static const unsigned int mRecvThreadNumber = 8;
    static const unsigned int mEpollSize = 20000;
};
};  // tcpclient
#endif // TCP_CLIENT_TCPCONFIG_H

