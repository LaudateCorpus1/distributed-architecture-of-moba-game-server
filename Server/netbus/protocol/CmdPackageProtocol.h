#ifndef __PROTOMANAGER_H__
#define __PROTOMANAGER_H__

#include "google/protobuf/message.h"
#include <map>
#include <string>
using std::map;
using std::string;

//�ı����䷽ʽ
enum class ProtoType{
	Json = 0,
	Protobuf = 1,
};

//�Զ���Э���
struct CmdPackage {

#define CMD_HEADER_SIZE 8
	// ���ṹ
	// serviceType (2 bytes) | cmdType (2 bytes) | userTag (4 bytes) | body

	int serviceType;	//�����
	int cmdType;		//�����
	unsigned int userTag;//�û���ʶ
	void* body;			// JSON str ������message;
};

//�Զ���Э��Ĺ�����
//���ƴ������ݵļ��ܣ����룬����
class CmdPackageProtocol
{
public:
	//��ʼ��Э��
	static void Init(ProtoType proto_type=ProtoType::Protobuf);
	
	static void RegisterProtobufCmdMap(map<int, string>& map);


	static const char* ProtoCmdTypeToName(int cmdType);
	
	static ProtoType ProtoType();

	static bool DecodeCmdMsg(unsigned char* cmd, const int cmd_len, struct CmdPackage*& out_msg);
	static void FreeCmdMsg(struct CmdPackage* msg);

	static unsigned char* EncodeCmdPackageToRaw(const struct CmdPackage* msg, int* out_len);
	static void FreeCmdPackageRaw(unsigned char* raw);

	static google::protobuf::Message* CreateMessage(const char* typeName);
	static void ReleaseMessage(google::protobuf::Message* msg);
};




#endif // !__PROTOMANAGER_H__

