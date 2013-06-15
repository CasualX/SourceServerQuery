#pragma once

//----------------------------------------------------------------
// Public interface for SSQ
//----------------------------------------------------------------

// Tell the linker to include some libraries for us.
#ifndef _LIB
#pragma comment(lib, "Ws2_32.lib")
#ifdef _DEBUG
#pragma comment(lib, "libssqd.lib")
#else
#pragma comment(lib, "libssq.lib")
#endif // _DEBUG
#endif // !_LIB

#define SSQ_INTERFACE class __declspec(novtable)

namespace ssq
{

// Server query interface
SSQ_INTERFACE IQuery
{
public:
	virtual ~IQuery() { }

	// Connect to an address (formatted <host>:<port>) and set the timeout
	// Will not call into callback on failure
	virtual bool Connect( const char* address, unsigned short port = 0, long timeout = 5000 ) = 0;

	// Disconnect if you want to reuse this object to do more queries on a different address
	virtual void Disconnect() = 0;

	// Begin the query, when async it'll return immediately
	// Returns true if the request was a success or if the async thread was started correctly
	// The only place that will call into the callback
	virtual bool Perform( bool async = false ) = 0;

	// Cancel the query or wait for it to finish
	virtual void Wait( bool cancel = false ) = 0;
};

// Response callback for a rules query
SSQ_INTERFACE IRulesResponse
{
public:
	// Each rule will get passed through here
	virtual void RulesResponded( const char* rule, const char* value ) = 0;
	// Finished the query
	virtual void RulesFinished( bool success ) = 0;
};

// Response callback for a player details query
SSQ_INTERFACE IPlayersResponse
{
public:
	// Each player will get passed through here
	virtual void PlayersAdd( const char* player, long score, float duration ) = 0;
	// Finished the query
	virtual void PlayersFinished( bool success ) = 0;
};

// Response callback for server info
struct gameserver_t
{
	char* address;
	char* name;
	char* map;
	char* folder;
	char* game;
	unsigned short appid;

	unsigned char players;
	unsigned char maxplayers;
	unsigned char bots;

	unsigned char protocol;
	char server_type;
	char environment;
	bool password;
	bool vac;

	//bool ismod;
	//struct mod_t
	//{
	//	char* website;
	//	char* download;
	//	long version;
	//	long size;
	//	bool mp;
	//	bool dll;
	//} mod;
};
SSQ_INTERFACE IServerResponse
{
public:
	// On success
	virtual void ServerInfo( gameserver_t& info ) = 0;
	// On failure
	virtual void ServerFailed() = 0;
};

// Create a rules query, you must delete this when done! Always returns a valid handle
IQuery* ServerRules( IRulesResponse* resp );
IQuery* PlayerDetails( IPlayersResponse* resp );
IQuery* ServerInfo( IServerResponse* resp );

}
