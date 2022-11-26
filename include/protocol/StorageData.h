/*
 * =====================================================================================
 *
 *       Filename:  StorageData.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  08/24/2019 12:47:21 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_RECEIVED_DATA_H
#define PROTOCOLS_RECEIVED_DATA_H

#include <time.h>
#include <mutex>

#include "protocol/code_def.h"

namespace protocols
{
class StorageData
{
public:
    StorageData(char * data, size_t dataLength, const int dataType)
     : mData(data)
     , mDataLength(dataLength)
     , mDataType(dataType)
     , mEndSequenceDataCounter(ZERO)
     , mMaxSequenceId(MINUS_ONE)
     , mUpperPointer(NULL) {
         clock_gettime(CLOCK_BOOTTIME, &mTime);
         clock_gettime(CLOCK_BOOTTIME, &mStartTimer);
     }
    ~StorageData() {
        mMutexData.lock();
        if (mData != NULL) {
            free(mData);
            mData = NULL;
        }
        mDataLength = ZERO;
        mMutexData.unlock();
    }
    char * getData() {
        mMutexData.lock();
        char * data = mData;
        mMutexData.unlock();
        return data;
    }
    size_t getDataLength() {
        mMutexData.lock();
        size_t dataLength = mDataLength;
        mMutexData.unlock();
        return dataLength;
    }
    const int getDataType() { return mDataType; }

    bool isTimeout(unsigned long long timeInterval) {  // unit is  millisecond.
        if (TEN < ++mEndSequenceDataCounter) {  // allow to repeat send 10 times.
            return false;
        }
        struct timespec newTime;
        memset(&newTime, ZERO, sizeof(struct timespec));
        clock_gettime(CLOCK_BOOTTIME, &newTime);
        unsigned long long newTimeMS = (newTime.tv_sec * ONE_THOUSAND) + (newTime.tv_nsec / ONE_MILLION);
        unsigned long long oldTimeMS = (mTime.tv_sec * ONE_THOUSAND) + (mTime.tv_nsec / ONE_MILLION);
        bool ret = (timeInterval < newTimeMS - oldTimeMS);
        if (ret) {
            clock_gettime(CLOCK_BOOTTIME, &mTime);
        }
        return ret;
    }
    int getEndSequenceDataCounter() {
        return mEndSequenceDataCounter;
    }
    void setGroupSId(std::string groupSId) {
        mGroupSId = groupSId;
    }
    std::string & getGroupSId() {
        return mGroupSId;
    }
    void setMaxSequenceId(int maxSequenceId) {
        mMaxSequenceId = maxSequenceId;
    }
    int getMaxSequenceId() {
        return mMaxSequenceId;
    }
    long long getStartTimer() {  // unit is millisecond.
        return ((mStartTimer.tv_sec * ONE_THOUSAND) + (mStartTimer.tv_nsec / ONE_MILLION));
    }

    void setUpperPointer(void * upperPointer) {
        mUpperPointer = upperPointer;
    }

    void * getUpperPointer() {
        return mUpperPointer;
    }

private:
    char                        * mData;
    size_t                      mDataLength;
    const int                   mDataType;
    std::mutex                  mMutexData;
    struct timespec             mTime;
    struct timespec             mStartTimer;  // record starting time for resend data to peer.
    int                         mEndSequenceDataCounter;
    std::string                 mGroupSId;
    int                         mMaxSequenceId;
    void                        * mUpperPointer;  // upper pointer.
};  // class StorageData
};  // namespace protocols
#endif  // PROTOCOLS_RECEIVED_DATA_H
