#ifndef __LUA_WRAPPER_H__
#define __LUA_WRAPPER_H__

#include <lua.hpp>

class lua_wrapper
{
public:
	static void Init();
	static void Exit();

	static lua_State* lua_state();

	//ִ��lua�ļ�
	static bool ExeLuaFile(char* luaFilePath);
	//���ýű�����
	static int ExeScriptHandle(int handle, int numArgs);
	//�Ƴ��ű�����
	static void RemoveScriptHandle(int handle);


private:
	//����C/C++����
	static void ExportFunc2Lua(
		const char* name,
		int (*func)(lua_State*));

};



#endif // !__LUA_WRAPPER_H__
