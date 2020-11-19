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

struct RawPackage
{
	int serviceType;	//�����
	int cmdType;		//�����
	unsigned int userTag;//�û���ʶ

	unsigned char* body;
	int rawLen;
};

//�Զ���Э��Ĺ�����
//���ƴ������ݵļ��ܣ����룬����
class CmdPackageProtocol
{
public:
	//��ʼ��Э��
	static void Init(::ProtoType proto_type);
	static void Init(::ProtoType proto_type, const string& protoFileDir);
	
	static void RegisterProtobufCmdMap(map<int, map<int, string>*>& map);


	static const char* GetMessageName(int stype, int ctype);
	static ProtoType ProtoType();

	//����CmdPackage��ͷ
	static bool DecodeBytesToRawPackage(unsigned char* cmd, const int cmd_len, struct RawPackage* out_msg);


	//����CmdPackage��
	static bool DecodeBytesToCmdPackage(unsigned char* cmd, const int cmd_len, struct CmdPackage*& out_msg);
	static void FreeCmdPackage(struct CmdPackage* msg);
	//��ԭʼ���ݱ����CmdPackage��
	static unsigned char* EncodeCmdPackageToBytes(const struct CmdPackage* msg, int* out_len);
	static void FreeCmdPackageBytes(unsigned char* raw);
	//�������ͷ�ProtobufMessage
	static google::protobuf::Message* CreateMessage(const char* typeName);
	static void ReleaseMessage(google::protobuf::Message* msg);
};




#endif // !__PROTOMANAGER_H__

