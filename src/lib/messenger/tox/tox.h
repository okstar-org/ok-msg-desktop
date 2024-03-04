/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#pragma once

namespace lib {
namespace messenger {
class Messenger;
}
} // namespace lib

using Tox = lib::messenger::Messenger;

typedef enum TOX_MESSAGE_TYPE {

  /**
   * Normal text message. Similar to PRIVMSG on IRC.
   */
  TOX_MESSAGE_TYPE_NORMAL,

  /**
   * A message describing an user action. This is similar to /me (CTCP ACTION)
   * on IRC.
   */
  TOX_MESSAGE_TYPE_ACTION,

} TOX_MESSAGE_TYPE;

// typedef TOX_ERR_OPTIONS_NEW Tox_Err_Options_New;
//  typedef TOX_ERR_NEW Tox_Err_New;
//  typedef TOX_ERR_BOOTSTRAP Tox_Err_Bootstrap;
//  typedef TOX_ERR_SET_INFO Tox_Err_Set_Info;
//  typedef TOX_ERR_FRIEND_ADD Tox_Err_Friend_Add;
//  typedef TOX_ERR_FRIEND_DELETE Tox_Err_Friend_Delete;
//  typedef TOX_ERR_FRIEND_BY_PUBLIC_KEY Tox_Err_Friend_By_Public_Key;
//  typedef TOX_ERR_FRIEND_GET_PUBLIC_KEY Tox_Err_Friend_Get_Public_Key;
//  typedef TOX_ERR_FRIEND_GET_LAST_ONLINE Tox_Err_Friend_Get_Last_Online;
//  typedef TOX_ERR_FRIEND_QUERY Tox_Err_Friend_Query;
//  typedef TOX_ERR_SET_TYPING Tox_Err_Set_Typing;
//  typedef TOX_ERR_FRIEND_SEND_MESSAGE Tox_Err_Friend_Send_Message;
//  typedef TOX_ERR_FILE_CONTROL Tox_Err_File_Control;
//  typedef TOX_ERR_FILE_SEEK Tox_Err_File_Seek;
//  typedef TOX_ERR_FILE_GET Tox_Err_File_Get;
//  typedef TOX_ERR_FILE_SEND Tox_Err_File_Send;
//  typedef TOX_ERR_FILE_SEND_CHUNK Tox_Err_File_Send_Chunk;
//  typedef TOX_ERR_CONFERENCE_NEW Tox_Err_Conference_New;
//  typedef TOX_ERR_CONFERENCE_DELETE Tox_Err_Conference_Delete;

/**
 * Error codes for peer info queries.
 */
typedef enum TOX_ERR_CONFERENCE_PEER_QUERY {

  /**
   * The function returned successfully.
   */
  TOX_ERR_CONFERENCE_PEER_QUERY_OK,

  /**
   * The conference number passed did not designate a valid conference.
   */
  TOX_ERR_CONFERENCE_PEER_QUERY_CONFERENCE_NOT_FOUND,

  /**
   * The peer number passed did not designate a valid peer.
   */
  TOX_ERR_CONFERENCE_PEER_QUERY_PEER_NOT_FOUND,

  /**
   * The client is not connected to the conference.
   */
  TOX_ERR_CONFERENCE_PEER_QUERY_NO_CONNECTION,

} TOX_ERR_CONFERENCE_PEER_QUERY;

typedef TOX_ERR_CONFERENCE_PEER_QUERY Tox_Err_Conference_Peer_Query;

// typedef TOX_ERR_CONFERENCE_SET_MAX_OFFLINE
// Tox_Err_Conference_Set_Max_Offline; typedef TOX_ERR_CONFERENCE_BY_ID
// Tox_Err_Conference_By_Id; typedef TOX_ERR_CONFERENCE_BY_UID
// Tox_Err_Conference_By_Uid; typedef TOX_ERR_CONFERENCE_INVITE
// Tox_Err_Conference_Invite;

typedef enum TOX_ERR_CONFERENCE_JOIN {

  /**
   * The function returned successfully.
   */
  TOX_ERR_CONFERENCE_JOIN_OK,

  /**
   * The cookie passed has an invalid length.
   */
  TOX_ERR_CONFERENCE_JOIN_INVALID_LENGTH,

  /**
   * The conference is not the expected type. This indicates an invalid cookie.
   */
  TOX_ERR_CONFERENCE_JOIN_WRONG_TYPE,

  /**
   * The friend number passed does not designate a valid friend.
   */
  TOX_ERR_CONFERENCE_JOIN_FRIEND_NOT_FOUND,

  /**
   * Client is already in this conference.
   */
  TOX_ERR_CONFERENCE_JOIN_DUPLICATE,

  /**
   * Conference instance failed to initialize.
   */
  TOX_ERR_CONFERENCE_JOIN_INIT_FAIL,

  /**
   * The join packet failed to send.
   */
  TOX_ERR_CONFERENCE_JOIN_FAIL_SEND,

} TOX_ERR_CONFERENCE_JOIN;

typedef TOX_ERR_CONFERENCE_JOIN Tox_Err_Conference_Join;

// typedef TOX_ERR_CONFERENCE_TITLE
// Tox_Err_Conference_Title; typedef TOX_ERR_CONFERENCE_GET_TYPE
// Tox_Err_Conference_Get_Type; typedef TOX_ERR_FRIEND_CUSTOM_PACKET
// Tox_Err_Friend_Custom_Packet; typedef TOX_ERR_GET_PORT Tox_Err_Get_Port;

typedef enum TOX_USER_STATUS {

  TOX_USER_STATUS_Available,   /**< The entity is online. */
  TOX_USER_STATUS_Chat,        /**< The entity is 'available for chat'. */
  TOX_USER_STATUS_Away,        /**< The entity is away. */
  TOX_USER_STATUS_DND,         /**< The entity is DND (Do Not Disturb). */
  TOX_USER_STATUS_XA,          /**< The entity is XA (eXtended Away). */
  TOX_USER_STATUS_Unavailable, /**< The entity is offline. */
  TOX_USER_STATUS_Probe,       /**< This is a presence probe. */
  TOX_USER_STATUS_Error,       /**< This is a presence error. */
  TOX_USER_STATUS_Invalid      /**< The stanza is invalid. */

} TOX_USER_STATUS;
typedef TOX_USER_STATUS Tox_User_Status;

typedef enum TOX_ERR_OPTIONS_NEW {

  /**
   * The function returned successfully.
   */
  TOX_ERR_OPTIONS_NEW_OK,

  /**
   * The function failed to allocate enough memory for the options struct.
   */
  TOX_ERR_OPTIONS_NEW_MALLOC,

} TOX_ERR_OPTIONS_NEW;
typedef TOX_MESSAGE_TYPE Tox_Message_Type;

// typedef TOX_PROXY_TYPE Tox_Proxy_Type;
// typedef TOX_SAVEDATA_TYPE Tox_Savedata_Type;
typedef enum TOX_LOG_LEVEL {

  /**
   * Very detailed traces including all network activity.
   */
  TOX_LOG_LEVEL_TRACE,

  /**
   * Debug messages such as which port we bind to.
   */
  TOX_LOG_LEVEL_DEBUG,

  /**
   * Informational log messages such as video call status changes.
   */
  TOX_LOG_LEVEL_INFO,

  /**
   * Warnings about internal inconsistency or logic errors.
   */
  TOX_LOG_LEVEL_WARNING,

  /**
   * Severe unexpected errors caused by external or internal inconsistency.
   */
  TOX_LOG_LEVEL_ERROR,

} TOX_LOG_LEVEL;

typedef TOX_LOG_LEVEL Tox_Log_Level;

/**
 * Protocols that can be used to connect to the network or friends.
 *
 * @deprecated All UPPER_CASE enum type names are deprecated. Use the
 *   Camel_Snake_Case versions, instead.
 */
typedef enum TOX_CONNECTION {

  /**
   * There is no connection. This instance, or the friend the state change is
   * about, is now offline.
   */
  TOX_CONNECTION_NONE,

  /**
   * A TCP connection has been established. For the own instance, this means it
   * is connected through a TCP relay, only. For a friend, this means that the
   * connection to that particular friend goes through a TCP relay.
   */
  TOX_CONNECTION_TCP,

  /**
   * A UDP connection has been established. For the own instance, this means it
   * is able to send UDP packets to DHT nodes, but may still be connected to
   * a TCP relay. For a friend, this means that the connection to that
   * particular friend was built using direct UDP packets.
   */
  TOX_CONNECTION_UDP,

} TOX_CONNECTION;
typedef TOX_CONNECTION Tox_Connection;

typedef enum TOX_FILE_CONTROL {

  /**
   * Sent by the receiving side to accept a file send request. Also sent after a
   * TOX_FILE_CONTROL_PAUSE command to continue sending or receiving.
   */
  TOX_FILE_CONTROL_RESUME,

  /**
   * Sent by clients to pause the file transfer. The initial state of a file
   * transfer is always paused on the receiving side and running on the sending
   * side. If both the sending and receiving side pause the transfer, then both
   * need to send TOX_FILE_CONTROL_RESUME for the transfer to resume.
   */
  TOX_FILE_CONTROL_PAUSE,

  /**
   * Sent by the receiving side to reject a file send request before any other
   * commands are sent. Also sent by either side to terminate a file transfer.
   */
  TOX_FILE_CONTROL_CANCEL,

} TOX_FILE_CONTROL;
typedef TOX_FILE_CONTROL Tox_File_Control;

/**
 * Conference types for the conference_invite event.
 *
 * @deprecated All UPPER_CASE enum type names are deprecated. Use the
 *   Camel_Snake_Case versions, instead.
 */
typedef enum TOX_CONFERENCE_TYPE {

  /**
   * Text-only conferences that must be accepted with the tox_conference_join
   * function.
   */
  TOX_CONFERENCE_TYPE_TEXT,

  /**
   * Video conference. The function to accept these is in toxav.
   */
  TOX_CONFERENCE_TYPE_AV,

} TOX_CONFERENCE_TYPE;
typedef TOX_CONFERENCE_TYPE Tox_Conference_Type;

enum TOX_FILE_KIND {
  TOX_FILE_KIND_DATA,

  TOX_FILE_KIND_AVATAR,

};

typedef enum TOX_ERR_FRIEND_SEND_MESSAGE {

  /**
   * The function returned successfully.
   */
  TOX_ERR_FRIEND_SEND_MESSAGE_OK,

  /**
   * One of the arguments to the function was NULL when it was not expected.
   */
  TOX_ERR_FRIEND_SEND_MESSAGE_NULL,

  /**
   * The friend number did not designate a valid friend.
   */
  TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_FOUND,

  /**
   * This client is currently not connected to the friend.
   */
  TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED,

  /**
   * An allocation error occurred while increasing the send queue size.
   */
  TOX_ERR_FRIEND_SEND_MESSAGE_SENDQ,

  /**
   * Message length exceeded TOX_MAX_MESSAGE_LENGTH.
   */
  TOX_ERR_FRIEND_SEND_MESSAGE_TOO_LONG,

  /**
   * Attempted to send a zero-length message.
   */
  TOX_ERR_FRIEND_SEND_MESSAGE_EMPTY,

} TOX_ERR_FRIEND_SEND_MESSAGE;
typedef TOX_ERR_FRIEND_SEND_MESSAGE Tox_Err_Friend_Send_Message;

typedef enum TOX_ERR_CONFERENCE_SEND_MESSAGE {

  /**
   * The function returned successfully.
   */
  TOX_ERR_CONFERENCE_SEND_MESSAGE_OK,

  /**
   * The conference number passed did not designate a valid conference.
   */
  TOX_ERR_CONFERENCE_SEND_MESSAGE_CONFERENCE_NOT_FOUND,

  /**
   * The message is too long.
   */
  TOX_ERR_CONFERENCE_SEND_MESSAGE_TOO_LONG,

  /**
   * The client is not connected to the conference.
   */
  TOX_ERR_CONFERENCE_SEND_MESSAGE_NO_CONNECTION,

  /**
   * The message packet failed to send.
   */
  TOX_ERR_CONFERENCE_SEND_MESSAGE_FAIL_SEND,

} TOX_ERR_CONFERENCE_SEND_MESSAGE;
typedef TOX_ERR_CONFERENCE_SEND_MESSAGE Tox_Err_Conference_Send_Message;

typedef enum TOX_ERR_BOOTSTRAP {

  /**
   * The function returned successfully.
   */
  TOX_ERR_BOOTSTRAP_OK,

  /**
   * One of the arguments to the function was NULL when it was not expected.
   */
  TOX_ERR_BOOTSTRAP_NULL,

  /**
   * The hostname could not be resolved to an IP address, or the IP address
   * passed was invalid.
   */
  TOX_ERR_BOOTSTRAP_BAD_HOST,

  /**
   * The port passed was invalid. The valid port range is (1, 65535).
   */
  TOX_ERR_BOOTSTRAP_BAD_PORT,

} TOX_ERR_BOOTSTRAP;
typedef TOX_ERR_BOOTSTRAP Tox_Err_Bootstrap;

typedef enum TOX_ERR_CONFERENCE_TITLE {

  /**
   * The function returned successfully.
   */
  TOX_ERR_CONFERENCE_TITLE_OK,

  /**
   * The conference number passed did not designate a valid conference.
   */
  TOX_ERR_CONFERENCE_TITLE_CONFERENCE_NOT_FOUND,

  /**
   * The title is too long or empty.
   */
  TOX_ERR_CONFERENCE_TITLE_INVALID_LENGTH,

  /**
   * The title packet failed to send.
   */
  TOX_ERR_CONFERENCE_TITLE_FAIL_SEND,

} TOX_ERR_CONFERENCE_TITLE;