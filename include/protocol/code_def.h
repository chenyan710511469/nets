/*
 * =====================================================================================
 *
 *       Filename:  code_def.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  05/22/2019 07:11:27 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱(Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef PROTOCOLS_CODE_DEF_H
#define PROTOCOLS_CODE_DEF_H

// define protocol: "length,group,sequence,code,protocol,type,data_total_length,data_length{data}"

#define MINUS_THREE                         (-3)
#define MINUS_TWO                           (-2)
#define MINUS_ONE                           (-1)
#define ZERO                                0
#define ONE                                 1
#define TWO                                 2
#define THREE                               3
#define FOUR                                4
#define FIVE                                5
#define SIX                                 6
#define SEVEN                               7
#define EIGHT                               8
#define NINE                                9
#define TEN                                 10
#define ELEVEN                              11
#define TWELVE                              12
#define THIRTEEN                            13
#define FOURTEEN                            14
#define FIFTEEN                             15
#define SIXTEEN                             16
#define SEVENTEEN                           17
#define EIGHTEEN                            18
#define NINETEEN                            19
#define TWENTY                              20

#define FIFTY                               50
#define SIXTY                               60

#define SIXTY_FOUR                          64

#define ONE_HUNDRED_AND_TWENTY              120

#define ONE_HUNDRED_AND_TWENTY_EIGHT        128

#define TWO_HUNDRED                         200
#define TWO_HUNDRED_AND_ONE                 201
#define TWO_HUNDRED_AND_TWO                 202
#define TWO_HUNDRED_AND_THREE               203
#define TWO_HUNDRED_AND_FOUR                204
#define TWO_HUNDRED_AND_FIVE                205
#define TWO_HUNDRED_AND_SIX                 206
#define TWO_HUNDRED_AND_SEVEN               207
#define TWO_HUNDRED_AND_EIGHT               208
#define TWO_HUNDRED_AND_NINE                209
#define TWO_HUNDRED_AND_TEN                 210
#define TWO_HUNDRED_AND_ELEVEN              211
#define TWO_HUNDRED_AND_TWELVE              212
#define TWO_HUNDRED_AND_THIRTEEN            213
#define TWO_HUNDRED_AND_FOURTEEN            214
#define TWO_HUNDRED_AND_FIFTEEN             215
#define TWO_HUNDRED_AND_SIXTEEN             216

#define TWO_HUNDRED_AND_FIFTY_SIX           256

#define FIVE_HUNDRED                        500
#define FIVE_HUNDRED_AND_TWELVE             512

#define ONE_THOUSAND                        1000

#define ONE_THOUSAND_AND_TWO_HUNDRED        1200

#define ONE_MILLION                         1000000

/****start****************client send data to server*****************/
#define SERVER_SEQUENCE_DATA                            ZERO
#define SERVER_REPLY_GROUP                              ONE
#define SERVER_ACK_GROUP                                TWO
#define SERVER_END_SEQUENCE                             THREE
#define SERVER_ACK_SEQUENCE                             FOUR
#define SERVER_REQUEST_SEQUENCE_DATA                    FIVE
#define SERVER_REPLACE_WITH_SEQUENCE_DATA               SIX

// assemble complete sequence data.
#define SERVER_ASSEMBLE_COMPLETE                        SEVEN
#define SERVER_ACK_ASSEMBLE_COMPLETE                    EIGHT
#define SERVER_ACK_COMPLETED                            NINE
#define SERVER_GROUP_NOT_EXIST                          TEN
#define SERVER_REQUEST_GROUP                            ELEVEN

// when has redundant group, the group should be erased.
#define SERVER_REDUNDANT_GROUP                          TWELVE
#define SERVER_RECONNECTED_PEER                         THIRTEEN
#define SERVER_CLOSE_PEER                               FOURTEEN
#define SERVER_CLOSE_PEER_OVER                          FIFTEEN
#define SERVER_CLOSE_PEER_OVER_ACK                      SIXTEEN
/****end******************client send data to server*****************/


/****start****************server send data to client*****************/
#define CLIENT_SEQUENCE_DATA                            TWO_HUNDRED
#define CLIENT_REPLY_GROUP                              TWO_HUNDRED_AND_ONE
#define CLIENT_ACK_GROUP                                TWO_HUNDRED_AND_TWO
#define CLIENT_END_SEQUENCE                             TWO_HUNDRED_AND_THREE
#define CLIENT_ACK_SEQUENCE                             TWO_HUNDRED_AND_FOUR
#define CLIENT_REQUEST_SEQUENCE_DATA                    TWO_HUNDRED_AND_FIVE
#define CLIENT_REPLACE_WITH_SEQUENCE_DATA               TWO_HUNDRED_AND_SIX

// assemble complete sequence data.
#define CLIENT_ASSEMBLE_COMPLETE                        TWO_HUNDRED_AND_SEVEN
#define CLIENT_ACK_ASSEMBLE_COMPLETE                    TWO_HUNDRED_AND_EIGHT
#define CLIENT_ACK_COMPLETED                            TWO_HUNDRED_AND_NINE
#define CLIENT_GROUP_NOT_EXIST                          TWO_HUNDRED_AND_TEN
#define CLIENT_REQUEST_GROUP                            TWO_HUNDRED_AND_ELEVEN

// when has redundant group, the group should be erased.
#define CLIENT_REDUNDANT_GROUP                          TWO_HUNDRED_AND_TWELVE
#define CLIENT_RECONNECTED_PEER_ACK                     TWO_HUNDRED_AND_THIRTEEN
#define CLIENT_CLOSE_PEER                               TWO_HUNDRED_AND_FOURTEEN
#define CLIENT_CLOSE_PEER_OVER                          TWO_HUNDRED_AND_FIFTEEN
#define CLIENT_CLOSE_PEER_OVER_ACK                      TWO_HUNDRED_AND_SIXTEEN
/****end******************server send data to client*****************/

#endif  // PROTOCOLS_CODE_DEF_H

