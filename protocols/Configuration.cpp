/*
 * =====================================================================================
 *
 *       Filename:  Configuration.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/06/2019 09:06:45 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/Configuration.h"

#include <unistd.h>

#include <iostream>
#include <exception>
#include <stdexcept>

#include "protocol/code_def.h"

namespace protocols
{
Configuration::Configuration()
 : mUdpclient(new _udpclient())
 , mUdpserver(new _udpserver())
 , mTcpclient(new _tcpclient())
 , mTcpserver(new _tcpserver())
 , mXmlDocument(new tinyxml2::XMLDocument(true, tinyxml2::COLLAPSE_WHITESPACE))
 , mIpVec(new std::vector<_ip *>())
{
}

Configuration::~Configuration()
{
    if(NULL != mUdpclient) {
        delete mUdpclient;
        mUdpclient = NULL;
    }
    if(NULL != mUdpserver) {
        delete mUdpserver;
        mUdpserver = NULL;
    }
    if(NULL != mTcpclient) {
        delete mTcpclient;
        mTcpclient = NULL;
    }
    if(NULL != mTcpserver) {
        delete mTcpserver;
        mTcpserver = NULL;
    }
    if(NULL != mXmlDocument) {
        delete mXmlDocument;
        mXmlDocument = NULL;
    }
    if(NULL != mIpVec) {
        std::vector<_ip *>::iterator it = mIpVec->begin();
        while(mIpVec->end() != (it = mIpVec->begin()))
        {
            delete *it;
            mIpVec->erase(it);
        }
        delete mIpVec;
        mIpVec = NULL;
    }
}

_udpclient *Configuration::getUdpclient()
{
    return mUdpclient;
}

_udpserver *Configuration::getUdpserver()
{
    return mUdpserver;
}

_tcpclient *Configuration::getTcpclient()
{
    return mTcpclient;
}

_tcpserver *Configuration::getTcpserver()
{
    return mTcpserver;
}
bool Configuration::parseXmlConfig(std::string xmlConfigPath)
{
    bool ret = false;
    try
    {
        if(xmlConfigPath.empty()) {
            throw std::runtime_error("xmlConfigPath can't empty.");
        }
        if(ZERO != access(xmlConfigPath.c_str(), R_OK)) {
            throw std::runtime_error(strerror(errno));
        }
        if (tinyxml2::XML_SUCCESS != mXmlDocument->LoadFile(xmlConfigPath.c_str())) {
            throw std::runtime_error("parse xml file failed.");
        }
        tinyxml2::XMLElement *xmlRoot = mXmlDocument->FirstChildElement("configuration");
        if(NULL == xmlRoot) {
            printf("%s is not content.", mXmlConfigPath.c_str());
            return false;
        }
        tinyxml2::XMLElement *xmlUdpClient = xmlRoot->FirstChildElement("udpclient");
        if(NULL != xmlUdpClient) {
            ret = parseUdpClientConfig(xmlUdpClient);
        }
        tinyxml2::XMLElement *xmlUdpServer = xmlRoot->FirstChildElement("udpserver");
        if(NULL != xmlUdpServer) {
            ret = parseUdpServerConfig(xmlUdpServer);
        }
        tinyxml2::XMLElement *xmlTcpClient = xmlRoot->FirstChildElement("tcpclient");
        if(NULL != xmlTcpClient) {
            ret = parseTcpClientConfig(xmlTcpClient);
        }
        tinyxml2::XMLElement *xmlTcpServer = xmlRoot->FirstChildElement("tcpserver");
        if(NULL != xmlTcpServer) {
            ret = parseTcpServerConfig(xmlTcpServer);
        }
    }
    catch(std::exception & ex) {
        std::cout << ex.what() << std::endl;
        throw ex;
    }
    catch(...) {
        throw;
    }
    if(ret) {
        mXmlConfigPath = xmlConfigPath;
    }
    return ret;
}

bool Configuration::parseUdpClientConfig(tinyxml2::XMLElement * xmlUdpClient)
{
    printf("%s:%d\n", __FUNCTION__, __LINE__);
    return parseXml(xmlUdpClient, UDP_CLIENT);
}

bool Configuration::parseUdpServerConfig(tinyxml2::XMLElement * xmlUdpServer)
{
    printf("%s:%d\n", __FUNCTION__, __LINE__);
    return parseXml(xmlUdpServer, UDP_SERVER);
}

bool Configuration::parseTcpClientConfig(tinyxml2::XMLElement * xmlTcpClient)
{
    printf("%s:%d\n", __FUNCTION__, __LINE__);
    return parseXml(xmlTcpClient, TCP_CLIENT);
}

bool Configuration::parseTcpServerConfig(tinyxml2::XMLElement * xmlTcpServer)
{
    printf("%s:%d\n", __FUNCTION__, __LINE__);
    return parseXml(xmlTcpServer, TCP_SERVER);
}

bool Configuration::parseXml(tinyxml2::XMLElement * xmlConfig, SocketType socketType)
{
    printf("%s:%d\n", __FUNCTION__, __LINE__);
    if(NULL == xmlConfig) {
        return false;
    }
    std::map<unsigned short, _ip *> * ipMapUdpclient = mUdpclient->getIpMap();
    std::map<unsigned short, _ip *> * ipMapUdpserver = mUdpserver->getIpMap();
    std::map<unsigned short, _ip *> * ipMapTcpclient = mTcpclient->getIpMap();
    std::map<unsigned short, _ip *> * ipMapTcpserver = mTcpserver->getIpMap();
    tinyxml2::XMLElement * xmlIp = xmlConfig->FirstChildElement("ip");
    if (NULL == xmlIp) {
        printf("ip node must have ip node.\n");
        return false;
    }
    do
    {
        const tinyxml2::XMLAttribute * configGroupId = xmlIp->FindAttribute("configGroupId");
        if(NULL == configGroupId) {
            printf("ip node must have configGroupId attribute.");
            return false;
        }
        const char * confGroupId = configGroupId->Value();
        if(0 >= strlen(confGroupId)) {
            printf("configGroupId can't be empty.");
            return false;
        }
        char * p = (char *)confGroupId;
        while(*p)
        {
            char c = *p;
            if(c < '0' || c > '9') {
                printf("configGroupId must be numbers, and more than 0.");
                return false;
            }
            ++p;
        }
        const tinyxml2::XMLAttribute * name = xmlIp->FindAttribute("name");
        const char * nameValue = NULL;
        if(NULL != name) {
            nameValue = name->Value();
        }
        const char * remoteIp = NULL;
        if(socketType == UDP_CLIENT || socketType == TCP_CLIENT) {
            const tinyxml2::XMLAttribute * remoteIpAddress = xmlIp->FindAttribute("remoteIpAddress");
            if(NULL == remoteIpAddress) {
                printf("ip node must have remoteIpAddress attribute.");
                return false;
            }
            remoteIp = remoteIpAddress->Value();
            if(0 >= strlen(remoteIp)) {
                printf("remoteIpAddress can't be empty.");
                return false;
            }
        }
        const tinyxml2::XMLAttribute * localIpAddress = xmlIp->FindAttribute("localIpAddress");
        if(NULL == localIpAddress) {
            printf("ip node must have localIpAddress attribute.");
            return false;
        }
        const char * localIp = localIpAddress->Value();
        if(0 >= strlen(localIp)) {
            printf("localIp can't be empty.");
            return false;
        }
        const tinyxml2::XMLAttribute * IPv = xmlIp->FindAttribute("IPv");
        if (IPv == NULL) {
            printf("ip node must have IPv attribute, IPv is IPv4(4) or IPv6(6).");
            return false;
        }
        std::string tmpIPv(IPv->Value());
        if (tmpIPv.empty()) {
            printf("IPv can't be empty.");
            return false;
        }
        for (int i = 0; i < tmpIPv.size(); ++i) {
            if (tmpIPv[i] >= 'a' && tmpIPv[i] <= 'z') {
                tmpIPv[i] = tmpIPv[i] - ('a' - 'A');
            }
        }
        std::string ipVersion;
        if (0 == tmpIPv.compare("4") || 0 == tmpIPv.compare("IPV4")) {
            ipVersion = "IPv4";
        }
        else if (0 == tmpIPv.compare("6") || 0 == tmpIPv.compare("IPV6")) {
            ipVersion = "IPv6";
        }
        else {
            printf("IPv attribute, is IPv4 or IPv6. if IPv is 4 or 6, that is OK.");
            return false;
        }
        tinyxml2::XMLElement * xmlPortChild = xmlIp->FirstChildElement("port");
        if(NULL == xmlPortChild) {
            printf("can't have port.");
            return false;
        }
        std::vector<struct _ports> vecPort;
        bool repeat = false;
        tinyxml2::XMLElement * xmlLocalPort = NULL;
        do
        {
            const char * cRemotePort = NULL;
            if(socketType == UDP_CLIENT || socketType == TCP_CLIENT) {
                tinyxml2::XMLElement * xmlRemotePort = xmlPortChild->FirstChildElement("remotePort");
                if(NULL == xmlRemotePort) {
                    printf("remotePort can't be empty.");
                    return false;
                }
                cRemotePort = xmlRemotePort->GetText();
                p = (char *)cRemotePort;
                while(*p)
                {
                    char c = *p;
                    if(c < '0' || c > '9') {
                        printf("remotePort must be numbers, and more than 0.");
                        return false;
                    }
                    ++p;
                }
            }
repeat:
            if (!repeat) {
                xmlLocalPort = xmlPortChild->FirstChildElement("localPort");
            }
            else {
                xmlLocalPort = xmlLocalPort->NextSiblingElement("localPort");
            }
            if(NULL == xmlLocalPort) {
                if (!vecPort.empty()) {
                    xmlPortChild = xmlPortChild->NextSiblingElement("port");
                    continue;
                }
                printf("localPort can't be empty.");
                return false;
            }
            const char * cLocalPort = xmlLocalPort->GetText();
            p = (char *)cLocalPort;
            while(*p)
            {
                char c = *p;
                if(c < '0' || c > '9') {
                    printf("localPort must be numbers, and more than 0.");
                    return false;
                }
                ++p;
            }
            struct _ports port = {.remotePort = 0, .localPort = 0, .started = false};
            if(socketType == UDP_CLIENT || socketType == TCP_CLIENT) {
                port.remotePort = atoi(cRemotePort);
                if(0 >= port.remotePort || 65535 < port.remotePort) {
                    printf("remotePort must be between 1 and 65535.");
                    return false;
                }
            }
            port.localPort = atoi(cLocalPort);
            if(0 >= port.localPort || 65535 < port.localPort) {
                printf("localPort must be between 1 and 65535.");
                return false;
            }
            std::map<unsigned short, _ip *>::iterator itPortMap = ipMapUdpclient->find(port.localPort);
            if(itPortMap != ipMapUdpclient->end()) {
                printf("The localPort(%d) is unique in the configuration file.", port.localPort);
                return false;
            }
            itPortMap = ipMapUdpserver->find(port.localPort);
            if(itPortMap != ipMapUdpserver->end()) {
                printf("The localPort(%d) is unique in the configuration file", port.localPort);
                return false;
            }
            itPortMap = ipMapTcpclient->find(port.localPort);
            if(itPortMap != ipMapTcpclient->end()){
                printf("The localPort(%d) is unique in the configuration file.", port.localPort);
                return false;
            }
            itPortMap = ipMapTcpserver->find(port.localPort);
            if(itPortMap != ipMapTcpserver->end()) {
                printf("The localPort(%d) is unique in the configuration file.", port.localPort);
                return false;
            }
            vecPort.push_back(port);
            if (socketType == UDP_SERVER || socketType == TCP_SERVER) {
                repeat = true;
                goto repeat;
            }
            xmlPortChild = xmlPortChild->NextSiblingElement("port");
        } while(NULL != xmlPortChild);
        if(vecPort.empty()) {
            printf("remote port and local port of udp client are required.");
            return false;
        }
        _ip * ip = new _ip();
        ip->configGroupId = atoi(confGroupId);
        if(NULL != nameValue) {
            ip->name = nameValue;
        }
        if(socketType == UDP_CLIENT || socketType == TCP_CLIENT) {
            ip->remoteIpAddress = remoteIp;
        }
        ip->localIpAddress = localIp;
        ip->ipVersion = ipVersion;
        std::vector<struct _ports>::iterator itPortVec = vecPort.begin();
        for(; itPortVec != vecPort.end(); ++itPortVec) {
            struct _ports port = *itPortVec;
            switch(socketType)
            {
                case UDP_CLIENT:
                    ipMapUdpclient->insert(std::pair<unsigned short, _ip *>(port.localPort, ip));
                    break;
                case UDP_SERVER:
                    ipMapUdpserver->insert(std::pair<unsigned short, _ip *>(port.localPort, ip));
                    break;
                case TCP_CLIENT:
                    ipMapTcpclient->insert(std::pair<unsigned short, _ip *>(port.localPort, ip));
                    break;
                case TCP_SERVER:
                    ipMapTcpserver->insert(std::pair<unsigned short, _ip *>(port.localPort, ip));
                    break;
                default:
                    return false;
            }
        }
        ip->ports = std::move(vecPort);
        mIpVec->push_back(ip);
    } while(NULL != (xmlIp = xmlIp->NextSiblingElement()));
    return true;
}

_udpclient::_udpclient()
{
}

_udpclient::~_udpclient()
{
}

_udpserver::_udpserver()
{
}

_udpserver::~_udpserver()
{
}

_tcpclient::_tcpclient()
{
}

_tcpclient::~_tcpclient()
{
}

_tcpserver::_tcpserver()
{
}

_tcpserver::~_tcpserver()
{
}

};
