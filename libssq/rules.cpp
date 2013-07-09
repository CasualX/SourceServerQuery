#include "stdafx.h"

#include "basequery.h"

namespace ssq
{
	
class CQueryRules : public CBaseQuery
{
public:
	CQueryRules( IRulesResponse* cb );
	virtual bool Thread();
			//void Parse( const char* str, long count );

			char* Parse( char* buf, char* end );
			void Parse( bf_read& bf, char buf[256] );
private:
	IRulesResponse* _cb;
};


CQueryRules::CQueryRules( IRulesResponse* cb ) : _cb(cb)
{
}
bool CQueryRules::Thread()
{
	// Process rules

	bf_read* bf;
	bool s = false;

	if ( Challenge( A2S_RULES ) && ( bf = Response() ) )
	{
		long num;
		if ( bf->Read<long>()==0xFFFFFFFF && bf->Read<unsigned char>()==S2A_RULES && ( num = bf->Read<unsigned short>() ) )
		{
			for ( ; num; --num )
			{
				const char* rule;
				const  char* value;
				if ( ( rule = bf->ReadString() ) && ( value = bf->ReadString() ) )
				{
					_cb->RulesResponded( rule, value );
				}
			}
			s = true;
		}

		::free( bf );
	}
	_cb->RulesFinished( s );
	return s;
}


IQuery* ServerRules( IRulesResponse* resp )
{
	CBaseQuery* q = new CQueryRules( resp );
	return q;
}

}
