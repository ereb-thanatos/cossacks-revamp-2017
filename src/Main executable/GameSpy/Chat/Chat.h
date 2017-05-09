/*
GameSpy Chat SDK 
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 1999 GameSpy Industries, Inc

Suite E-204
2900 Bristol Street
Costa Mesa, CA 92626
(714)549-7689
Fax(714)549-0757
*/

#ifndef _CHAT_H_
#define _CHAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/************
** DEFINES **
************/
// User and channel message types.
//////////////////////////////////
#define CHAT_MESSAGE        0
#define CHAT_ACTION         1
#define CHAT_NOTICE         2

// User modes.
//////////////
#define CHAT_NORMAL         0
#define CHAT_VOICE          1
#define CHAT_OP             2

// Part reasons (see the chatUseParted callback).
/////////////////////////////////////////////////
#define CHAT_LEFT           0
#define CHAT_QUIT           1
#define CHAT_KICKED         2
#define CHAT_KILLED         3

// Possible nick errors while connecting.
/////////////////////////////////////////
#define CHAT_IN_USE         0
#define CHAT_INVALID        1

/**********
** TYPES **
**********/
// Boolean type.
////////////////
typedef enum { CHATFalse, CHATTrue } CHATBool;

// A CHAT object represents a client connection to a chat server.
/////////////////////////////////////////////////////////////////
typedef void * CHAT;

// Object representing a channel's mode.
////////////////////////////////////////
typedef struct CHATChannelMode
{
	CHATBool InviteOnly;
	CHATBool Private;
	CHATBool Secret;
	CHATBool Moderated;
	CHATBool NoExternalMessages;
	CHATBool OnlyOpsChangeTopic;
	int Limit;
} CHATChannelMode;

/**********************
** GLOBALS CALLBACKS **
**********************/
// Gets raw incoming network traffic.
/////////////////////////////////////
typedef void (* chatRaw)(CHAT chat,
						 const char * raw,
						 void * param);

// Called when the client has been disconnected.
////////////////////////////////////////////////
typedef void (* chatDisconnected)(CHAT chat,
								  const char * reason,
								  void * param);

// Called when a private message from another user is received.
// If user==NULL, this is a message from the server.
///////////////////////////////////////////////////////////////
typedef void (* chatPrivateMessage)(CHAT chat,
									const char * user,
									const char * message,
									int type,  // See defined message types above.
									void * param);

// Called when invited into a channel.
//////////////////////////////////////
typedef void (* chatInvited)(CHAT chat,
							 const char * channel,
							 const char * user,
							 void * param);

// A connection's global callbacks.
///////////////////////////////////
typedef struct chatGlobalCallbacks
{
	chatRaw raw;
	chatDisconnected disconnected;
	chatPrivateMessage privateMessage;
	chatInvited invited;
	void * param;
} chatGlobalCallbacks;

/**********************
** CHANNEL CALLBACKS **
**********************/
// Called when a message is received in a channel.
//////////////////////////////////////////////////
typedef void (* chatChannelMessage)(CHAT chat,
									const char * channel,
									const char * user,
									const char * message,
									int type,  // See defined message types above.
									void * param);

// Called when the local client is kicked from a channel.
/////////////////////////////////////////////////////////
typedef void (* chatKicked)(CHAT chat,
							const char * channel,
							const char * user,
							const char * reason,
							void * param);

// Called when a user joins a channel we're in.
///////////////////////////////////////////////
typedef void (* chatUserJoined)(CHAT chat,
								const char * channel,
								const char * user,
								int mode,    // See defined user modes above.
								void * param);

// Called when a user parts a channel we're in.
///////////////////////////////////////////////
typedef void (* chatUserParted)(CHAT chat,
								const char * channel,
								const char * user,
								int why,    // See defined part reasons above.
								const char * reason,
								const char * kicker,
								void * param);

// Called when a user in a channel we're in changes nicks.
//////////////////////////////////////////////////////////
typedef void (* chatUserChangedNick)(CHAT chat,
									 const char * channel,
									 const char * oldNick,
									 const char * newNick,
									 void * param);

// Called when the topic changes in a channel we're in.
///////////////////////////////////////////////////////
typedef void (* chatTopicChanged)(CHAT chat,
								  const char * channel,
								  const char * topic,
								  void * param);

// Called when the mode changes in a channel we're in.
//////////////////////////////////////////////////////
typedef void (* chatChannelModeChanged)(CHAT chat,
										const char * channel,
										CHATChannelMode * mode,
										void * param);

// Called when a user's mode changes in a channel we're in.
///////////////////////////////////////////////////////////
typedef void (* chatUserModeChanged)(CHAT chat,
									 const char * channel,
									 const char * user,
									 int mode,  // See defined user modes above.
									 void * param);

// Called when the user list changes (due to a join or a part) in a channel we're in.
/////////////////////////////////////////////////////////////////////////////////////
typedef void (* chatUserListUpdated)(CHAT chat,
									 const char * channel,
									 void * param);

// A channel's callbacks.
/////////////////////////
typedef struct chatChannelCallbacks
{
	chatChannelMessage channelMessage;
	chatKicked kicked;
	chatUserJoined userJoined;
	chatUserParted userParted;
	chatUserChangedNick userChangedNick;
	chatTopicChanged topicChanged;
	chatChannelModeChanged channelModeChanged;
	chatUserModeChanged userModeChanged;
	chatUserListUpdated userListUpdated;
	void * param;
} chatChannelCallbacks;

/************
** GENERAL **
************/
// Called when a connect attempt completes.
///////////////////////////////////////////
typedef void (* chatConnectCallback)(CHAT chat,
									 CHATBool success,
									 void * param);
// Called if there is an error with the nick while connecting.
// To retry with a new nick, call chatRetryWithNick.
// Otherwise, call chatDisconnect to stop the connection.
//////////////////////////////////////////////////////////////
typedef void (* chatNickErrorCallback)(CHAT chat,
									   int type,  // CHAT_IN_USE, CHAT_INVALID
									   const char * nick,
									   void * param);
typedef void (* chatFillInUserCallback)(CHAT chat,
										unsigned int IP, // PANTS|08.21.00 - changed from unsigned long
										char user[128],
										void * param);
// Connects you to a chat server and returns a CHAT object.
///////////////////////////////////////////////////////////
CHAT chatConnect(const char * serverAddress,
				 int port,
                 const char * nick,
				 const char * user,
				 const char * name,
				 chatGlobalCallbacks * callbacks,
				 chatNickErrorCallback nickErrorCallback,
                 chatConnectCallback connectCallback,
                 void * param,
                 CHATBool blocking);
CHAT chatConnectSpecial(const char * serverAddress,
				 int port,
                 const char * nick,
				 const char * name,
				 chatGlobalCallbacks * callbacks,
				 chatNickErrorCallback nickErrorCallback,
				 chatFillInUserCallback fillInUserCallback,
                 chatConnectCallback connectCallback,
                 void * param,
                 CHATBool blocking);
// If the chatNickErrorCallback gets called, then this can be called
// with a new nick to retry.  If this isn't called, the connection can be
// disconnected with chatDisconnect.  If the new nick is successful, then
// the chatConnectCallback will get called.  If there's another nick 
// error, the chatNickErrorCallback will get called again.
/////////////////////////////////////////////////////////////////////////
void chatRetryWithNick(CHAT chat,
					   const char * nick);

// Disconnect the chat connection.
//////////////////////////////////
void chatDisconnect(CHAT chat);

// Processes the chat connection.
/////////////////////////////////
void chatThink(CHAT chat);

// Sends raw data, without any interpretation.
//////////////////////////////////////////////
void chatSendRaw(CHAT chat,
				 const char * command);

/*************
** CHANNELS **
*************/
// Gets called for each channel enumerated.
///////////////////////////////////////////
typedef void (* chatEnumChannelsCallbackEach)(CHAT chat,
											  CHATBool success,
											  int index,
											  const char * channel,
											  const char * topic,
											  int numUsers,
											  void * param);
// Gets called after all channels have been enumerated.
///////////////////////////////////////////////////////
typedef void (* chatEnumChannelsCallbackAll)(CHAT chat,
											 CHATBool success,
											 int numChannels,
											 const char ** channels,
											 const char ** topics,
											 int * numUsers,
											 void * param);
// Enumerates the channels available on a chat server.
//////////////////////////////////////////////////////
void chatEnumChannels(CHAT chat,
					  chatEnumChannelsCallbackEach callbackEach,
					  chatEnumChannelsCallbackAll callbackAll,
					  void * param,
					  CHATBool blocking);

// Gets called for each channel enumerated.
///////////////////////////////////////////
typedef void (* chatEnumJoinedChannelsCallback)(CHAT chat,
											  int index,
											  const char * channel,
											  void * param);

// Enumerates the channels that we are joined to
//////////////////////////////////////////////////////
void chatEnumJoinedChannels(CHAT chat,
					  chatEnumJoinedChannelsCallback callback,
					  void * param);



// Gets called when a channel has been entered.
///////////////////////////////////////////////
typedef void (* chatEnterChannelCallback)(CHAT chat,
										  CHATBool success,
										  const char * channel,
										  void * param);
// Enters a channel.
////////////////////
void chatEnterChannel(CHAT chat,
					  const char * channel,
					  const char * password,
					  chatChannelCallbacks * callbacks,
					  chatEnterChannelCallback callback,
					  void * param,
					  CHATBool blocking);

// Leaves a channel.
////////////////////
void chatLeaveChannel(CHAT chat,
					  const char * channel);

// Sends a message to a channel.
////////////////////////////////
void chatSendChannelMessage(CHAT chat,
							const char * channel,
							const char * message,
							int type);

// Sets the topic in a channel.
///////////////////////////////
void chatSetChannelTopic(CHAT chat,
						 const char * channel,
						 const char * topic);

// Gets called when a channel's topic has been retrieved.
/////////////////////////////////////////////////////////
typedef void (* chatGetChannelTopicCallback)(CHAT chat,
											 CHATBool success,
											 const char * channel,
											 const char * topic,
											 void * param);
// Gets a channel's topic.
//////////////////////////
void chatGetChannelTopic(CHAT chat,
						 const char * channel,
						 chatGetChannelTopicCallback callback,
						 void * param,
						 CHATBool blocking);

// Sets a channel's mode.
/////////////////////////
void chatSetChannelMode(CHAT chat,
						const char * channel,
						CHATChannelMode * mode);

// Gets called when a channel's mode has been retrieved.
////////////////////////////////////////////////////////
typedef void (* chatGetChannelModeCallback)(CHAT chat,
											CHATBool success,
											const char * channel,
											CHATChannelMode * mode,
											void * param);
// Gets a channel's mode.
/////////////////////////
void chatGetChannelMode(CHAT chat,
						const char * channel,
						chatGetChannelModeCallback callback,
						void * param,
						CHATBool blocking);

// Sets the password in a channel.
//////////////////////////////////
void chatSetChannelPassword(CHAT chat,
							const char * channel,
							CHATBool enable,
							const char * password);

// Called when the channel's password has been retrieved.
/////////////////////////////////////////////////////////
typedef void (* chatGetChannelPasswordCallback)(CHAT chat,
												CHATBool success,
												const char * channel,
												CHATBool enabled,
												const char * password,
												void * param);
// Gets the password in a channel.
//////////////////////////////////
void chatGetChannelPassword(CHAT chat,
							const char * channel,
							chatGetChannelPasswordCallback callback,
							void * param,
							CHATBool blocking);

// Called with the list of bans in a channel.
/////////////////////////////////////////////
typedef void (* chatEnumChannelBansCallback)(CHAT chat,
											 CHATBool success,
											 const char * channel,
											 int numBans,
											 const char ** bans,
											 void * param);
// Enumerate through the bans in a channel.
///////////////////////////////////////////
void chatEnumChannelBans(CHAT chat,
						 const char * channel,
						 chatEnumChannelBansCallback callback,
						 void * param,
						 CHATBool blocking);

// Adds a channel ban.
//////////////////////
void chatAddChannelBan(CHAT chat,
					   const char * channel,
					   const char * ban);

// Removes a ban string from a channel.
///////////////////////////////////////
void chatRemoveChannelBan(CHAT chat,
						  const char * channel,
						  const char * ban);

/**********
** USERS **
**********/
// Called with the list of users in a channel.
//////////////////////////////////////////////
typedef void (* chatEnumUsersCallback)(CHAT chat,
									   CHATBool success,
									   const char * channel, //PANTS|02.11.00|added paramater
									   int numUsers,
									   const char ** users,
									   int * modes,
									   void * param);
// Enumerate through the users in a channel.
////////////////////////////////////////////
void chatEnumUsers(CHAT chat,
				   const char * channel,
				   chatEnumUsersCallback callback,
				   void * param,
				   CHATBool blocking);

// Send a private message to a user.
////////////////////////////////////
void chatSendUserMessage(CHAT chat,
						 const char * user,
						 const char * message,
						 int type);

// Called with a user's info.
/////////////////////////////
typedef void (* chatGetUserInfoCallback)(CHAT chat,
										 CHATBool success,
										 const char * nick,  //PANTS|02.14.2000|added nick and user
										 const char * user,
										 const char * name,
										 const char * address,
										 int numChannels,
										 const char ** channels,
										 void * param);
// Get a user's info.
/////////////////////
void chatGetUserInfo(CHAT chat,
					 const char * user,
					 chatGetUserInfoCallback callback,
					 void * param,
					 CHATBool blocking);

// Invite a user into a channel.
////////////////////////////////
void chatInviteUser(CHAT chat,
					const char * channel,
					const char * user);

// Kick a user from a channel.
//////////////////////////////
void chatKickUser(CHAT chat,
				  const char * channel,
				  const char * user,
				  const char * reason);

// Ban a user from a channel.
/////////////////////////////
void chatBanUser(CHAT chat,
				 const char * channel,
				 const char * user);

// Sets a user's mode in a channel.
///////////////////////////////////
void chatSetUserMode(CHAT chat,
					 const char * channel,
					 const char * user,
					 int mode);

// Called with the user's mode.
///////////////////////////////
typedef void (* chatGetUserModeCallback)(CHAT chat,
										 CHATBool success,
										 const char * channel,
										 const char * user,
										 int mode,
										 void * param);
// Gets a user's mode in a channel.
///////////////////////////////////
void chatGetUserMode(CHAT chat,
					 const char * channel,
					 const char * user,
					 chatGetUserModeCallback callback,
					 void * param,
					 CHATBool blocking);

// Called with the list of users in a channel.
//////////////////////////////////////////////
typedef void (* chatChangeNickCallback)(CHAT chat,
										CHATBool success,
										const char * oldNick,
										const char * newNick,
										void * param);
// Change the local user's nick.
////////////////////////////////
void chatChangeNick(CHAT chat,
					const char * newNick,
					chatChangeNickCallback callback,
					void * param,
					CHATBool blocking);

// Get our local nickname
////////////////////////////////
char *chatGetNick(CHAT chat);

#ifdef __cplusplus
}
#endif

#endif