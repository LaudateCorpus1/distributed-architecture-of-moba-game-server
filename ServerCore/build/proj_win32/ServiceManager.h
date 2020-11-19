#ifndef __SERVICEMANAGER_H__
#define __SERVICEMANAGER_H__
#endif // !__SERVICEMANAGER_H__

class ServiceManager
{
public:
	static void Init();
	//ע�����
	static bool RegisterService(int serviceType, AbstractService* service);
	//Netbus�յ����Ļص�������true��ʾ���������ˣ�����Ҫ�ر�socket
	static bool OnRecvCmdPackage(const AbstractSession* session, const CmdPackage* package);
	//�ͻ��˶Ͽ��Ļص�
	static void OnSessionDisconnected(const AbstractSession* session);
};

