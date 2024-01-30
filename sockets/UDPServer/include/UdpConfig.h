/*
 * =====================================================================================
 *
 *       Filename:  UdpConfig.h
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
#ifndef UDPCONFIG_H
#define UDPCONFIG_H

namespace udpserver
{
class UdpConfig
{
public:
    UdpConfig(unsigned int theAcceptThreadNumber, unsigned int theRecvThreadNumber, unsigned int theEpollSize);
    static const unsigned int mAcceptThreadNumber = 2;
    static const unsigned int mRecvThreadNumber = 12;
    static const unsigned int mEpollSize = 200;
};
};  // udpserver
#endif // UDPCONFIG_H

