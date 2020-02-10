#include "Netbus.h"
#include <uv.h>
#include "../../netbus/protocol/WebSocketProtocol.h"
#include "../../netbus/protocol/TcpPackageProtocol.h"
#include "../../netbus/session/TcpSession.h"

#include "protocol/CmdPackageProtocol.h"
#include "service/ServiceManager.h"

#include "../utils/logger/logger.h"
#include "session/UdpSession.h"

#pragma region ��������

void OnRecvCommond(AbstractSession& session, unsigned char* body, const int len);

void OnWebSocketRecvData(TcpSession* session);

void OnTcpRecvData(TcpSession* session);

#pragma endregion

struct UdpRecvBuf
{
	char* data;
	int maxRecvLen;
};

#pragma region �ص�����

extern "C"
{
	#pragma region Tcp_&_Websocket
	//�ر����ӻص�
	static void tcp_close_cb(uv_handle_t* handle) {
		log_debug("�û��Ͽ�����");

		auto session = (TcpSession*)handle->data;
		TcpSession::Destory(session);
	}

	//�Ͽ����ӵĻص�
	static void tcp_shutdown_cb(uv_shutdown_t* req, int status) {
		uv_close((uv_handle_t*)(req->handle), tcp_close_cb);
	}

	//Tcp�����ַ����ռ�
	static void tcp_str_alloc(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf) {

		auto session = (TcpSession*)handle->data;

		if (session->recved < RECV_LEN)
		{
			//session�ĳ���Ϊ RECV_LEN ��recvBuf��û�д���
			*buf = uv_buf_init(session->recvBuf + session->recved,RECV_LEN - session->recved); 
		}
		else
		{// recvBuf�����ˣ����ǻ�û�ж��� 

		
			if (session->long_pkg == NULL)
			{// �����û��new�ռ�

				switch (session->socketType)
				{
				case SocketType::TcpSocket:
					break;
				#pragma region WebSocketЭ��
				case SocketType::WebSocket:
					
					if (session->isWebSocketShakeHand)
					{// �չ���
						int pkgSize;
						int headSize;
						WebSocketProtocol::ReadHeader((unsigned char*)session->recvBuf, session->recved, &pkgSize, &headSize);
					
						session->long_pkg_size = pkgSize;
						session->long_pkg = (char*)malloc(pkgSize);
					
						memcpy(session->long_pkg, session->recvBuf, session->recved);
					}
					break;
				#pragma endregion

				}
			}

			*buf = uv_buf_init(session->long_pkg + session->recved, session->long_pkg_size - session->recved);
		}
		
	}

	//��ȡ������Ļص�
	static void tcp_after_read(uv_stream_t* stream,
		ssize_t nread,
		const uv_buf_t* buf) {

		auto session = (TcpSession*) stream->data;

		//���ӶϿ�
		if (nread < 0) {
			log_debug("���ӶϿ�");
			session->Close();
			return;
		}

		session->recved += nread;

		switch (session->socketType)
		{

		#pragma region TcpЭ��

		case SocketType::TcpSocket:
			OnTcpRecvData(session);
			break;

		#pragma endregion

		#pragma region WebSocketЭ��	
		case SocketType::WebSocket: 

			if (session->isWebSocketShakeHand == 0)
			{	//	shakeHand
				if (WebSocketProtocol::ShakeHand(session, session->recvBuf, session->recved))
				{	//���ֳɹ�
					log_debug("���ֳɹ�");
					session->isWebSocketShakeHand = 1;
					session->recved = 0;
				}
			}
			else//	recv/send Data
			{
				OnWebSocketRecvData(session);
			}
			break;
		#pragma endregion

		default:
			break;
		}
	}

	//TCP���û����ӽ���
	static void TcpOnConnect(uv_stream_t* server, int status)
	{
#pragma region ����ͻ���

		auto session = TcpSession::Create();

		auto pNewClient = &session->tcpHandle;
		pNewClient->data = session;

		//���¿ͻ���TCP���Ҳ���뵽�¼�ѭ����
		uv_tcp_init(uv_default_loop(), pNewClient);
		//�ͻ��˽�������� 
		uv_accept(server,(uv_stream_t*) pNewClient);

#pragma region ��ȡ������IP�Ͷ˿�

		sockaddr_in addr;
		int len = sizeof(addr);
		uv_tcp_getpeername(pNewClient, (sockaddr*)&addr, &len);
		uv_ip4_name(&addr, session->clientAddress, 64);
		session->clientPort = ntohs(addr.sin_port);
		//����socket����
		session->socketType = *((SocketType*)&server->data);
		log_debug("new client comming:\t%s:%d", session->clientAddress, session->clientPort);

#pragma endregion


		//��ʼ������Ϣ
		uv_read_start((uv_stream_t*)pNewClient, tcp_str_alloc, tcp_after_read);
#pragma endregion


	} 
	
	#pragma endregion

	#pragma region Udp

	//Udp�����ַ����ռ�
	static void udp_str_alloc(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf)
	{
		suggested_size = (suggested_size < 8192) ? 8192 : suggested_size;

		auto pBuf = (UdpRecvBuf*)handle->data;
		if (pBuf->maxRecvLen < suggested_size)
		{// ������ǰ�ռ䲻��
			if (pBuf->data)
			{
				free(pBuf->data);
				pBuf->data = NULL;
			}
			pBuf->data = (char*)malloc(suggested_size);
			pBuf->maxRecvLen = suggested_size;
		
		}

		buf->base = pBuf->data;
		buf->len = suggested_size;
	}

	//Udp�����ַ������
	static void udp_after_recv(uv_udp_t* handle,
		ssize_t nread,
		const uv_buf_t* buf,
		const struct sockaddr* addr,
		unsigned flags)
	{
		UdpSession us;
		us.udp_handler = (uv_udp_t*)handle;
		us.addr = addr;
		us.clientPort = ntohs(((struct sockaddr_in*)addr)->sin_port);
		uv_ip4_name((struct sockaddr_in*)addr, us.clientAddress, 128);

		log_debug("ip: %s:%d nread = %d", us.clientAddress,us.clientPort, nread);

		OnRecvCommond(us,(unsigned char*)buf->base,buf->len);
	}

	#pragma endregion

	
}

#pragma endregion


static void OnRecvCommond(AbstractSession& session,unsigned char* body,const int len)
{
	//test
	log_debug("client command!!");

	CmdPackage* msg=NULL;
	if (CmdPackageProtocol::DecodeCmdMsg(body, len, msg))
	{
		if (!ServiceManager::OnRecvCmdPackage(&session, msg))
		{
			session.Close();
		}

		//�ͷ�����
		CmdPackageProtocol::FreeCmdMsg(msg);
	}
	else
	{
		log_debug("����ʧ��");
	}
}


#pragma region WebSocketProtocol


static void OnWebSocketRecvData(TcpSession* session)
{
	auto pkgData = (unsigned char*)(session->long_pkg != NULL ? session->long_pkg : session->recvBuf);

	while (session->recved > 0)
	{
		//�Ƿ��ǹرհ�
		if (pkgData[0] == 0x88) {
			log_debug("�յ��رհ�");
			session->Close();
			return;
		}

		int pkgSize;
		int headSize;
		if (!WebSocketProtocol::ReadHeader(pkgData, session->recved, &pkgSize, &headSize))
		{// ��ȡ��ͷʧ��
			log_debug("��ȡ��ͷʧ��");
			break;
		}

		if (session->recved < pkgSize)
		{// û����������
			log_debug("û����������");
			break;
		}

		//����λ�ý���ͷ������֮��
		//bodyλ��������֮��
		unsigned char* body = pkgData + headSize;
		unsigned char* mask = body - 4;

		//�����յ��Ĵ�����
		WebSocketProtocol::ParserRecvData(body, mask, pkgSize - headSize);

		//�����յ����������ݰ�
		OnRecvCommond(*session, body, pkgSize);
	
		if (session->recved > pkgSize)
		{
			memmove(pkgData, pkgData + pkgSize, session->recved - pkgSize);
		}

		//ÿ�μ�ȥ��ȡ���ĳ���
		session->recved -= pkgSize;

		//���������������
		if (session->recved == 0 && session->long_pkg != NULL)
		{
			free(session->long_pkg);
			session->long_pkg = NULL;
			session->long_pkg_size = 0;
		}
	}
}

#pragma endregion

#pragma region TcpProtocol

static void OnTcpRecvData(TcpSession* session)
{
	auto pkgData = (unsigned char*)(session->long_pkg != NULL ? session->long_pkg : session->recvBuf);

	while (session->recved > 0)
	{
		int pkgSize;
		int headSize;
		if (!TcpProtocol::ReadHeader(pkgData, session->recved, &pkgSize, &headSize))
		{// ��ȡ��ͷʧ��
			log_debug("��ȡ��ͷʧ��");
			break;
		}
		if (session->recved < pkgSize)
		{// û����������
			log_debug("û����������");
			break;
		}

		//bodyλ��������֮��
		unsigned char* body = pkgData + headSize;

		//�����յ����������ݰ�
		OnRecvCommond(*session, body, pkgSize-headSize);

		if (session->recved > pkgSize)
		{
			memmove(pkgData, pkgData + pkgSize, session->recved - pkgSize);
		}

		//ÿ�μ�ȥ��ȡ���ĳ���
		session->recved -= pkgSize;

		//���������������
		if (session->recved == 0 && session->long_pkg != NULL)
		{
			free(session->long_pkg);
			session->long_pkg = NULL;
			session->long_pkg_size = 0;
		}
	}
}

#pragma endregion




static Netbus g_instance;
const Netbus* Netbus::Instance()
{
	return &g_instance;
}

void Netbus::StartTcpServer(int port)const
{
	auto listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

	memset(listen, 0, sizeof(uv_tcp_t));

	sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);
	uv_tcp_init(uv_default_loop(), listen);
	auto ret = uv_tcp_bind(listen, (const sockaddr*)&addr, 0);
	if (0 != ret)
	{
		log_debug("bind error");
		free(listen);
		listen = NULL;
		return;
	}

	//ǿת��¼socket����
	listen->data = (void*)SocketType::TcpSocket;

	uv_listen((uv_stream_t*)listen, SOMAXCONN, TcpOnConnect);
	log_debug("Tcp �������ѿ��� %d",port);
}

void Netbus::StartWebSocketServer(int port)const
{
	auto listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

	memset(listen, 0, sizeof(uv_tcp_t));

	sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);
	uv_tcp_init(uv_default_loop(), listen);
	auto ret = uv_tcp_bind(listen, (const sockaddr*)&addr, 0);
	if (0 != ret)
	{
		log_debug("bind error");
		free(listen);
		listen = NULL;
		return;
	}

	//ǿת��¼socket����
	listen->data = (void*)SocketType::WebSocket;

	uv_listen((uv_stream_t*)listen, SOMAXCONN, TcpOnConnect);
	log_debug("WebSocket �������ѿ��� %d", port);
}

void Netbus::StartUdpServer(int port) const
{
	auto server = (uv_udp_t*)malloc(sizeof(uv_udp_t));
	memset(server, 0, sizeof(uv_udp_t));

	uv_udp_init(uv_default_loop(), server);

	server->data = malloc(sizeof(UdpRecvBuf));
	memset(server->data, 0, sizeof(UdpRecvBuf));

	sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);
	auto ret = uv_udp_bind(server, (const sockaddr*)&addr, 0);

	uv_udp_recv_start(server, udp_str_alloc, udp_after_recv);
	log_debug("Udp �������ѿ��� %d", port);
}

void Netbus::Run()const
{
	log_debug("��ʼ����");
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void Netbus::Init() const
{
	CmdPackageProtocol::Init();
	ServiceManager::Init();
	InitAllocers();
}
