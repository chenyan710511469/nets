/*
 * =====================================================================================
 *
 *       Filename:  ProtocolData.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/07/2019 09:31:45 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_PROTOCOL_DATA_H
#define PROTOCOLS_PROTOCOL_DATA_H

#include <mutex>
#include <string>

#include "code_def.h"

namespace protocols
{
class ProtocolData
{
public:
    ProtocolData();
    ~ProtocolData();
    int getLength();
    std::string getRemoteIp();
    unsigned short getRemotePort();
    int getGroup();
    int getSequence();
    int getCode();
    std::string getProtocol();
    int getType();
    int getDataTotalLength();
    int getDataLength();
    char * getData();

    void setLength(int length);
    void setRemoteIp(std::string remoteIp);
    void setRemotePort(unsigned short remotePort);
    void setGroup(int group);
    void setSequence(int sequence);
    void setCode(int code);
    void setProtocol(std::string _protocol);
    void setType(int type);
    void setDataTotalLength(int dataTotalLength);
    void setData(char * data, int dataLength);
    struct timespec & getTime();

    ProtocolData(const ProtocolData & other) = delete;
    ProtocolData & operator=(const ProtocolData & other) = delete;

private:
    int                 mLength;
    std::string         mRemoteIp;
    unsigned short      mRemotePort;
    int                 mGroup;
    int                 mSequence;
    int                 mCode;
    std::string         mProtocol;
    int                 mType;
    int                 mDataTotalLength;
    int                 mDataLength;
    char                * mData;
    std::mutex          mMutexData;
    struct timespec     mTime;
};
};
#endif  // PROTOCOLS_PROTOCOL_DATA_H

