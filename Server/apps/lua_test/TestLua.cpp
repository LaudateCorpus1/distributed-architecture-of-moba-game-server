#include<iostream>
#include<string>

#include "proto/game.pb.h"
using namespace std; 

#include "../../lua_wrapper/lua_wrapper.h"
#include "../../netbus/Netbus.h"
#include "../../utils/logger/logger.h"



int main(int argc, char** argv)
{
	Netbus::Instance()->Init();
	lua_wrapper::Init();
	
	//����
	if (argc != 3)
	{
		string searchPath = "../../apps/lua_test/scripts/";
		lua_wrapper::AddSearchPath(searchPath);
		lua_wrapper::DoFile(searchPath+"main.lua");
	}
	else
	{
		string searchPath = argv[1];
		if (*(searchPath.end() - 1) != '/')
		{
			searchPath += "/";		
		}
		lua_wrapper::AddSearchPath(searchPath);
		lua_wrapper::DoFile(searchPath + argv[2]);
	}



	//����
	Netbus::Instance()->Run();



	//�˳�
	lua_wrapper::Exit();
	log_debug("�������˳�");
	system("pause");
	return 0;
}