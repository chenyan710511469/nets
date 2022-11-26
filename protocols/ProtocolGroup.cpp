/*
 * =====================================================================================
 *
 *       Filename:  ProtocolGroup.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/16/2019 05:49:56 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include "protocol/ProtocolGroup.h"


namespace protocols
{
ProtocolGroup::ProtocolGroup()
 : mGroupId(-1)
 , mAssemble(false)
 , mAssembleComplete(false)
 , mAckGroup(false)
{
    if(0 > clock_gettime(CLOCK_BOOTTIME, &mTime)) {
        printf("error:%s. errno: %d\n", strerror(errno), errno);
    }
}

ProtocolGroup::~ProtocolGroup()
{
    mMutexSequence.lock();
    do
    {
        std::map<std::string, ProtocolSequence *>::iterator it = mMapSequence.begin();
        if(it != mMapSequence.end()) {
            delete it->second;
            it->second = NULL;
            mMapSequence.erase(it);
        }
        printf("%s:%d\n", __FILE__, __LINE__);
    } while(0 < mMapSequence.size());
    mMutexSequence.unlock();
}

void ProtocolGroup::setGroupId(int groupId)
{
    mGroupId = groupId;
}

void ProtocolGroup::setAssemble(bool assemble)
{
    mAssemble = assemble;
}

void ProtocolGroup::setAssembleComplete(bool assembleComplete)
{
    mAssembleComplete = assembleComplete;
}

int ProtocolGroup::getGroupId()
{
    return mGroupId;
}

bool ProtocolGroup::getAssemble()
{
    return mAssemble;
}

bool ProtocolGroup::getAssembleComplete()
{
    return mAssembleComplete;
}

void ProtocolGroup::setGroupClient(std::string groupClient)
{
    mGroupClient = groupClient;
}

std::string & ProtocolGroup::getGroupClient()
{
    return mGroupClient;
}

bool ProtocolGroup::addMapSequence(std::string & key, ProtocolSequence * pSequence)
{
    ProtocolData * pData = pSequence->getProtocolData();
    mMutexSequence.lock();
    std::map<std::string, ProtocolSequence *>::iterator it = mMapSequence.find(key);
    if(it == mMapSequence.end()) {
        std::pair<std::map<std::string, ProtocolSequence *>::iterator, bool> ret =
        mMapSequence.insert(std::pair<std::string, ProtocolSequence *>(key, pSequence));
        mMutexSequence.unlock();
        return ret.second;
    } else if(pData->getCode() == SERVER_REPLACE_WITH_SEQUENCE_DATA) {
        delete it->second;
        mMapSequence.erase(it);
        std::pair<std::map<std::string, ProtocolSequence *>::iterator, bool> ret =
        mMapSequence.insert(std::pair<std::string, ProtocolSequence *>(key, pSequence));
        mMutexSequence.unlock();
        return ret.second;
    }
    mMutexSequence.unlock();
    return false;  // not insert into mMapSequence.
}

ProtocolSequence * ProtocolGroup::getSequence(std::string & key)
{
    mMutexSequence.lock();
    std::map<std::string, ProtocolSequence *>::iterator it = mMapSequence.find(key);
    if(it == mMapSequence.end()) {
        mMutexSequence.unlock();
        return NULL;
    }
    ProtocolSequence * pSequence = it->second;
    mMutexSequence.unlock();
    return pSequence;
}

void ProtocolGroup::setAckGroup(bool ackGroup)
{
    mAckGroup = ackGroup;
}

bool ProtocolGroup::getAckGroup()
{
    return mAckGroup;
}

bool ProtocolGroup::isTimeout(long timeInterval)
{
    struct timespec nowTime;
    clock_gettime(CLOCK_BOOTTIME, &nowTime);
    return (timeInterval < nowTime.tv_sec - mTime.tv_sec);
}

};  // namespace protocols
