#include "stdafx.h"

#include "basequery.h"

namespace ssq
{

class CQueryPlayers : public CBaseQuery
{
public:
	CQueryPlayers( IPlayersResponse* cb ) : _cb(cb) { }
	virtual bool Thread();

private:
	IPlayersResponse* _cb;
};

bool CQueryPlayers::Thread()
{
	bf_read bf;

	// Request players
	if ( Challenge( A2S_PLAYER ) && Recv( bf ) && bf.Read<long>()==-1 && bf.Read<char>()==S2A_PLAYER )
	{
		// Alleged number of players
		unsigned char num = bf.Read<unsigned char>();

		for ( ; num; --num )
		{
			unsigned char index = bf.Read<unsigned char>();
			const char* name = bf.ReadString();
			if ( !name )
				goto failure;
			long score = bf.Read<long>();
			float& duration = bf.Read<float>();

			_cb->PlayersAdd( name, score, duration );
		}

		_cb->PlayersFinished( true );
		return true;
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
