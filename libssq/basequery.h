#pragma once

#include "public/ssq.h"
#include "stdafx.h"

namespace ssq
{
	
// Query types
enum
{
	A2M_GET_SERVERS_BATCH2 = 0x31,
	M2A_SERVER_BATCH = 0x66,

	S2C_CHALLENGE = 0x41,

	S2A_PLAYER = 0x44,
	S2A_RULES = 0x45,
	S2A_INFO = 0x49,
	S2A_LOG = 0x52,

	A2S_INFO = 0x54,
	A2S_PLAYER = 0x55,
	A2S_RULES = 0x56,
	A2S_GETCHALLENGE = 0x57,

	A2A_PING = 0x69,
	A2A_ACK = 0x6A,
};

// Really need this...
class bf_read
{
public:
	template< typename T >
	inline T& Peek() { return *(T*)it; }
	
	template< typename T >
	inline T& Read() { return *((T*&)it)++; }

	template< typename T >
	inline bool Read( T& t )
	{
		if ( ( it + sizeof(T) )<=(raw+bytes) )
		{
			t =  *((T*&)it)++;
			return true;
		}
		else
		{
			return false;
		}
	}

	inline char* ReadString()
	{
		char* str = it;
		while ( *it++ )
		{
			// Overflowed
			if ( it>=(raw+bytes) )
				return nullptr;
		}
		return str;
	}

	inline bool Skip( unsigned long c )
	{
		it += c;
		return it<(raw+bytes);
	}

	// Accessors
	inline long size() const { return bytes; }

public:
	char* it;
	long bytes;
	char raw[1260];
};



SSQ_INTERFACE CBaseQuery : public IQuery
{
public:
	CBaseQuery();
	~CBaseQuery();

	// Create the socket and connect
	virtual bool Connect( const char* address, unsigned short port, long timeout );
	// Destroy the socket
	virtual void Disconnect();
	// Callback indicated we're not needed anymore
	void CleanupAfterThread();

	// Send some data
	bool Send( const void* data, long size );
	// Receive some data
	long Recv( void* buffer, long bytes );
	bool Recv( bf_read& bf );
	// Handles multi packet stuff, please free the result
	bf_read* Response();
	
	// Async abstraction
	virtual bool Perform( bool async );
	virtual void Wait( bool cancel );
	virtual bool Thread() = 0;
	static DWORD WINAPI Thunk( PVOID param );

	// Do the whole challenge thing for us and finish by sending the final request
	// Use with A2S_RULES and A2S_PLAYER
	bool Challenge( unsigned char request );

private:
	sockaddr_in _addr;
	SOCKET _socket;
	HANDLE _thread;
	bool _deleteme;
};

}
