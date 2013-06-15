#include "stdafx.h"

#include "basequery.h"

namespace ssq
{

SSQ_INTERFACE CQueryPlayers : public CBaseQuery
{
public:
	CQueryPlayers( IPlayersResponse* cb ) : _cb(cb) { }
	virtual bool Thread();

private:
	IPlayersResponse* _cb;
};

bool CQueryPlayers::Thread()
{
	// Local vars
	unsigned char buf[1260];
	long bytes;

	// Request rules
	if ( !Challenge( A2S_PLAYER ) )
		goto failure;

	// Process players
	bytes = Recv( buf, sizeof(buf) );

	if ( *(long*)(buf+0)==-1 && buf[4]==S2A_PLAYER )
	{
		unsigned char num = buf[5];
		unsigned char* it = buf+5;
		unsigned char* end = buf+bytes;

		for ( ; num; --num )
		{
			unsigned char index = *it++;
			const char* name = (const char*) it;
			while ( *it++ ) { if ( it>=end ) goto failure; }
			long score = *((long*&)it)++;
			float& duration = *((float*&)it)++;
			if ( it>end ) goto failure;
			_cb->PlayersAdd( name, score, duration );
		}
	}

failure:
	_cb->PlayersFinished( false );
	return false;
}



IQuery* PlayerDetails( IPlayersResponse* resp )
{
	CBaseQuery* q = new CQueryPlayers( resp );
	return q;
}

}