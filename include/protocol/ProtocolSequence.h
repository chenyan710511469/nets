/*
 * =====================================================================================
 *
 *       Filename:  ProtocolSequence.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/16/2019 05:53:02 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_PROTOCOL_SEQUENCE_H
#define PROTOCOLS_PROTOCOL_SEQUENCE_H

#include <mutex>

#include "protocol/ProtocolData.h"

namespace protocols
{
class ProtocolSequence
{
public:
    ProtocolSequence();
    ~ProtocolSequence();

    ProtocolSequence(const ProtocolSequence & other) = delete;
    ProtocolSequence & operator=(const ProtocolSequence & other) = delete;

    void setSequence(int sequence);
    void setProtocolData(ProtocolData * pData);
    void setIsMaxSequence(bool isMaxSequence);
    void setAckSequence(bool ack_sequence);

    int getSequence();
    ProtocolData * getProtocolData();
    bool getIsMaxSequence();
    bool getAckSequence();

private:
    int                             mSequence;
    ProtocolData                    * mPData;
    bool                            mIsMaxSequence;
    std::mutex                      mMutex;
    bool                            mAckSequence;
};  // ProtocolSequence
};  // protocols

#endif  // PROTOCOLS_PROTOCOL_SEQUENCE_H
