#ifndef __UDPSESSION_H__
#define __UDPSESSION_H__

#include "AbstractSession.h"

class UdpSession :
	public AbstractSession
{
private:
	uv_udp_t* udp_handler;
	const sockaddr* addr;

public:
	char clientAddress[64];
	int clientPort;


	void Init(uv_udp_t* udpHandle,const sockaddr* addr);

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
