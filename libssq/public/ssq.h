#pragma once

//----------------------------------------------------------------
// Public interface for SSQ
//----------------------------------------------------------------

namespace ssq
{

// Server query interface
class IQuery
{
public:
	virtual ~IQuery() { }
	virtual bool Connect( const char* address, long timeout = 5000 ) = 0;
	virtual void Disconnect() = 0;
	// Begin the query, when async it'll return immediately
	// Returns true if the request was a success or if the async thread was started correctly
	virtual bool Perform( bool async ) = 0;
	// Cancel or wait for the query to finish
	virtual void Wait( bool cancel ) = 0;
};

// Response callback for a rules query
class IRulesResponse
{
public:
	// Each rule will get passed through here
	virtual void RulesResponded( const char* rule, const char* value ) = 0;
	// Finished the query
	virtual void RulesFinished( bool success ) = 0;
};
// Response callback for a player details query
class IPlayersResponse
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
class IServerResponse
{
public:
	// On success
	virtual void ServerInfo( gameserver_t& info ) = 0;
	// On failure
	virtual void ServerFailed() = 0;
};


// Create a rules query, you must delete this when done! Always returns a valid handle
IQuery* ServerRules( const char* address, IRulesResponse* resp );
IQuery* PlayerDetails( const char* address, IPlayersResponse* resp );
IQuery* ServerInfo( const char* address, IServerResponse* resp );

}
