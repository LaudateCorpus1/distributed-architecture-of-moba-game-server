#include "WinUtil.h"
#include "../Encoding.h"

string WinUtil::GetDirPath(const string& filePath)
{
	auto index = filePath.find_last_of("\\");
	return filePath.substr(0, index);
}

//����ֵ�� 
//���callback����false��ֹ����������enumsubfilesҲ����false��
//����������Ϻ󷵻�true
bool WinUtil::SearchFromDir(
	const std::wstring& dir_with_back_slant,  //��Ŀ¼������: L"C:\\", L"E:\\test\\"        
	const std::wstring& filename,             //����: L"*", L"123.txt", L"*.exe", L"123.???"
	unsigned int maxdepth,                    //�����ȡ�0���������ļ��У�-1�������������ļ���
	SearchType flags,                          //���ؽ��Ϊ�ļ��������ļ��У����Ƕ�����
	std::function<bool(const std::wstring & dir, _wfinddata_t & attrib)> callback
)
{
	auto dir = dir_with_back_slant;
	if (*(dir.end() - 1) != '\\')
		dir += '\\';
	
	_wfinddata_t dat;
	size_t hfile;
	std::wstring fullname = dir + filename;
	std::wstring tmp;
	bool ret = true;


	hfile = _wfindfirst(fullname.c_str(), &dat);
	if (hfile == -1) goto a;
	do {
		if (!(wcscmp(L".", dat.name) && wcscmp(L"..", dat.name))) continue;
		if (
			((dat.attrib & _A_SUBDIR) && (!((int)flags & (int)SearchType::ENUM_DIR))) ||
			((!(dat.attrib & _A_SUBDIR)) && (!((int)flags & (int)SearchType::ENUM_FILE)))
			) continue;
		ret = callback(dir, dat);
		if (!ret) {
			_findclose(hfile);
			return ret;
		}
	} while (_wfindnext(hfile, &dat) == 0);
	_findclose(hfile);

a:

	if (!maxdepth) return ret;

	tmp = dir + L"*";
	hfile = _wfindfirst(tmp.c_str(), &dat);
	if (hfile == -1) return ret;
	do {
		if (!(wcscmp(L".", dat.name) && wcscmp(L"..", dat.name))) continue;
		if (!(dat.attrib & _A_SUBDIR)) continue;
		tmp = dir + dat.name + L"\\";
		ret = SearchFromDir(tmp, filename, maxdepth - 1, flags, callback);
		if (!ret) {
			_findclose(hfile);
			return ret;
		}


	} while (_wfindnext(hfile, &dat) == 0);
	_findclose(hfile);

	return ret;

}

bool WinUtil::SearchFromDir(
	const std::string& dir_with_back_slant, 
	const std::string& filename, 
	unsigned int maxdepth, 
	SearchType flags, 
	std::function<bool(const std::string & dir, const string & fileName)> callback)
{
	std::wstring dir = Encoding::UTF8ToUnicode(dir_with_back_slant);
	std::wstring fileName = Encoding::UTF8ToUnicode(filename);
	return WinUtil::SearchFromDir(dir, fileName, maxdepth, flags,
		[=](const std::wstring& dir, _wfinddata_t& attrib)->bool
		{
			//dirĩβ��һ����б��'\\'
			auto _dir = Encoding::UnicodeToUTF8(dir);
			auto wName = wstring(attrib.name);
			//return true����������return falseֹͣ����
			return callback(_dir,Encoding::UnicodeToUTF8(wName));         
		});
}
