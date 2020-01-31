#include "TcpPackageProtocol.h"
#include <stdlib.h>
#include <string.h>

#include "../../utils/cache_alloc.h"

extern cache_allocer* writeBufAllocer;



bool TcpProtocol::ReadHeader(unsigned char* data, int dataLen, int* out_pkgSize, int* out_headerSize)
{
	if (dataLen < 2) 
	{// ̫�̣��޷�����
		return false;
	}

	*out_pkgSize = data[0] | (data[1] << 8);
	*out_headerSize = 2;

	return false;
}

unsigned char* TcpProtocol::Package(const unsigned char* rawData, int rowDataLen, int* pkgLen)
{
	int headSize = 2; 
	*pkgLen = headSize + rowDataLen;
	unsigned char* dataBuf = (unsigned char*)cache_alloc(writeBufAllocer, headSize + rowDataLen);
	
	//��¼���峤��
	dataBuf[0] = (unsigned char)((*pkgLen) & 0x000000ff);
	dataBuf[1] = (unsigned char)(((*pkgLen) & 0x0000ff00) >> 8);
	
	//�����ݿ��������Ⱥ���
	memcpy(dataBuf + headSize, rawData, rowDataLen);
	
	return dataBuf;
}

void TcpProtocol::ReleasePackage(unsigned char* package)
{
	cache_free(writeBufAllocer, package);
}
