#ifndef __SERVICE_EXPORT_TO_LUA_H__
#define __SERVICE_EXPORT_TO_LUA_H__

#include "../netbus/service/AbstractService.h"


struct lua_State;
int register_service_export(lua_State* lua);


class LuaService :
	public AbstractService
{
public:
	unsigned int luaRecvFuncHandle;
	unsigned int luaDisconnectFuncHandle;

	//Session���յ�����ʱ����
	virtual bool OnSessionRecvCmd(const AbstractSession* session, const CmdPackage* package)const override;
	//Session�ر�ʱ����
	virtual bool OnSessionDisconnected(const AbstractSession* session)const override;
};


#endif // !__SERVICE_EXPORT_TO_LUA_H__
