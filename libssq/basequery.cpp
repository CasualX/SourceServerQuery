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
bool CBaseQuery::Connect( const char* address, unsigned short portnr, long timeout )
{
	if ( _socket!=INVALID_SOCKET || _thread )
	{
		// Already busy... Call Disconnect() first!
		// FIXME! Should we call it instead!?
		return false;
	}

	// Separate host from port

	char buf[64];
	const char* port = nullptr;
	for ( unsigned int i = 0; address[i]; i++ )
	{
		char& c = buf[i] = address[i];
		if ( c==':' )
		{
			c = 0;
			port = address+i+1;
			goto found_port;
		}
	}
	// A port is required!
	if ( !portnr )
		return false;
found_port:

	// Lookup host name

	addrinfo hint = {
		AI_PASSIVE,
		AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP,
		0,
		nullptr,
		nullptr,
		nullptr,
	};

	addrinfo* ptr;
	if ( ::getaddrinfo( buf, port, &hint, &ptr ) || !ptr )
		return false;

	// Create the socket

	_socket = ::socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol );
	if ( _socket!=INVALID_SOCKET )
	{
		_addr = *(sockaddr_in*) ptr->ai_addr;
		if ( portnr )
			_addr.sin_port = ::htons( portnr );

		// Bind our socket
		// Ignoring errors here as these are unlikely to fail, they'll be reported later anyway...

		sockaddr_in local;
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = INADDR_ANY;
		local.sin_port = 0;

		::bind( _socket, (const sockaddr*)&local, sizeof(local) );
		::setsockopt( _socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout) );

		::freeaddrinfo( ptr );
		return true;
	}

	::freeaddrinfo( ptr );

#ifdef _DEBUG
	int err = ::WSAGetLastError();
#endif // _DEBUG
	return false;
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
#ifdef _DEBUG
		DWORD code;
		::GetExitCodeThread( _thread, &code );
#endif // _DEBUG
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
bool CBaseQuery::Recv( bf_read& bf )
{
	bf.bytes = ::recv( _socket, bf.raw, sizeof(bf.raw), 0 );
	bf.it = bf.raw;
	return bf.bytes>0;
}
bool CBaseQuery::Read( bf_read& bf )
{
	// Buffer, 1260 is steam max packet size before split happens
	unsigned char temp[1260];
	// Buffers for each packet so we can put them in order
	unsigned char* packets[4] = { nullptr, nullptr, nullptr, nullptr };

	do
	{
		// Read a packet
		Recv( temp, sizeof(temp) );

	}
	while ( true );
	return false;
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
		bool s = Thread();
#ifdef _DEBUG
		// Testing to see what went wrong...
		int err = ::WSAGetLastError();
#endif // _DEBUG
		return s;
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
	q->Thread();
#ifdef _DEBUG
	return ::WSAGetLastError();
#else
	return 0;
#endif // _DEBUG
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
