/*
 * =====================================================================================
 *
 *       Filename:  Configuration.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/06/2019 08:24:42 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_CONFIGURATION_H
#define PROTOCOLS_CONFIGURATION_H

#include <unistd.h>

#include <string>
#include <vector>
#include <map>

#include "tinyxml2/tinyxml2.h"
#include "protocol/protocol_def.h"

namespace protocols
{
struct _ports
{
    unsigned short remotePort;
    unsigned short localPort;
    bool           started;
};
class _ip
{
public:
    _ip(){}
    ~_ip(){}

    int configGroupId;
    std::string name;
    std::string remoteIpAddress;
    std::string localIpAddress;
    std::string ipVersion;
    std::vector<struct _ports> ports;
};
class _udpclient
{
public:
    _udpclient();
    ~_udpclient();

    std::map<unsigned short, _ip *> * getIpMap()
    {
        return &mIpMap;
    }
private:
    // key = localPort
    std::map<unsigned short, _ip *> mIpMap;
};
class _udpserver
{
public:
    _udpserver();
    ~_udpserver();

    std::map<unsigned short, _ip *> * getIpMap()
    {
        return &mIpMap;
    }
private:
    // key = localPort
    std::map<unsigned short, _ip *> mIpMap;
};
class _tcpclient
{
public:
    _tcpclient();
    ~_tcpclient();

    std::map<unsigned short, _ip *> * getIpMap()
    {
        return &mIpMap;
    }
private:
    // key = localPort
    std::map<unsigned short, _ip *> mIpMap;
};
class _tcpserver
{
public:
    _tcpserver();
    ~_tcpserver();

    std::map<unsigned short, _ip *> * getIpMap()
    {
        return &mIpMap;
    }
private:
    // key = localPort
    std::map<unsigned short, _ip *> mIpMap;
};
class Configuration
{
public:
    Configuration();
    ~Configuration();
    bool parseXmlConfig(std::string xmlConfigPath);

    _udpclient * getUdpclient();
    _udpserver * getUdpserver();
    _tcpclient * getTcpclient();
    _tcpserver * getTcpserver();
    std::string & getXmlConfigPath() { return mXmlConfigPath; }

private:
    bool parseUdpClientConfig(tinyxml2::XMLElement * xmlUdpClient);
    bool parseUdpServerConfig(tinyxml2::XMLElement * xmlUdpServer);
    bool parseTcpClientConfig(tinyxml2::XMLElement * xmlTcpClient);
    bool parseTcpServerConfig(tinyxml2::XMLElement * xmlTcpServer);

    bool parseXml(tinyxml2::XMLElement * xmlConfig, SocketType socketType);

private:
    _udpclient                  * mUdpclient;
    _udpserver                  * mUdpserver;
    _tcpclient                  * mTcpclient;
    _tcpserver                  * mTcpserver;

    std::string                 mXmlConfigPath;
    tinyxml2::XMLDocument       * mXmlDocument;
    std::vector<_ip *>          * mIpVec;
};
};  // end protocols

#endif  // PROTOCOLS_CONFIGURATION_H
