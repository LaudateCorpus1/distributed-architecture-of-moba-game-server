#ifndef __WEBSOCKETPROTOCOL_H__
#define __WEBSOCKETPROTOCOL_H__

class WebSocketProtocol
{ 
public:
	//�����Ƿ��չ���
	static bool ShakeHand(AbstractSession* session, char* body, int len);
	//��ȡWebSocketͷ
	static bool ReadHeader(unsigned char* pkgData, int pkgLen, int* out_pkgSize, int* out_header_size);
	//�����յ�������
	static void ParserRecvData(unsigned char* rawData,unsigned char* mask,int rawLen);
	//�������
	static unsigned char* Package(const unsigned char* rawData, int rowDataLen, int * out_pkgLen);
	//�ͷŴ������ʹ�õ��ڴ�
	static void ReleasePackage(unsigned char* pkg);
};

#endif // !__WEBSOCKETPROTOCOL_H__



