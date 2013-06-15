// ssquery.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ssq.h"

class CResponse : public ssq::IRulesResponse, public ssq::IPlayersResponse
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
		if ( !stricmp( type, "player" ) )
		{
			query = ssq::PlayerDetails( &resp );
		}
		else if( !stricmp( type, "rules" ) )
		{
			query = ssq::ServerRules( &resp );
		}
		else
		{
			printf( "Invalid query type '%s', did you mean 'player' or 'rules'?\n", type );
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

