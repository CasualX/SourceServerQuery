# SourceServerQuery

C++ Library for querying Source game servers.
Currently only supports rules queries and targets VS 2010 (but I'm sure it'll work with any other version).

### Why?
The [C alternative](https://developer.valvesoftware.com/wiki/Source_Server_Query_Library) looks horrible and has a bunch of unneeded stuff. I want a clean, basic C++ version.

### How?
Add libssq\public as include and library path.

### Example
```C++
	#include <cstdio>
    #include "ssq.h"
	class CResponse : public ssq::IRulesResponse
	{
	public:
		void RulesResponded( const char* rule, const char* value )
		{
			printf( " rule %s = %s\n", rule, value );
		}
		void RulesFinished( bool success )
		{
			if ( !success )
				printf( "%s", "Server failed to respond!" );
		}
	};
	int main( int argc, char* argv[] )
	{
		CResponse resp;
		ssq::IQuery* query = ssq::ServerRules( &resp );
		query->Connect( "95.154.193.166:27015", 5000 );
		query->Perform();
		delete query;
	}
