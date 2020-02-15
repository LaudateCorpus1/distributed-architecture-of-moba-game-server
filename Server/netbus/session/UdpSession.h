#ifndef __UDPSESSION_H__
#define __UDPSESSION_H__

#include "AbstractSession.h"

class UdpSession :
	public AbstractSession
{
public:
	uv_udp_t* udp_handler;
	char clientAddress[32];
	int clientPort;
	const struct sockaddr* addr;

	virtual void Close() override;
	//��������
	virtual void SendData(unsigned char* body, int len) override;
	//��ȡIP��ַ�Ͷ˿�
	virtual const char* GetAddress(int& clientPort) const override;
	//�����Զ����
	virtual void SendCmdPackage(CmdPackage* msg)override;

	// ͨ�� AbstractSession �̳�
	virtual void SendRawPackage(RawPackage* pkg) override;
};


#endif // !__UDPSESSION_H__
