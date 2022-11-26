/*
 * =====================================================================================
 *
 *       Filename:  ProtocolData.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  05/12/2019 06:50:35 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <string.h>
#include "protocol/ProtocolData.h"

namespace protocols
{
ProtocolData::ProtocolData()
 : mLength(0)
 , mRemotePort(0)
 , mGroup(0)
 , mType(0)
 , mSequence(0)
 , mCode(0)
 , mDataTotalLength(0)
 , mDataLength(0)
 , mData(NULL)
{
    clock_gettime(CLOCK_BOOTTIME, &mTime);
}
ProtocolData::~ProtocolData()
{
    if(mData != NULL)
    {
        mMutexData.lock();
        free(mData);
        mData = NULL;
        mDataLength = 0;
        mMutexData.unlock();
    }
}

int ProtocolData::getLength()
{
    return mLength;
}

std::string ProtocolData::getRemoteIp() {
    return mRemoteIp;
}

unsigned short ProtocolData::getRemotePort() {
    return mRemotePort;
}

int ProtocolData::getGroup()
{
    return mGroup;
}

int ProtocolData::getType()
{
    return mType;
}

int ProtocolData::getSequence()
{
    return mSequence;
}

int ProtocolData::getCode()
{
    return mCode;
}

int ProtocolData::getDataTotalLength()
{
    return mDataTotalLength;
}

int ProtocolData::getDataLength()
{
    return mDataLength;
}

char *ProtocolData::getData()
{
    mMutexData.lock();
    char * tmpData = mData;
    mMutexData.unlock();
    return tmpData;
}

void ProtocolData::setLength(int length)
{
    mLength = length;
}

void ProtocolData::setRemoteIp(std::string remoteIp)
{
    mRemoteIp = remoteIp;
}

void ProtocolData::setRemotePort(unsigned short remotePort)
{
    mRemotePort = remotePort;
}

void ProtocolData::setGroup(int group)
{
    mGroup = group;
}

void ProtocolData::setType(int type)
{
    mType = type;
}

void ProtocolData::setSequence(int sequence)
{
    mSequence = sequence;
}

void ProtocolData::setCode(int code)
{
    mCode = code;
}

void ProtocolData::setDataTotalLength(int dataTotalLength)
{
    mDataTotalLength = dataTotalLength;
}

void ProtocolData::setData(char * data, int dataLength)
{
    if(data == NULL || 0 >= dataLength) {
        return;
    }
    mMutexData.lock();
    if(mData == NULL) {
        while(NULL == (mData = (char*)malloc(dataLength + ONE)));
        memset(mData, 0, dataLength + ONE);
        memcpy((void*)mData, (void*)data, dataLength);
        mDataLength = dataLength;
    }
    else {
        char * tmpData = NULL;
        while(NULL == (tmpData = (char*)malloc(mDataLength + ONE)));
        memset(tmpData, 0, mDataLength + ONE);
        memcpy(tmpData, mData, mDataLength);
        free(mData);
        mData = NULL;
        while(NULL == (mData = (char*)malloc(mDataLength + dataLength + ONE)));
        memset(mData, 0, mDataLength + dataLength + ONE);
        memcpy(mData, tmpData, mDataLength);
        memcpy(mData + mDataLength, data, dataLength);
        mDataLength += dataLength;
    }
    mMutexData.unlock();
}

void ProtocolData::setProtocol(std::string _protocol)
{
    mProtocol = _protocol;
}

std::string ProtocolData::getProtocol()
{
    return mProtocol;
}

struct timespec & ProtocolData::getTime()
{
    return mTime;
}

};
