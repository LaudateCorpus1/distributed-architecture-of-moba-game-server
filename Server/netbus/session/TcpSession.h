#ifndef __UVSESSION_H__
#define __UVSESSION_H__

#include "AbstractSession.h"
#define RECV_LEN 4096

//Socket����
enum class SocketType
{
	TcpSocket,
	WebSocket
};


class TcpSession :
	public AbstractSession
{
public:
	static TcpSession* Create();
	static void Destory(TcpSession*& session);

	#pragma region AbstractSession

	virtual void Close() override;
	//��������
	virtual void SendData(unsigned char* body, int len) override;
	//��ȡIP��ַ�Ͷ˿�
	virtual const char* GetAddress(int & clientPort) const override;
	//�����Զ����
	virtual void SendCmdPackage(CmdPackage* msg)override;

	// ͨ�� AbstractSession �̳�
	virtual void SendRawPackage(RawPackage* pkg) override;

	//���ڶ����
	virtual void Enable()override;
	virtual void Disable()override;
	#pragma endregion


	void* operator new(size_t size);
	void operator delete(void* mem);

public:
	uv_shutdown_t shutdown;
	uv_tcp_t tcpHandle;
	//��¼��ǰ�Ự�����IP�Ͷ˿�
	char clientAddress[32];
	int clientPort;

	//���ƽ�������
	char recvBuf[RECV_LEN];
	int recved; 


	SocketType socketType;

	//WebSocket������Ƿ��Ѿ�����
	int isWebSocketShakeHand;

	// ��������С���� RECV_LEN �����ݾͱ��������������recvData
	char* long_pkg;
	int long_pkg_size;
private:
	//�Ƿ��Ѿ��رգ��������첽ʱ�����ظ��ر�
	bool isShutDown;

};


void InitAllocers();

#endif // !__UVSESSION_H__



