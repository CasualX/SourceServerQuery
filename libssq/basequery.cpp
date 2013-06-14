#include "stdafx.h"

#include "basequery.h"

namespace ssq
{

void CBaseQuery_Cleanup()
{
	::WSACleanup();
}
CBaseQuery::CBaseQuery() : _socket(INVALID_SOCKET), _thread(NULL)
{
	// Delayed init stuff
	static bool once = false;
	if ( !once )
	{
		once = true;
		WSADATA data;
		if ( ::WSAStartup( WINSOCK_VERSION, &data ) || data.wVersion!=WINSOCK_VERSION )
		{
			// Uhm I don't think I need to call this if WSAStartup fails...
			::WSACleanup();
			throw std::exception( "Failed to initialize WSA!" );
		}
		atexit( &CBaseQuery_Cleanup );
	}
}
CBaseQuery::~CBaseQuery()
{
	Disconnect();
}
bool CBaseQuery::Connect( const char* address, long timeout )
{
	if ( _socket!=INVALID_SOCKET || _thread )
	{
		// Already busy... Call Disconnect() first!
		// FIXME! Should we call it instead!?
		return false;
	}

	// Create buffer to separate ip from port
	// NOTE! MUST be an IP address!

	char buf[64];
	unsigned int port = 0;
	strcpy_s( buf, address );
	for ( size_t i = 0; buf[i]; i++ )
	{
		if ( buf[i]==':' )
		{
			buf[i] = 0;
			port = atoi( buf+i+1 );
			break;
		}
	}

	// Setup dest addr

	unsigned long ip = inet_addr( buf );
	if ( ip==INADDR_ANY || ip==INADDR_BROADCAST )
		return false;
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = ip;
	_addr.sin_port = htons(port);
	
	// Setup local addr

	::sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = 0;

	// create the socket

	if ( ( _socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) )==INVALID_SOCKET )
		return false;
	
	if ( ::bind( _socket, (sockaddr*)&local, sizeof(local) ) )
		return false;

	if ( ::setsockopt( _socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout) ) )
		return false;

	return true;
}
void CBaseQuery::Disconnect()
{
	// There's a race condition if closesocket gets called *just* before recv on the networking thread...
	if ( _socket!=INVALID_SOCKET && ::closesocket( _socket ) )
	{
		// Nothing to see here, move along!
		// But seriously, do something if this fails?
	}
	// If this is called while we're still working, wait for it
	// NOTE! We shouldn't have to wait long, closesocket should cancel the blocking calls!
	if ( _thread )
	{
		::WaitForSingleObject( _thread, INFINITE );
		::CloseHandle( _thread );
		_thread = NULL;
	}
}
bool CBaseQuery::Send( const void* data, long bytes )
{
	return ::sendto( _socket, (const char*)data, bytes, 0, (const sockaddr*)&_addr, sizeof(_addr) )!=SOCKET_ERROR;
}
long CBaseQuery::Recv( void* buf, long bytes )
{
	return ::recv( _socket, (char*)buf, bytes, 0 );
}
bool CBaseQuery::Perform( bool async )
{
	// Need an active socket and not already performing
	// Yeah, yeah, not thread safe, shut it.
	if ( _socket==INVALID_SOCKET || _thread )
	{
		return false;
	}

	if ( async )
	{
		_thread = ::CreateThread( NULL, 0, &Thunk, this, 0, NULL );
		return _thread!=NULL;
	}
	else
	{
		return Thread();
	}
}
void CBaseQuery::Wait( bool cancel )
{
	if ( cancel )
	{
		Disconnect();
	}
	else if ( _thread )
	{
		// Timeout is specified by the socket, so we wait infinitely long.
		::WaitForSingleObject( _thread, INFINITE );
	}
}
DWORD CBaseQuery::Thunk( PVOID param )
{
	CBaseQuery* q = (CBaseQuery*)param;
	return q->Thread()==false;
}
bool CBaseQuery::Challenge( unsigned char type )
{
	// Only these two queries should call this!
	assert( type==A2S_RULES || type==A2S_PLAYER );

	unsigned char buf[16];
	long bytes, challenge;

	// Request challenge
	*(long*)(buf+0) = ~0;
	*(char*)(buf+4) = type;
	*(long*)(buf+5) = ~0;
	if ( !Send( buf, 9 ) )
		return false;
	
	// Process challenge
	bytes = Recv( buf, sizeof(buf) );
	if ( bytes!=9 || *(long*)(buf+0)!=~0 || buf[4]!=S2C_CHALLENGE )
		return false;
	challenge = *(long*)(buf+5);

	// Request response
	*(long*)(buf+0) = ~0;
	*(char*)(buf+4) = type;
	*(long*)(buf+5) = challenge;
	if ( !Send( buf, 9 ) )
		return false;

	return true;
}

}
