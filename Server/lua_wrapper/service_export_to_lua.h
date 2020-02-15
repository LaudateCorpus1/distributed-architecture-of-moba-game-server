#ifndef __SERVICE_EXPORT_TO_LUA_H__
#define __SERVICE_EXPORT_TO_LUA_H__

#include "../netbus/service/AbstractService.h"


struct lua_State;
int register_service_export(lua_State* lua);


class LuaService :
	public AbstractService
{
public:
	unsigned int luaRecvCmdPackageHandle;
	unsigned int luaDisconnectFuncHandle;
	unsigned int luaRecvRawPackageHandle;

	//Session���յ�RawPackageʱ����
	virtual bool OnSessionRecvRawPackage(const AbstractSession* session, const RawPackage* package)const override;

	//Session���յ�CmdPackageʱ����
	virtual bool OnSessionRecvCmdPackage(const AbstractSession* session, const CmdPackage* package)const override;
	//Session�ر�ʱ����
	virtual bool OnSessionDisconnected(const AbstractSession* session)const override;
};


#endif // !__SERVICE_EXPORT_TO_LUA_H__
