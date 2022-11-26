/*
 * =====================================================================================
 *
 *       Filename:  UdpMessage.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2016年08月09日 10时40分49秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱
 *   Organization:
 *
 * =====================================================================================
 */

#include "sockets/udp/server/UdpMessage.h"

namespace udpserver
{
UdpMessage::UdpMessage(int socketFd) : mSocketFd(socketFd)
 , mUdpIoData(NULL)
 , mUdpIoDataSize(0)
{
}

UdpMessage::~UdpMessage()
{
    if(NULL != mUdpIoData)
    {
        free(mUdpIoData);
        mUdpIoData = NULL;
    }
    mUdpIoDataSize = 0;
}

size_t UdpMessage::getDataSize()
{
    return mUdpIoDataSize;
}

char * UdpMessage::getData()
{
    return mUdpIoData;
}

bool UdpMessage::setData(const char * data, const size_t dataSize)
{
    if(!checkData(data, dataSize))
    {printf("%s:%d\ndata = %s, dataSize = %ld\n", __FILE__, __LINE__, data, dataSize);
        return false;
    }
    if(NULL != mUdpIoData)
    {printf("%s:%d\ndata = %s, dataSize = %ld\n", __FILE__, __LINE__, data, dataSize);
        free(mUdpIoData);
        mUdpIoData = NULL;
    }
    if(NULL == data)
    {printf("%s:%d\ndata = %s, dataSize = %ld\n", __FILE__, __LINE__, data, dataSize);
        mUdpIoData = NULL;
        mUdpIoDataSize = 0;
        return true;
    }
MallocUdpIoData1:
    mUdpIoData = (char *)malloc(dataSize);
    if(NULL == mUdpIoData)
    {
        usleep(1000);
        goto MallocUdpIoData1;
    }
    memset(mUdpIoData, 0, dataSize);
    memcpy(mUdpIoData, data, dataSize);
    mUdpIoDataSize = dataSize;
    return true;
}

int UdpMessage::getSocketFd()
{
    return mSocketFd;
}

UdpMessage::UdpMessage(const UdpMessage & other) : mSocketFd(0)
 , mUdpIoData(NULL)
 , mUdpIoDataSize(0)
{
    if(this->setData(other.mUdpIoData, other.mUdpIoDataSize))
    {
        this->mSocketFd = other.mSocketFd;
    }
    else
    {
        //throw std::runtime_error("call UdpMessage::UdpMessage(const UdpMessage & other) error.");
    }
}

UdpMessage & UdpMessage::operator=(const UdpMessage & other)
{
    if(this == &other)
    {printf("%s:%d\n", __FILE__, __LINE__);
        return *this;
    }
    if(this->setData(other.mUdpIoData, other.mUdpIoDataSize))
    {
        this->mSocketFd = other.mSocketFd;
    }
    else
    {
        //throw std::runtime_error("call UdpMessage::UdpMessage(const UdpMessage & other) error.");
    }
    return *this;
}

bool UdpMessage::append(const char * data, const size_t dataSize)
{
    if(!checkData(data, dataSize))
    {
        return false;
    }
    if(NULL == data && 0 == dataSize)
    {
        return false;
    }
    void * aSwapData = NULL;
    if(NULL != mUdpIoData)
    {
MallocSwapData:
        aSwapData = malloc(mUdpIoDataSize);
        if(NULL == aSwapData)
        {
            usleep(1000);
            goto MallocSwapData;
        }
        memset(aSwapData, 0, mUdpIoDataSize);
        memcpy(aSwapData, mUdpIoData, mUdpIoDataSize);
        free(mUdpIoData);
        mUdpIoData = NULL;
    }
MallocUdpIoData2:
    mUdpIoData = (char *)malloc(mUdpIoDataSize + dataSize);
    if(NULL == mUdpIoData)
    {
        usleep(1000);
        goto MallocUdpIoData2;
    }
    memset(mUdpIoData, 0, mUdpIoDataSize + dataSize);
    if(NULL != aSwapData)
    {
        memcpy(mUdpIoData, aSwapData, mUdpIoDataSize);
        free(aSwapData);
        aSwapData = NULL;
    }
    memcpy(mUdpIoData + mUdpIoDataSize, data, dataSize);
    mUdpIoDataSize += dataSize;

    return true;
}

bool UdpMessage::checkData(const char * data, const size_t dataSize)
{
    if(0 > dataSize)
    {printf("%s:%d\n", __FILE__, __LINE__);
        return false;
    }
    if(0 == dataSize && NULL != data)
    {printf("%s:%d\n", __FILE__, __LINE__);
        return false;
    }
    if(NULL == data && 0 < dataSize)
    {printf("%s:%d\n", __FILE__, __LINE__);
        return false;
    }
    return true;
}

};  // udpserver

