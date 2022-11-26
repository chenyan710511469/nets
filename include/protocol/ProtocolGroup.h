/*
 * =====================================================================================
 *
 *       Filename:  ProtocolGroup.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/16/2019 05:50:42 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_PROTOCOL_GROUP_H
#define PROTOCOLS_PROTOCOL_GROUP_H

#include <time.h>
#include <string.h>

#include <map>
#include <mutex>

#include "ProtocolSequence.h"
#include "protocol/code_def.h"

namespace protocols
{
class ProtocolGroup
{
public:
    ProtocolGroup();
    ~ProtocolGroup();

    ProtocolGroup(const ProtocolGroup & other) = delete;
    ProtocolGroup & operator=(const ProtocolGroup & other) = delete;

    void setGroupId(int groupServer);
    void setGroupClient(std::string groupClient);
    void setAssemble(bool assemble);
    void setAssembleComplete(bool assembleComplete);
    bool addMapSequence(std::string & key, ProtocolSequence * pSequence);
    void setAckGroup(bool ackGroup);

    int                     getGroupId();
    std::string &           getGroupClient();
    bool                    getAssemble();
    bool                    getAssembleComplete();
    ProtocolSequence *      getSequence(std::string & key);
    bool                    getAckGroup();
    bool                    isTimeout(long timeInterval);
private:
    int                                         mGroupId;
    std::string                                 mGroupClient;
    bool                                        mAssemble;
    bool                                        mAssembleComplete;
    std::map<std::string, ProtocolSequence *>   mMapSequence;
    std::mutex                                  mMutexSequence;
    bool                                        mAckGroup;
    struct timespec                             mTime;
};  // ProtocolGroup
};  // protocols

#endif  // PROTOCOLS_PROTOCOL_GROUP_H
