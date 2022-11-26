/*
 * =====================================================================================
 *
 *       Filename:  ProtocolSequence.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/16/2019 05:52:26 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/ProtocolSequence.h"

namespace protocols
{
ProtocolSequence::ProtocolSequence()
 : mSequence(-1)
 , mPData(NULL)
 , mIsMaxSequence(false)
 , mAckSequence(false)
{}

ProtocolSequence::~ProtocolSequence()
{
    mMutex.lock();
    if(NULL != mPData) {
        delete mPData;
        mPData = NULL;
    }
    mMutex.unlock();
}

void ProtocolSequence::setSequence(int sequence)
{
    mSequence = sequence;
}

void ProtocolSequence::setProtocolData(ProtocolData * pData)
{
    mMutex.lock();
    if(NULL != mPData) {
        delete mPData;
        mPData = NULL;
    }
    mPData = pData;
    mMutex.unlock();
}

void ProtocolSequence::setIsMaxSequence(bool isMaxSequence)
{
    mIsMaxSequence = isMaxSequence;
}

int ProtocolSequence::getSequence()
{
    return mSequence;
}

ProtocolData * ProtocolSequence::getProtocolData()
{
    return mPData;
}

bool ProtocolSequence::getIsMaxSequence()
{
    return mIsMaxSequence;
}

void ProtocolSequence::setAckSequence(bool ack_sequence)
{
    mAckSequence = ack_sequence;
}

bool ProtocolSequence::getAckSequence()
{
    return mAckSequence;
}

};  // protocols

