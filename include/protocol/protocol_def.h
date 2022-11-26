/*
 * =====================================================================================
 *
 *       Filename:  protocol_def.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  05/11/2019 10:56:21 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_PROTOCOL_DEF_H
#define PROTOCOLS_PROTOCOL_DEF_H

#include <time.h>
#include <string>

#include "protocol/code_def.h"

// invalid data.
#define TYPE_INVALID                        0

// json data.
#define TYPE_JSON                           1

// xml data.
#define TYPE_XML                            2

// text data
#define TYPE_TEXT                           3

#define REQUEST_GROUP_ID                    MINUS_ONE

namespace protocols
{
enum SocketType
{
    SOCKET_MIN = 0,
    UDP_SERVER = 1,
    UDP_CLIENT = 2,
    TCP_SERVER = 3,
    TCP_CLIENT = 4,
    SOCKET_MAX,
};

};  // protocols

#endif  // PROTOCOLS_PROTOCOL_DEF_H

