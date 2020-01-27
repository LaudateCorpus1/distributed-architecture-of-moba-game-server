#include "UvSession.h"
#include "../../utils/cache_alloc.h"

#pragma region �ڴ����

#define SESSION_CACHE 3000
#define	WRITEREQ_CACHE 2048

//��ʼ���ڴ������
static cache_allocer* sessionAllocer = nullptr;
static cache_allocer* wrAllocer = nullptr;
void InitSessionAllocer()
{
	if (nullptr == sessionAllocer)
	{
		sessionAllocer = create_cache_allocer(SESSION_CACHE, sizeof(UvSession));
	}	
	if (nullptr == wrAllocer)
	{
		wrAllocer = create_cache_allocer(WRITEREQ_CACHE, sizeof(uv_write_t));
	}
}

#pragma endregion


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

	//���д����Ļص�
	static void after_write(uv_write_t* req, int status)
	{
		//���д����ɹ�
		if (status == 0)
		{
			printf("write success\n");
		}
		cache_free(wrAllocer, req);
	}
}
#pragma endregion


#pragma region Static

UvSession* UvSession::Create()
{
	//�ֶ�����
	auto temp = (UvSession*)cache_alloc(sessionAllocer, sizeof(UvSession));
	temp->UvSession::UvSession();
	temp->Enable();
	return temp;
} 

void UvSession::Destory(UvSession* & session)
{
	session->Disable();
	//�ֶ�����
	session->UvSession::~UvSession();
	cache_free(sessionAllocer, session);

	session = nullptr;

}

#pragma endregion


#pragma region Implement

void UvSession::Close()
{
	if (this->isShutDown) {
		return;
	}
	this->isShutDown = true;
	printf("�����ػ�\n");
	uv_shutdown(&this->shutdown, (uv_stream_t*)&this->tcpHandle, shutdown_cb);
}

void UvSession::SendData(unsigned char* body, int len)
{
	//���Է��͸����ǵĿͻ���
	auto w_req = (uv_write_t*)cache_alloc(wrAllocer, sizeof(uv_write_t));
	auto w_buf =  uv_buf_init((char*)body, len);
	uv_write(w_req, (uv_stream_t*)&this->tcpHandle, &w_buf, 1, after_write);
}

const char* UvSession::GetAddress(int & clientPort) const
{
	clientPort = this->clientPort;
	return this->clientAddress;
}

#pragma endregion



#pragma region Override

void UvSession::Enable()
{
	AbstractSession::Enable();
	isShutDown = false;
	memset(&this->shutdown, 0, sizeof(this->shutdown));
	memset(&this->tcpHandle, 0, sizeof(this->tcpHandle));
	memset(this->clientAddress, 0, sizeof(this->clientAddress));
	this->clientPort = 0;
	this->recved = 0;
	this->isWebSocketShakeHand = 0;
}

void UvSession::Disable()
{
	AbstractSession::Disable();
}

#pragma endregion


