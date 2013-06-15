#include "stdafx.h"

#include "basequery.h"

namespace ssq
{
	
class CQueryRules : public CBaseQuery
{
public:
	CQueryRules( IRulesResponse* cb );
	virtual bool Thread();
			void Parse( const char* str, long count );
private:
	IRulesResponse* _cb;
};


CQueryRules::CQueryRules( IRulesResponse* cb ) : _cb(cb)
{
}
bool CQueryRules::Thread()
{
	// Local vars
	unsigned char buf[1260];
	long bytes;

	// Request rules
	if ( !Challenge( A2S_RULES ) )
		goto failure;

	// Process rules
	// FIXME! Support multi-packet and async responses!
	// FIXME! Figure out why I get an empty multi-packet header here (need to skip 0xC bytes)!
	bytes = Recv( buf, sizeof(buf) );
	if ( bytes<0xC )
		goto failure;
	char* it = (char*)( buf+0xC );
	if ( it[4]!=S2A_RULES )
		goto failure;

	long num = *(unsigned short*)(it+5);
	it = (char*)( it+7 );
	char* end = (char*)( it+bytes );
	for ( ; num; --num )
	{
		const char* rule = it;
		while ( *it++ ) { if ( it>=end ) goto overflow; }
		const char* value = it;
		while ( *it++ ) { if ( it>=end ) goto overflow; }
		_cb->RulesResponded( rule, value );
	}

overflow:
	// For now ignore overflows...
	_cb->RulesFinished( true );
	return true;

failure:
	_cb->RulesFinished( false );
#ifdef _DEBUG
	int err = ::WSAGetLastError();
#endif // _DEBUG
	return false;
}
void CQueryRules::Parse( const char* str, long size )
{

}



IQuery* ServerRules( IRulesResponse* resp )
{
	CBaseQuery* q = new CQueryRules( resp );
	return q;
}

}
