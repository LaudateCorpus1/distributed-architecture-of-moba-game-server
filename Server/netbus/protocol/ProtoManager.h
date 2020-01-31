#ifndef __PROTOMANAGER_H__
#define __PROTOMANAGER_H__


//�ı����䷽ʽ
enum class ProtoType{
	Json = 0,
	Protobuf = 1,
};

//�Զ���Э���
struct CmdPackage {

	// ���ṹ
	// serviceType (2 bytes) | cmdType (2 bytes) | userTag (4 bytes) | body

	int serviceType;	//�����
	int cmdType;		//�����
	unsigned int userTag;//�û���ʶ
	void* body;			// JSON str ������message;
};

//�Զ���Э��Ĺ�����
//���ƴ������ݵļ��ܣ����룬����
class ProtoManager
{
public:
	//��ʼ��Э��
	static void Init(ProtoType proto_type=ProtoType::Protobuf);
	static void RegisterPfCmdMap(const char** pf_map, int len);
	static ProtoType ProtoType();

	static bool DecodeCmdMsg(unsigned char* cmd, int cmd_len, struct CmdPackage*& out_msg);
	static void FreeCmdMsg(struct CmdPackage* msg);

	static unsigned char* EncodeCmdMsgToRaw(const struct CmdPackage* msg, int* out_len);
	static void FreeCmdMsgRaw(unsigned char* raw);
};




#endif // !__PROTOMANAGER_H__

