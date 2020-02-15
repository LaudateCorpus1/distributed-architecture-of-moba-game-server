#ifndef __WIN_UTIL_H__
#define __WIN_UTIL_H__


#include <functional>
#include <io.h>
#include <string>
using std::string;
//��������������������������������
//��Ȩ����������ΪCSDN������liuqx0717����ԭ�����£���ѭ CC 4.0 BY - SA ��ȨЭ�飬ת���븽��ԭ�ĳ������Ӽ���������
//ԭ�����ӣ�https ://blog.csdn.net/liuqx97bb/article/details/77074833
enum class SearchType {
	ENUM_FILE = 1,
	ENUM_DIR,
	ENUM_BOTH
};

class WinUtil
{
public:
	static string GetDirPath(const string& filePath);
	static bool SearchFromDir(
		const std::wstring& dir_with_back_slant,  //��Ŀ¼������: "L"C:\\", L"E:\\test\\"       
		const std::wstring& filename,             //����: L"*", L"123.txt", L"*.exe", L"123.???"
		unsigned int maxdepth,                    //�����ȡ�0���������ļ��У�-1�������������ļ���
		SearchType flags,                          //���ؽ��Ϊ�ļ��������ļ��У����Ƕ�����
		std::function<bool(const std::wstring & dir, _wfinddata_t & attrib)> callback //return true����������return falseֹͣ����
	);

	static bool SearchFromDir(
		const std::string& dir_with_back_slant,  //��Ŀ¼������: "L"C:\\", L"E:\\test\\"       
		const std::string& filename,             //����: L"*", L"123.txt", L"*.exe", L"123.???"
		unsigned int maxdepth,                    //�����ȡ�0���������ļ��У�-1�������������ļ���
		SearchType flags,                          //���ؽ��Ϊ�ļ��������ļ��У����Ƕ�����
		std::function<bool(const std::string & dirPath, const string & fileName)> callback //return true����������return falseֹͣ����
	);
};

//����ֵ��
//���callback����false��ֹ����������enumsubfilesҲ����false��
//����������Ϻ󷵻�true


#endif // !__WIN_UTIL_H__




