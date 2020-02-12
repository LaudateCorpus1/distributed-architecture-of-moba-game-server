#ifndef __LUA_WRAPPER_H__
#define __LUA_WRAPPER_H__

#include <lua.hpp>
#include <string>

class lua_wrapper
{
public:
	static void Init();
	static void Exit();

	static lua_State* lua_state();

	//ִ��lua�ļ�
	static bool DoFile(const std::string& luaFile);
	//���ýű�����
	static int ExeScriptHandle(int handle, int numArgs);
	//�Ƴ��ű�����
	static void RemoveScriptHandle(int handle);
	static void AddSearchPath(const std::string& path);
	//����C/C++����
	static void ExportFunc2Lua(
		const char* name,
		int (*func)(lua_State*));

};



#endif // !__LUA_WRAPPER_H__
