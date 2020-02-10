
#include <cstring>
#include <cstdlib>
#include "CmdPackageProtocol.h"
#include "google/protobuf/message.h"
#include "../../utils/logger/logger.h"


#define MAX_PF_MAP_SIZE 1024

#pragma region ȫ�ֱ���

static ProtoType g_protoType;
static char* g_pf_map[MAX_PF_MAP_SIZE];
static int g_cmdCount = 0;

#pragma endregion

static google::protobuf::Message* CreateMessage(const char* typeName)
{
	google::protobuf::Message* msg = NULL;
	//�������֣��ҵ�message����������
	auto descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
	if (descriptor)
	{
		//������������Ӷ��󹤳�������һ����Ӧģ�����
		//����ģ�帴�Ƴ���һ��
		auto protoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (protoType)
		{
			msg = protoType->New();
		}
	}
	return msg;
}

static void ReleaseMessage(google::protobuf::Message* msg)
{
	delete msg;
}


void CmdPackageProtocol::Init(::ProtoType proto_type)
{
	g_protoType = proto_type;
}

void CmdPackageProtocol::RegisterPfCmdMap(const char** pf_map, int len)
{
	len = MAX_PF_MAP_SIZE - g_cmdCount < len 
		? MAX_PF_MAP_SIZE - g_cmdCount 
		: len;

	for (auto i = 0; i < len; i++)
	{
		g_pf_map[g_cmdCount + i] = strdup(pf_map[i]);
	}

	g_cmdCount += len;

}

ProtoType CmdPackageProtocol::ProtoType()
{
	return g_protoType;
}

bool CmdPackageProtocol::DecodeCmdMsg(unsigned char* cmd, const int cmd_len, CmdPackage*& out_msg)
{
	out_msg = NULL;
	// serviceType (2 bytes) | cmdType (2 bytes) | userTag (4 bytes) | body
	if (cmd_len < CMD_HEADER_SIZE)
	{// ����̫��
		log_debug("����̫��");
		return false;
	}

	out_msg = (CmdPackage*)malloc(sizeof(CmdPackage));
	memset(out_msg, 0, sizeof(CmdPackage));

	out_msg->serviceType = cmd[0] | (cmd[1] << 8);
	out_msg->cmdType = cmd[2] | (cmd[3] << 8);
	out_msg->userTag = cmd[4] | (cmd[5] << 8) | (cmd[6] << 16) | (cmd[7] << 24);
	out_msg->body = NULL;

	if (cmd_len == CMD_HEADER_SIZE)
		return true;


	// ����

	auto dataLen = cmd_len - CMD_HEADER_SIZE;
	char* tempCharPointer;
	google::protobuf::Message* tempMessagePointer;
	switch (g_protoType)
	{
	case ProtoType::Json:
		tempCharPointer = (char*)malloc(dataLen + 1);
		memcpy(tempCharPointer, cmd + CMD_HEADER_SIZE, dataLen);
		tempCharPointer[dataLen] = 0;
		out_msg->body = tempCharPointer;
		break;
	case ProtoType::Protobuf:
		//û�����protobufЭ��
		if (out_msg->cmdType < 0
			|| out_msg->cmdType >= g_cmdCount
			|| g_pf_map[out_msg->cmdType] == NULL)
		{
			log_debug("û�����protobufЭ��");
			free(out_msg);
			out_msg = NULL;
			return false;
		}

		tempMessagePointer = CreateMessage(g_pf_map[out_msg->cmdType]);
		if (tempMessagePointer == NULL)
		{
			log_debug("��ȡMessage����Ϊ��");
			free(out_msg);
			out_msg = NULL;
			return false;
		}

		if (!tempMessagePointer->ParseFromArray(cmd + CMD_HEADER_SIZE, cmd_len - CMD_HEADER_SIZE))
		{
			log_debug("��Ϣ�����л�ʧ��");
			free(out_msg);
			out_msg = NULL;
			ReleaseMessage(tempMessagePointer);
			return false;
		}

		out_msg->body = tempMessagePointer;


		break;
	}

	return true;
}

void CmdPackageProtocol::FreeCmdMsg(CmdPackage* msg)
{
	if (msg->body)
	{
		switch (g_protoType)
		{
		case ProtoType::Json:
			free(msg->body);
			msg->body = NULL;
			break;
		case ProtoType::Protobuf:
			auto pm = (google::protobuf::Message*)msg->body;
			delete pm;
			msg->body = NULL;
			break;
		}
	}

	free(msg);
}

unsigned char* CmdPackageProtocol::EncodeCmdPackageToRaw(const CmdPackage* msg, int* out_len)
{
	//����


	unsigned char* rawData = NULL;

	char* tempCharPointer;
	int tempLen;
	google::protobuf::Message* tempMessagePointer;

	switch (g_protoType)
	{
	case ProtoType::Json:

		tempCharPointer = (char*)msg->body;
		tempLen = strlen(tempCharPointer) + 1;

		rawData = (unsigned char*)malloc(CMD_HEADER_SIZE + tempLen);
		memcpy(rawData + CMD_HEADER_SIZE, tempCharPointer, tempLen -1);

		rawData[CMD_HEADER_SIZE + tempLen - 1] = 0;

		//��������
		*out_len = CMD_HEADER_SIZE + tempLen;
		break;
	case ProtoType::Protobuf:
		tempMessagePointer = (google::protobuf::Message*)msg->body;
		tempLen = tempMessagePointer->ByteSize();
		rawData = (unsigned char*)malloc(CMD_HEADER_SIZE + tempLen);

		if (!tempMessagePointer->SerializePartialToArray(rawData + CMD_HEADER_SIZE, tempLen))
		{// ������л����ɹ�
			free(rawData);
			*out_len = 0;
			return NULL;
		}

		//��������
		*out_len = CMD_HEADER_SIZE + tempLen;
		break;
	}

	//д��ͷ��Ϣ
	rawData[0] = msg->serviceType & 0x000000ff;
	rawData[1] = (msg->serviceType & 0x0000ff00) >> 8;
	rawData[2] = msg->cmdType & 0x000000ff;
	rawData[3] = (msg->cmdType & 0x0000ff00) >> 8;

	//userTag
	memcpy(rawData + 4, &msg->userTag, 4);

	return rawData;
}

void CmdPackageProtocol::FreeCmdPackageRaw(unsigned char* raw) 
{
	free(raw);
}
