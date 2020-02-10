#include <uv.h>
#include "UdpSession.h"
#include "../protocol/CmdPackageProtocol.h"
#include "../../utils/logger/logger.h"

void AfterUdpSend(uv_udp_send_t* req, int status)
{
	if (status == 0)
	{
		log_debug("udp send success");
	}
	free(req);
}

void UdpSession::Close()
{
}

void UdpSession::SendData(unsigned char* body, int len)
{
	auto wbuf = uv_buf_init((char*)body, len);
	auto req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
	uv_udp_send(req, this->udp_handler, &wbuf, 1, this->addr, AfterUdpSend);
} 

const char* UdpSession::GetAddress(int& clientPort) const
{
	clientPort = this->clientPort;
	return this->clientAddress;
}

void UdpSession::SendCmdPackage(CmdPackage* msg)
{
	int bodyLen;
	auto rawData = CmdPackageProtocol::EncodeCmdPackageToRaw(msg, &bodyLen);
	if (rawData)
	{// ����ɹ� 

		//��������
		SendData(rawData, bodyLen);

		//�ͷ�����
		CmdPackageProtocol::FreeCmdPackageRaw(rawData);
	}
	else
	{
		log_debug("����ʧ��");
	}
}
