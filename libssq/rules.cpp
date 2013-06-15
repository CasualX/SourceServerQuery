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
	// Process rules
	// FIXME! Support multi-packet and async responses!
	// FIXME! Figure out why I get an empty multi-packet header here (need to skip 0xC bytes)!
	bf_read bf;

	if ( Challenge( A2S_RULES ) && Recv( bf ) && bf.Skip(0xC+4) && bf.Read<char>()==S2A_RULES )
	{
		long num = bf.Read<unsigned short>();
		for ( ; num; --num )
		{
			const char* rule;
			const  char* value;
			if ( ( rule = bf.ReadString() ) && ( value = bf.ReadString() ) )
			{
				_cb->RulesResponded( rule, value );
			}
			else
			{
				// For now ignore overflows...
				break;
			}
		}
		_cb->RulesFinished( true );
		return true;
	}
	_cb->RulesFinished( false );
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
