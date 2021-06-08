#ifndef __ABSTRACTSERVICE_H__
#define __ABSTRACTSERVICE_H__

class AbstractSession;
struct CmdPackage;
struct RawPackage;

//���������
class AbstractService
{
public:
	bool useRawPackage;

	AbstractService();

	virtual bool OnSessionRecvRawPackage(const AbstractSession* session, const RawPackage* package)const;

	//Session���յ�����ʱ����
	virtual bool OnSessionRecvCmdPackage(const AbstractSession* session, const CmdPackage* package)const;
	
	//Session�Ͽ�ĳ�����������ʱ���ùر�ʱ����
	virtual bool OnSessionDisconnected(const AbstractSession* session,const int & serviceType)const;

	//Session���ӵ���������ʱ�����
	virtual void OnSessionConnected(const AbstractSession* session, const int& serviceType)const;

	virtual ~AbstractService() {}
};




#endif // !__ABSTRACTSERVICE_H__



