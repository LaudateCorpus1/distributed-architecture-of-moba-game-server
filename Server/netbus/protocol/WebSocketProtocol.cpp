#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "WebSocketProtocol.h"
#include "../../3rd/crypto/base64_encoder.h"
#include "../../3rd/http_parser/http_parser.h"
#include "../../3rd/crypto/sha1.h"

#include "../../utils/cache_alloc/cache_alloc.h"
#include "../../utils/logger/logger.h"

#include "../../netbus/session/AbstractSession.h"


#pragma region ȫ�ֳ���

static const char* wb_migic =
"258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
// base64(sha1(key + wb_migic))
static const char* wb_accept =
"HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade:websocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: %s\r\n"
"WebSocket-Protocol:chat\r\n\r\n";

#pragma endregion


#pragma region ȫ�ֱ���


extern cache_allocer* writeBufAllocer;

static char filed_sec_key[512];
static char value_sec_key[512];
static int is_sec_key = 0;
static int has_sec_key = 0;
//�����Ƿ����
static int is_shaker_ended = 0;


#pragma endregion


#pragma region �ص�����
extern "C" {

#pragma region Http�ص�

	static int on_message_end(http_parser* p) {
		is_shaker_ended = 1;
		return 0;
	}

	static int on_ws_header_field(http_parser* p, const char* at, size_t length)
	{
		if (strncmp(at, "Sec-WebSocket-Key", length) == 0)
		{// ���������ʱWebSocketKey
			is_sec_key = 1;
		}
		else
		{
			is_sec_key = 0;
		}
		return 0;
	}

	static int on_ws_header_value(http_parser* p, const char* at, size_t length)
	{
		if (!is_sec_key)
		{
			return 0;
		}

		//����websocket header
		strncpy(value_sec_key, at, length);
		value_sec_key[length] = 0;

		has_sec_key = 1;

		return 0;
	}

#pragma endregion
}

#pragma endregion


#pragma region WebSocketProtocol

bool WebSocketProtocol::ShakeHand(AbstractSession* session, char* body, int len)
{

#pragma region ����HTTP�ص�����
	http_parser_settings settings;
	http_parser_settings_init(&settings);

	settings.on_header_field = on_ws_header_field;
	settings.on_header_value = on_ws_header_value;
	settings.on_message_complete = on_message_end;
#pragma endregion


#pragma region ����HTTP��ҳ

	http_parser p;
	http_parser_init(&p, HTTP_REQUEST);

	is_sec_key = 0;
	has_sec_key = 0;
	is_shaker_ended = 0;

	http_parser_execute(&p, &settings, body, len);

#pragma endregion




	if (has_sec_key && is_shaker_ended)
	{
		// ��������websocket�����Sec-WebSocket-Key
		log_debug("Sec-WebSocket-Key:\t%s", value_sec_key);

		// key + migic
		static char key_migic[512];
		static char sha1_key_migic[SHA1_DIGEST_SIZE];
		static char sendClientData[512];

		sprintf(key_migic, "%s%s", value_sec_key, wb_migic);

		//��Կ����
		int sha1Size;
		crypt_sha1((unsigned char*)key_migic, strlen(key_migic), (unsigned char*)&sha1_key_migic, &sha1Size);

		//base64���볤��
		int base64_len;
		char* base64Buf = base64_encode((uint8_t*)sha1_key_migic, sha1Size, &base64_len);

		//������Ҫ���͵���������
		sprintf(sendClientData, wb_accept, base64Buf);

		//�ͷ�base64��������
		base64_encode_free(base64Buf);
		
		session->SendData((unsigned char*)sendClientData, strlen(sendClientData));
	
		return true;
	}

	return false;
}

//��ȡ��ͷ����
bool WebSocketProtocol::ReadHeader(unsigned char* pkgData, int pkgLen, int* out_pkgSize, int* out_header_size)
{
	if (pkgData[0] != 0x81 && pkgData[0] != 0x82)
	{
		//������Чwebsocket����ͷ��ֱ�ӷ��أ�������
		log_debug("������Чwebsocket����ͷ, ֱ�ӷ���");
		return false; 
	}
	if (pkgLen < 2)
	{// û���㹻λ���dataLen
		log_debug("û���㹻λ���dataLen");
		return false;
	}

	//������Ҫȥ��һ�����λ��ȡʣ����λ������
	unsigned int dataLen = pkgData[1] & 0x0000007f;



	int headSize = 2;
	if (dataLen == 126)
	{// ���������ֽڱ�ʾ���ݳ���
		headSize += 2;
		if (pkgLen < headSize)
		{// û���㹻λ���dataLen
			log_debug("û���㹻λ���dataLen");
			return false;
		}
		dataLen = pkgData[3] | (pkgData[2] << 8);
	}
	else if (dataLen == 127)
	{// ����˸��ֽڱ�ʾ���ݳ���

		headSize += 8;
		if (pkgLen < headSize)
		{// û���㹻λ���dataLen
			log_debug("û���㹻λ���dataLen");
			return false;
		}
		int low = pkgData[5] | (pkgData[4] << 8) | (pkgData[3] << 16) | (pkgData[2] << 24);
		int high = pkgData[9] | (pkgData[8] << 8) | (pkgData[7] << 16) | (pkgData[6] << 24);

		//��Ϊ�����ò�����ô�����ݣ�����dataLen��ֻȡ��λ
		dataLen = low;

	}

	//�����ĸ�maskλ
	headSize += 4;
	if (pkgLen < headSize)
	{// û���㹻λ���mask
		log_debug("û���㹻λ���mask");
		return false;
	}

	*out_header_size = headSize;
	*out_pkgSize = dataLen;

	return true;
}

//�����յ��Ĵ�����
void WebSocketProtocol::ParserRecvData(unsigned char* rawData, unsigned char* mask, int rawLen)
{
	for (int i = 0; i < rawLen; i++)
	{
		rawData[i] ^= mask[i % 4];
	}
}

//�������ݴ����WebSocket��
unsigned char* WebSocketProtocol::Package(const unsigned char* rawData, int rowDataLen, int * out_pkgLen)
{
	int headSize = 2;
	if (rowDataLen > 125 && rowDataLen < 65536)
	{// �����ֽ�
		headSize += 2;
	}
	else if (rowDataLen >= 65536)
	{// �˸��ֽ�
		headSize += 8;
		//̫�����ǾͲ�����
		return NULL;
	}

	unsigned char* dataBuf = (unsigned char*)cache_alloc(writeBufAllocer,headSize + rowDataLen);
	
	dataBuf[0] = 0x81;
	if (rowDataLen < 125) {
		dataBuf[1] = rowDataLen;
	}
	else if (rowDataLen > 125 && rowDataLen < 65536) {
		dataBuf[1] = 126;
		dataBuf[2] = (rowDataLen & 0x0000ff00) >> 8;
		dataBuf[3] = (rowDataLen & 0x000000ff);
	}

	memcpy(dataBuf + headSize, rawData, rowDataLen);

	*out_pkgLen = headSize + rowDataLen;
	
	return dataBuf;
}

//�ͷ�WebSocket��
void WebSocketProtocol::ReleasePackage(unsigned char* pkg)
{
	cache_free(writeBufAllocer, pkg);
}

#pragma endregion




