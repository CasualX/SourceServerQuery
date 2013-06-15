#include "stdafx.h"

#include "basequery.h"

namespace ssq
{

class CQueryInfo : public CBaseQuery
{
public:
	CQueryInfo( IServerResponse* cb ) : _cb(cb) { }

	virtual bool Thread();

private:
	IServerResponse* _cb;
};



bool CQueryInfo::Thread()
{
	char request[] = "\xFF\xFF\xFF\xFFTSource Engine Query";
	if ( !Send( request, sizeof(request) ) )
		goto failure;

	bf_read bf;
	if ( !Recv( bf ) || bf.bytes<21 || bf.Read<long>()!=-1 || bf.Read<char>()!=S2A_INFO )
		goto failure;

	gameserver_t gs;
	if ( bf.Read( gs.protocol ) &&
		(gs.name=bf.ReadString()) &&
		(gs.map=bf.ReadString()) &&
		(gs.folder=bf.ReadString()) &&
		(gs.game=bf.ReadString()) &&
		 bf.Read( gs.appid ) &&
		 bf.Read( gs.players ) &&
		 bf.Read( gs.maxplayers ) &&
		 bf.Read( gs.bots ) &&
		 bf.Read( gs.server_type ) &&
		 bf.Read( gs.environment ) &&
		 bf.Read( gs.password ) &&
		 bf.Read( gs.vac ) &&
		 /* No support for The Ship! */
		 (gs.version=bf.ReadString()) &&
		 bf.Read( gs.edf.flags ) &&
		 ( !gs.edf.has_port() || bf.Read( gs.edf.port ) ) &&
		 ( !gs.edf.has_steamid() || bf.Read( gs.edf.steamid ) ) &&
		 ( !gs.edf.has_stv() || ( bf.Read( gs.edf.stv_port ) && ( gs.edf.stv_name = bf.ReadString() ) ) ) &&
		 ( !gs.edf.has_tags() || ( gs.edf.tags = bf.ReadString() ) ) &&
		 ( !gs.edf.has_gameid() || bf.Read( gs.edf.gameid ) ) )
	{
		_cb->ServerInfo( gs );
		return true;
	}

failure:
	_cb->ServerFailed();
	return false;
}


IQuery* ServerInfo( IServerResponse* resp )
{
	IQuery* q = new CQueryInfo( resp );
	return q;
}

}
