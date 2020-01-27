#include "Netbus.h"
#include <uv.h>
#include "UvSession.h"
#include "WebSocketProtocol.h"


#pragma region �ص�����

extern "C"
{
	//�ر����ӻص�
	static void close_cb(uv_handle_t* handle) {
		printf("�û��Ͽ�����\n");

		auto session = (UvSession*)handle->data;
		UvSession::Destory(session);
	}

	//�Ͽ����ӵĻص�
	static void shutdown_cb(uv_shutdown_t* req, int status) {
		uv_close((uv_handle_t*)(req->handle), close_cb);
	}

	//�����ַ����ռ�
	static void string_alloc(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf) {

		auto session = (UvSession*)handle->data;
		*buf = uv_buf_init(
			session->recvBuf + session->recved,
			RECV_LEN - session->recved); 
	}

	//��ȡ������Ļص�
	static void after_read(uv_stream_t* stream,
		ssize_t nread,
		const uv_buf_t* buf) {

		auto session = (UvSession*) stream->data;

		//���ӶϿ�
		if (nread < 0) {
			session->Close();
			return;
		}

		session->recved += nread;


		switch (session->socketType)
		{
		case SocketType::TcpSocket:
			break;
		case SocketType::WebSocket: 

			if (session->isWebSocketShakeHand == 0)
			{	//	shakeHand
				if (WebSocketProtocol::ShakeHand(session, session->recvBuf, session->recved))
				{	//���ֳɹ�
					session->isWebSocketShakeHand = 1;
				}
			}
			else//	recv/send Data
			{

			}
			break;
		default:
			break;
		}


		//�ַ�����β
		//buf->base[nread] = 0;
		//printf("recv %d\t%s\n", nread, buf->base);

		////������Ϣ
		//session->SendData((unsigned char*)buf->base, buf->len);

		//session->Close();
	}

	//TCP���û����ӽ���
	static void OnConnect(uv_stream_t* server, int status)
	{
#pragma region ����ͻ���

		auto session = UvSession::Create();

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
		printf("new client comming:\t%s:%d\n", session->clientAddress, session->clientPort);

#pragma endregion


		//��ʼ������Ϣ
		uv_read_start((uv_stream_t*)pNewClient, string_alloc, after_read);
#pragma endregion


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
		printf("bind error\n");
		free(listen);
		listen = nullptr;
		return;
	}

	//ǿת��¼socket����
	listen->data = (void*)SocketType::TcpSocket;

	uv_listen((uv_stream_t*)listen, SOMAXCONN, OnConnect);
	printf("Tcp �������ѿ���\n");
}

void Netbus::StartWebsocketServer(int port)const
{
	auto listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

	memset(listen, 0, sizeof(uv_tcp_t));

	sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);
	uv_tcp_init(uv_default_loop(), listen);
	auto ret = uv_tcp_bind(listen, (const sockaddr*)&addr, 0);
	if (0 != ret)
	{
		printf("bind error\n");
		free(listen);
		listen = nullptr;
		return;
	}

	//ǿת��¼socket����
	listen->data = (void*)SocketType::WebSocket;

	uv_listen((uv_stream_t*)listen, SOMAXCONN, OnConnect);
	printf("Tcp �������ѿ���\n");
}

void Netbus::Run()const
{
	printf("��ʼ����\n");
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void Netbus::Init() const
{
	InitSessionAllocer();
}
