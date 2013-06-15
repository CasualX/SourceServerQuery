// ssquery.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ssq.h"

class CResponse : public ssq::IRulesResponse, public ssq::IPlayersResponse, public ssq::IServerResponse
{
public:
	virtual void RulesResponded( const char* rule, const char* value )
	{
		printf( " rule %s = %s\n", rule, value );
	}
	virtual void RulesFinished( bool success )
	{
		printf( " rules %s\n", success?"success":"failure" );
	}
	virtual void PlayersAdd( const char* player, long score, float duration )
	{
		printf( " player %d %f %s\n", score, duration, player );
	}
	virtual void PlayersFinished( bool success )
	{
		printf( " players %s\n", success?"success":"failure" );
	}
	virtual void ServerInfo( ssq::gameserver_t& info )
	{
		printf( " name: %s\n"
			" map: %s\n"
			" folder: %s\n"
			" game: %s\n"
			" appid: %d\n"
			" version: %s\n",
			info.name, info.map, info.folder, info.game, info.appid, info.version );
		printf( " players: %d\n"
			" maxplayers: %d\n"
			" bots: %d\n",
			info.players, info.maxplayers, info.bots );
		printf( " protocol: %d\n"
			" server_type: %c\n"
			" environment: %c\n"
			" password: %s\n"
			" vac: %s\n",
			info.protocol, info.server_type, info.environment,
			info.password?"yes":"no",
			info.vac?"yes":"no" );
		if ( info.edf.has_port() )
			printf( " port: %d\n", info.edf.port );
		if ( info.edf.has_steamid() )
			printf( " steamid: %ull\n", info.edf.steamid );
		if ( info.edf.has_stv() )
			printf( " stv_port: %d\n stv_name: %s\n", info.edf.stv_port, info.edf.stv_name );
		if ( info.edf.has_tags() )
			printf( " tags: %s\n", info.edf.tags );
		if ( info.edf.has_gameid() )
			printf( " gameid: %ull\n", info.edf.gameid );
	}
	virtual void ServerFailed()
	{
		printf( " info failure\n" );
	}
};

int main( int argc, char* argv[] )
{
	if ( argc<3 )
	{
		printf( "%s",
			"Source Server Query by CasualX\n"
			"\n"
			"Example query all players in a server:\n"
			" ssquery 95.154.193.166:27015 player\n"
			"Example query server rules:\n"
			" ssquery 95.154.193.166:27015 rules\n" );
		return 0;
	}
	else
	{
		CResponse resp;

		const char* type = argv[2];
		ssq::IQuery* query;
		if ( !_stricmp( type, "player" ) )
		{
			query = ssq::PlayerDetails( &resp );
		}
		else if( !_stricmp( type, "rules" ) )
		{
			query = ssq::ServerRules( &resp );
		}
		else if ( !_stricmp( type, "info" ) )
		{
			query = ssq::ServerInfo( &resp );
		}
		else
		{
			printf( "Invalid query type '%s', did you mean 'player', 'rules' or 'info'?\n", type );
			return -1;
		}
		
		const char* address = argv[1];
		unsigned long timeout = 5000;
		if ( argc>3 ) timeout = atoi( argv[3] );

		if ( !query->Connect( address, 0, timeout ) )
		{
			printf( "Failed to connect to '%s'.\n", address );
		}
		else if ( !query->Perform() )
		{
			printf( "Request failed." );
		}

		delete query;
		return 0;
	}
}

