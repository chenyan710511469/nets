/*
 * =====================================================================================
 *
 *       Filename:  TcpMessage.cpp
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

#include "sockets/tcp/server/TcpMessage.h"

#include <thread>

namespace tcpserver
{
TcpMessage::TcpMessage(int socketFd) : mSocketFd(socketFd)
 , mTcpIoData(NULL)
 , mTcpIoDataSize(0) {
}

TcpMessage::~TcpMessage() {
    mMutex.lock();
    if(NULL != mTcpIoData) {
        free(mTcpIoData);
        mTcpIoData = NULL;
    }
    mTcpIoDataSize = 0;
    mMutex.unlock();
}

size_t TcpMessage::getDataSize() {
    mMutex.lock();
    size_t dataSize = mTcpIoDataSize;
    mMutex.unlock();
    return dataSize;
}

char * TcpMessage::getData() {
    mMutex.lock();
    char * data = mTcpIoData;
    mMutex.unlock();
    return data;
}

bool TcpMessage::setData(const char * data, const size_t dataSize) {
    mMutex.lock();
    if(NULL != mTcpIoData) {
        free(mTcpIoData);
        mTcpIoData = NULL;
    }
    while (NULL == (mTcpIoData = (char *)malloc(dataSize + 1))) {
        std::cout << __FUNCTION__ << ":" <<  __LINE__ << ", thread id: " << std::this_thread::get_id() << std::endl;
    }
    memset(mTcpIoData, 0, dataSize + 1);
    memcpy(mTcpIoData, data, dataSize);
    mTcpIoDataSize = dataSize;
    mMutex.unlock();
    return true;
}

int TcpMessage::getSocketFd() {
    return mSocketFd;
}

TcpMessage::TcpMessage(const TcpMessage & other)
 : mSocketFd(0)
 , mTcpIoData(NULL)
 , mTcpIoDataSize(0) {
    if(this->setData(other.mTcpIoData, other.mTcpIoDataSize)) {
        this->mSocketFd = other.mSocketFd;
    }
    else {
        //throw std::runtime_error("call TcpMessage::TcpMessage(const TcpMessage & other) error.");
    }
}

TcpMessage & TcpMessage::operator=(const TcpMessage & other) {
    if(this == &other) {
        return *this;
    }
    if(this->setData(other.mTcpIoData, other.mTcpIoDataSize)) {
        this->mSocketFd = other.mSocketFd;
    }
    else {
        //throw std::runtime_error("call TcpMessage::TcpMessage(const TcpMessage & other) error.");
    }
    return *this;
}

bool TcpMessage::append(const char * data, const size_t dataSize) {
    mMutex.lock();
    void * aSwapData = NULL;
    if(NULL != mTcpIoData) {
        while (NULL == (aSwapData = malloc(mTcpIoDataSize + 1)));
        memset(aSwapData, 0, mTcpIoDataSize + 1);
        memcpy(aSwapData, mTcpIoData, mTcpIoDataSize);
        free(mTcpIoData);
        mTcpIoData = NULL;
    }
    while (NULL == (mTcpIoData = (char *)malloc(mTcpIoDataSize + dataSize + 1)));
    memset(mTcpIoData, 0, mTcpIoDataSize + dataSize + 1);
    if(NULL != aSwapData) {
        memcpy(mTcpIoData, aSwapData, mTcpIoDataSize);
        free(aSwapData);
        aSwapData = NULL;
    }
    memcpy(mTcpIoData + mTcpIoDataSize, data, dataSize);
    mTcpIoDataSize += dataSize;
    mMutex.unlock();
    return true;
}

};  // namespace tcpserver

