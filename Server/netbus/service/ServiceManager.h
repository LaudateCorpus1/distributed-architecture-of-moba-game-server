#ifndef __SERVICEMANAGER_H__
#define __SERVICEMANAGER_H__
#endif // !__SERVICEMANAGER_H__
class AbstractService;
struct RawPackage;
class ServiceManager
{
public:
	static void Init();
	//ע�����
	static bool RegisterService(int serviceType, AbstractService* service);
	//Netbus�յ����Ļص�������true��ʾ���������ˣ�����Ҫ�ر�socket
	static bool OnRecvRawPackage(AbstractSession* session, const RawPackage* package);
	//�ͻ��˶Ͽ��Ļص�
	static void OnSessionDisconnected(const AbstractSession* session);

	//�ͻ������ӳɹ��ص�
	static void OnSessionConnect(const AbstractSession* session);
};

