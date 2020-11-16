#include "Netbus.h"
#include <uv.h>
#include "../../netbus/protocol/WebSocketProtocol.h"
#include "../../netbus/protocol/TcpPackageProtocol.h"
#include "../../netbus/session/TcpSession.h"

#include "service/ServiceManager.h"

#include "../utils/logger/logger.h"
#include "session/UdpSession.h"

#include "../utils/cache_alloc/small_alloc.h"

#define my_alloc small_alloc
#define my_free small_free

#pragma region ��������

void OnRecvCommond(AbstractSession* session, unsigned char* body, const int len);

void OnWebSocketRecvData(TcpSession* session);

void OnTcpRecvData(TcpSession* session);

#pragma endregion

struct UdpRecvBuf
{
	char* data;
	int maxRecvLen;
};

struct TcpConnectInfo
{
	TcpConnectedCallback cb;
	void* data;
};

struct TcpListenInfo
{
	TcpListenCallback cb;
	SocketType socketType;
	void* data;
};

#pragma region �ص�����

extern "C"
{
#pragma region Tcp_&_Websocket

	//Tcp�����ַ����ռ�
	static void tcp_str_alloc(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf) {

		auto session = (TcpSession*)handle->data;

		if (session->recved < RECV_LEN)
		{
			//session�ĳ���Ϊ RECV_LEN ��recvBuf��û�д���
			*buf = uv_buf_init(session->recvBuf + session->recved, RECV_LEN - session->recved);
		}
		else
		{// recvBuf�����ˣ����ǻ�û�ж��� 


			if (session->long_pkg == NULL)
			{// �����û��new�ռ�
				int pkgSize;
				int headSize;
				switch (session->socketType)
				{
				case SocketType::TcpSocket:
					TcpProtocol::ReadHeader((unsigned char*)session->recvBuf, session->recved, &pkgSize, &headSize);
					break;
				case SocketType::WebSocket:
					WebSocketProtocol::ReadHeader((unsigned char*)session->recvBuf, session->recved, &pkgSize, &headSize);
					break;
				}
				session->long_pkg_size = pkgSize;
				session->long_pkg = (char*)malloc(pkgSize);

				memcpy(session->long_pkg, session->recvBuf, session->recved);
			}

			*buf = uv_buf_init(session->long_pkg + session->recved, session->long_pkg_size - session->recved);
		}

	}

	//��ȡ������Ļص�
	static void tcp_after_read(uv_stream_t* stream,
		ssize_t nread,
		const uv_buf_t* buf) {

		auto session = (TcpSession*)stream->data;

		//���ӶϿ�
		if (nread < 0) {
			session->Close();
			return;
		}

		session->recved += nread;

		switch (session->socketType)
		{
		case SocketType::TcpSocket:
			OnTcpRecvData(session);
			break;
		case SocketType::WebSocket:
			#pragma region WebSocketЭ��	
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
			#pragma endregion
			break;

		default:
			break;
		}
	}

	//TCP���û����ӽ���
	static void TcpOnConnect(uv_stream_t* server, int status)
	{
		auto info = (TcpListenInfo*)server->data;
		auto session = TcpSession::Create();

		//��ʼ��session����Ϣ����socket���ͣ�ip���˿ڣ�������libuv�¼�ѭ��
		session->Init(info->socketType);
		
		//�ͻ��˽�������� 
		uv_accept(server, (uv_stream_t*)&session->tcpHandle);

		//�ص�
		if(info->cb)
			info->cb(session, info->data);

		//Session���ӳɹ��Ļص�
		ServiceManager::OnSessionConnect(session);

		//��ʼ������Ϣ
		uv_read_start((uv_stream_t*)&session->tcpHandle, tcp_str_alloc, tcp_after_read);
		
	}

#pragma endregion

#pragma region Udp

	//Udp�����ַ����ռ�
	static void udp_str_alloc(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf)
	{
		suggested_size = (suggested_size < 4096) ? 4096 : suggested_size;

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
		if (nread <= 0)
			return;
		UdpSession us;

		us.Init(handle, addr);

		OnRecvCommond(&us, (unsigned char*)buf->base, nread);
	}

#pragma endregion

	void AfterUdpSend(uv_udp_send_t* req, int status)
	{
		if (status)
		{
			log_error("udp����ʧ��");
		}
		my_free(req);
	}
}

#pragma endregion


static void OnRecvCommond(AbstractSession* session, unsigned char* body, const int bodyLen)
{
	RawPackage raw;
	if (CmdPackageProtocol::DecodeBytesToRawPackage(body, bodyLen, &raw))
	{
		//���÷�������������������
		if (!ServiceManager::OnRecvRawPackage(session, &raw))
		{
			session->Close();
		}		
	}
	else
	{
		int port;
		auto ip = session->GetAddress(port);
		log_error("[%s:%d]�����İ�����ʧ��",ip,port);
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
		OnRecvCommond(session, body, pkgSize);

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
	auto tcpPkgData = (unsigned char*)(session->long_pkg != NULL ? session->long_pkg : session->recvBuf);

	while (session->recved > 0)
	{
		int tcpPkgSize;
		int tcpHeadSize;
		if (!TcpProtocol::ReadHeader(tcpPkgData, session->recved, &tcpPkgSize, &tcpHeadSize))
		{// ��ȡ��ͷʧ��
			int port;
			auto ip = session->GetAddress(port);
			log_error("TCP��ȡ��ͷʧ��,�������ԣ�[%s:%d]",ip,port);
			break;
		}

		if (tcpPkgSize < tcpHeadSize) {
			session->Close();
			int port;
			auto ip = session->GetAddress(port);
			log_warning("�յ�[%s:%d]�����ķǷ����������СС�ڰ�ͷ����session�ѹر�",ip,port);
			break;
		}

		if (session->recved < tcpPkgSize)
		{// û����������
			log_debug("û����������");
			break;
		}

		//bodyλ��������֮��
		unsigned char* body = tcpPkgData + tcpHeadSize;

		//�����յ����������ݰ�
		OnRecvCommond(session, body, tcpPkgSize - tcpHeadSize);

		if (session->recved > tcpPkgSize)
		{
			memmove(tcpPkgData, tcpPkgData + tcpPkgSize, session->recved - tcpPkgSize);
		}

		//ÿ�μ�ȥ��ȡ���ĳ���
		session->recved -= tcpPkgSize;

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

static void after_connect(uv_connect_t* handle, int status)
{
	auto session = (AbstractSession*)handle->handle->data;
	auto info = (TcpConnectInfo*)handle->data;

	//������쳣���
	if (status) 
	{
		if (info)
		{
			if(info->cb)
				info->cb(1, NULL, info->data);
			my_free(info);
			info = NULL;
		}
		if(session)
			session->Close();
		my_free(handle);
		return;
	}

	//���һ������
	if (info)
	{
		if (info->cb)
		{
			info->cb(0, session, info->data);
		}
		my_free(info);
		info = NULL;
	}
	uv_read_start(handle->handle, tcp_str_alloc, tcp_after_read);
	my_free(handle);
}



static Netbus g_instance;
Netbus* Netbus::Instance()
{
	return &g_instance;
}

Netbus::Netbus()
{
	this->udpHandle = NULL;
}

void Netbus::TcpListen(int port, TcpListenCallback callback, void* udata)const
{
	auto listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

	memset(listen, 0, sizeof(uv_tcp_t));

	sockaddr_in addr;
	auto ret = uv_ip4_addr("0.0.0.0", port, &addr);
	if (ret)
	{
		log_error("uv_ip4_addr error��%d", port);
		free(listen);
		listen = NULL;
		return;

	}

	uv_tcp_init(uv_default_loop(), listen);
	ret = uv_tcp_bind(listen, (const sockaddr*)&addr, 0);
	if (ret)
	{
		log_error("Tcp �˿ڰ�ʧ�ܣ�%d",port);
		free(listen);
		listen = NULL;
		return;
	}

	
	auto pInfo = new TcpListenInfo();
	pInfo->cb = callback;
	pInfo->data = udata;
	pInfo->socketType = SocketType::TcpSocket;

	//ǿת��¼TcpListenInfo����
	listen->data = (void*)pInfo;

	uv_listen((uv_stream_t*)listen, SOMAXCONN, TcpOnConnect);
}

void Netbus::WebSocketListen(int port, TcpListenCallback callback, void* udata)const
{
	auto listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

	memset(listen, 0, sizeof(uv_tcp_t));

	sockaddr_in addr;
	auto ret = uv_ip4_addr("0.0.0.0", port, &addr);
	if (ret)
	{
		log_error("uv_ip4_addr error��%d", port);
		free(listen);
		listen = NULL;
		return;

	}

	uv_tcp_init(uv_default_loop(), listen);
	ret = uv_tcp_bind(listen, (const sockaddr*)&addr, 0);
	if (0 != ret)
	{
		log_error("WebSocket �˿ڰ�ʧ�ܣ�%d",port);
		free(listen);
		listen = NULL;
		return;
	}


	auto pInfo = new TcpListenInfo();
	pInfo->cb = callback;
	pInfo->data = udata;
	pInfo->socketType = SocketType::WebSocket;

	//ǿת��¼TcpListenInfo����
	listen->data = (void*)pInfo;

	uv_listen((uv_stream_t*)listen, SOMAXCONN, TcpOnConnect);
}

void Netbus::UdpListen(int port)
{
	if (this->udpHandle)
	{
		log_warning("��ʱ��֧��ͬʱ�������udp");
		return;
	}

	sockaddr_in addr;
	auto ret = uv_ip4_addr("0.0.0.0", port, &addr);
	if (ret)
	{
		log_error("uv_ip4_addr error��%d", port);
		return;
	}
	auto server = (uv_udp_t*)malloc(sizeof(uv_udp_t));
	memset(server, 0, sizeof(uv_udp_t));

	uv_udp_init(uv_default_loop(), server);

	server->data = malloc(sizeof(UdpRecvBuf));
	memset(server->data, 0, sizeof(UdpRecvBuf));
	ret = uv_udp_bind(server, (const sockaddr*)&addr, 0);

	if (ret)
	{
		log_error("Udp ��ʧ��");
		if (server)
		{
			if (server->data)
				free(server->data);
			free(server);
			server = NULL;
			return;
		}
	}

	this->udpHandle = server;

	uv_udp_recv_start(server, udp_str_alloc, udp_after_recv);
}

void Netbus::Run()const
{
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void Netbus::Init() const
{
	ServiceManager::Init();
	InitAllocers();
}

void Netbus::TcpConnect(const char* serverIp, int port, TcpConnectedCallback callback, void* udata)const
{
	sockaddr_in addr;
	auto ret = uv_ip4_addr(serverIp, port, &addr);
	if (ret)
	{
		log_error("uv_ip4_addr error��%s:%d", serverIp,port);
		return;

	}

	//����Session����ʼ�����ֵ����socketType,ip,port,isCliet��������libuv�¼�ѭ��
	auto s = TcpSession::Create();
	s->Init(SocketType::TcpSocket, serverIp, port, true);

	auto req = (uv_connect_t*)my_alloc(sizeof(uv_connect_t));
	memset(req, 0, sizeof(uv_connect_t));

	auto info = (TcpConnectInfo*)my_alloc(sizeof(TcpConnectInfo));
	memset(info, 0, sizeof(TcpConnectInfo));

	info->cb = callback;
	info->data = udata;
	req->data = info;

	ret = uv_tcp_connect(req, &s->tcpHandle, (struct sockaddr*) & addr, after_connect);
}

void Netbus::UdpSendTo(char* ip, int port, unsigned char* body, int len)
{
	auto wbuf = uv_buf_init((char*)body, len);
	auto req = (uv_udp_send_t*)my_alloc(sizeof(uv_udp_send_t));

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = inet_addr(ip);

	uv_udp_send(req, (uv_udp_t*)this->udpHandle, &wbuf, 1,(const SOCKADDR*) &addr, AfterUdpSend);
}
 