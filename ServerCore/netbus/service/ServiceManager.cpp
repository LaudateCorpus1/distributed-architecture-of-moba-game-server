#include <cstring>
#include "AbstractService.h"
#include "../../netbus/protocol/CmdPackageProtocol.h"
#include "../../netbus/session/AbstractSession.h"
#include "ServiceManager.h"
#include "../../utils/logger/logger.h"
#include <map>
using std::map;

static map<int, AbstractService*> serviceMap;

void ServiceManager::Init()
{
	//memset(g_serviceSet, 0, sizeof(g_serviceSet));
}

bool ServiceManager::RegisterService(int serviceType, AbstractService* service)
{
	if (serviceMap.find(serviceType)!=serviceMap.end())
	{// ע���
		log_warning("����ע��ʧ�ܡ����ظ���%d", serviceType);
		return false;
	}
	serviceMap[serviceType] = service;
	return true;
}

bool ServiceManager::OnRecvRawPackage(AbstractSession* session, const RawPackage* raw)
{
	if (NULL == raw)
	{
		log_warning("RawPackage��Ϊ��");
		return false;
	}
	if (serviceMap.find(raw->serviceType)==serviceMap.end())
	{// û��ע���
		log_warning("δע���serviceType: %d", raw->serviceType);
		return false;
	}

	//�Ƿ�ʹ��RawPackage
	if (serviceMap[raw->serviceType]->useRawPackage)
	{
		return serviceMap[raw->serviceType]->OnSessionRecvRawPackage(session, raw);
	}

	//ʹ��CmdPackage
	CmdPackage* cmdPackage;

	//���ַ����н�����CmdPackage
	if (CmdPackageProtocol::DecodeBytesToCmdPackage(raw->body, raw->rawLen, cmdPackage))
	{
		//���ݰ���serviceType���ö�Ӧ�ķ���ȥ���������
		auto ret = serviceMap[raw->serviceType]->OnSessionRecvCmdPackage(session, cmdPackage);
		if (!ret)
		{// ����з��񷵻�false���͹ر�session
			session->Close();
		}

		CmdPackageProtocol::FreeCmdPackage(cmdPackage);
		return ret;
	}

	//����ʧ��
	return false;
}

void ServiceManager::OnSessionDisconnected(const AbstractSession* session)
{
	for (auto kv : serviceMap)
	{
		if (kv.second == NULL)
			continue;
		kv.second->OnSessionDisconnected(session, kv.first);
	}
}

void ServiceManager::OnSessionConnect(const AbstractSession* session)
{
	for (auto kv : serviceMap)
	{
		if (kv.second == NULL)
			continue;
		kv.second->OnSessionConnected(session,kv.first);
	}
}
