#ifndef __ABSTRACTSERVICE_H__
#define __ABSTRACTSERVICE_H__

class AbstractSession;
struct CmdPackage;

//���������
class AbstractService
{
public:
	//Session���յ�����ʱ����
	virtual bool OnSessionRecvCmd(const AbstractSession* session, const CmdPackage* package)const;
	//Session�ر�ʱ����
	virtual bool OnSessionDisconnected(const AbstractSession* session)const;
};




#endif // !__ABSTRACTSERVICE_H__



