#ifndef __WEBSOCKETPROTOCOL_H__
#define __WEBSOCKETPROTOCOL_H__

class WebSocketProtocol
{ 
public:
	//�����Ƿ��չ���
	static bool ShakeHand(AbstractSession* session, char* body, int len);
	//��ȡWebSocketͷ
	static int ReadHeader(unsigned char* pkgData, int pkgLen,int pkgSize,int& out_header_size);
	//�����յ�������
	static void ParserRecvData(unsigned char* rawData,unsigned char* mask,int rawLen);
	//�������
	static unsigned char* PackageData(const unsigned char* rawData, int len, int* dataLen);
	//�ͷŴ������ʹ�õ��ڴ�
	static void FreePackageData(unsigned char* pkg);
};

#endif // !__WEBSOCKETPROTOCOL_H__



